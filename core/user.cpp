#include "user.h"
#include "bot.h"
#include "botContext.h"
#include "fund.h"

void User::injectJsonData(QString jsonStr)
{
	qDebug() << "injecting" << jsonStr.left(1024);
	QJsonDocument jsonDoc(QJsonDocument::fromJson(jsonStr.toUtf8()));
	const QJsonObject& jsonObj = jsonDoc.object();

	{
		QFile sampleReturn("jsonData.json");
		sampleReturn.open(QFile::WriteOnly | QFile::Truncate);
		QTextStream fileout(&sampleReturn);
		fileout << jsonDoc.toJson(QJsonDocument::Indented);
	}

	//////// "user"
	QJsonObject jsonUser = jsonObj["user"].toObject();
	m_email = jsonUser["local"].toObject()["email"].toString();
	qDebug() << "user" << jsonUser["_id"].toString() << ":" << jsonUser["local"].toObject()["email"].toString();
	Q_ASSERT_X(jsonUser["_id"].toString() == id(), "injectJsonData", jsonUser["_id"].toString().toUtf8() + " != " + id().toUtf8());

	//////// "banks"
	QJsonArray jsonBankArray = jsonObj["banks"].toArray();
	qDebug() << jsonBankArray.size() << "banks";
	for (int iB = 0; iB < jsonBankArray.size(); ++iB) {
		QJsonObject jsonBank = jsonBankArray[iB].toObject();
		Bank* pBank = new Bank(jsonBank, this);
		m_banks.push_back(pBank);
	}

	//////// "accounts"
	QJsonArray jsonAccountArray = jsonObj["accounts"].toArray();
	qDebug() << jsonAccountArray.size() << "accounts";
	for (int iA = 0; iA < jsonAccountArray.size(); ++iA) {
		QJsonObject jsonAcc = jsonAccountArray[iA].toObject();
		QString bankTok = jsonAcc["access_token"].toString();
		Bank* pBank = getBankByToken(bankTok);
		Account* pAcc = new Account(jsonAcc, pBank);
		m_accounts.push_back(pAcc);
	}

	//////// "transactions"
	QJsonArray jsonTransArray = jsonObj["transactions"].toArray();
	qDebug() << jsonTransArray.size() << "transactions";
	for (int iT = 0; iT < jsonTransArray.size(); ++iT) {
		QJsonObject jsonTrans = jsonTransArray[iT].toObject();
		QString acPlaidId = jsonTrans["plaid_account"].toString();
		Account* pInAcc = getAccountByPlaidId(acPlaidId);
		m_allTransactions.appendNew(pInAcc)->read(jsonTrans);
	}
	m_allTransactions.sort();

	//////// complete Accounts with transaction pointers in bundles
	for (int i = 0; i < m_allTransactions.count(); ++i) {
		Transaction* pT = &m_allTransactions.transArray()[i];
		pT->account->append(pT);
	}

	//////// "funds"
	QJsonArray jsonFundArray = jsonObj["funds"].toArray();
	qDebug() << jsonFundArray.size() << "funds";
	for (int iF = 0; iF < jsonFundArray.size(); ++iF) {
		QJsonObject jsonFund = jsonFundArray[iF].toObject();
		Fund* pFund = new Fund(jsonFund, this);
		if (pFund->name() == "Extra cash") {
			m_extraCacheFund = pFund;
			// set the cashes of the "Extra cash" fund
			m_extraCacheFund->populate(jsonObj["cashes"].toArray());
			qDebug() << m_extraCacheFund->cashes().size() << "Extra Cashes";
		}
	}

	makeHashBundles();

	emit injected(this);
}

BotContext* User::makeBotContext()
{
	if(m_botContext)
		delete m_botContext;
	m_botContext = new BotContext(this);
	return m_botContext;
}

void User::injectJsonBot(QString jsonStr)
{
	qDebug() << "injecting" << jsonStr.left(1024);
	QJsonDocument jsonDoc(QJsonDocument::fromJson(jsonStr.toUtf8()));
	const QJsonObject& jsonObj = jsonDoc.object();

	{
		QFile sampleReturn("jsonBot.json");
		sampleReturn.open(QFile::WriteOnly | QFile::Truncate);
		QTextStream fileout(&sampleReturn);
		fileout << jsonDoc.toJson(QJsonDocument::Indented);
	}

	// remake the context just in case
	makeBotContext();
	m_bestBot = new Bot(jsonObj, this);
	m_bestBot->init(m_botContext);

	emit botInjected();

	m_bestBot->summarize();
}

double User::costLiving(double withinPercentileCost, double multiplicator /*= 1.0*/)
{
	QVector<double> costs;
	for (int i = 0; i < m_allTransactions.count(); ++i) {
		double amnt = m_allTransactions.trans(i).amountDbl();
		amnt *= multiplicator;
		if (amnt < 0.0) {
			costs.append(-amnt);
		}
	}
	qSort(costs);
	double avg = 0.0;
	int lastCostsInd = costs.count() * withinPercentileCost;
	for (int i = 0; i < lastCostsInd; ++i) {
		avg += costs[i];
	}
	double numDays = m_allTransactions.firstTransactionDate().daysTo(m_allTransactions.lastTransactionDate());
	if (numDays) {
		avg /= numDays;
		qDebug() << (multiplicator > 0.0 ? "cost" : "make") << "living ("<< qRound(withinPercentileCost * 100.0) << "\%:" <<lastCostsInd<<"T)" << avg;
	}
	else {
		qWarning() << "numDays" << numDays << "returning cost/make Living 0";
	}
	return avg;
}

double User::makeLiving(double withinPercentileCost, double multiplicator /*= 1.0*/)
{
	return costLiving(withinPercentileCost, -multiplicator);
}

QVector<Transaction> User::predictedFutureTransactions(double threshProba) {
	QVector<Transaction> ret;
	if (m_bestBot) {
		for (Transaction& t : m_bestBot->predictTrans(threshProba)) {
			Q_ASSERT(t.flags & Transaction::Predicted);
			if (t.date >= m_today.addDays(-1)) {
				// if it didn't CameTrue
				if (!(t.flags & Transaction::CameTrue)) {
					ret.append(t);
				}
			}
		}
	}
	LOG() << "predictedFutureTransactions(" << threshProba << "), size = " << ret.size() << endl;
	return ret;
}

SparkLine User::predictedSparkLine(double threshProba)
{
	// will hold temporary relative changes so that they can be organized
	// with the losses first (to be safe and not predict a salary before a bill if on the same day)
	SparkLine temp;
	for (Transaction& t : predictedFutureTransactions(threshProba)) {
		int futDays = m_today.daysTo(t.date);
		temp.insertMulti(futDays, t.amountDbl());
	}
	// now we sort them and make absolute values in the Sparkline
	double balanceNow = balance(Account::Type::Checking);
	LOG() << "predictedSparkLine(" << threshProba << "), temp.size = " << temp.size() << ", temp.uniqueKeys = " << temp.uniqueKeys().size() << ". balanceNow = " << balanceNow << endl;
	qDebug() << "temp.uniqueKeys()" << temp.uniqueKeys();
	SparkLine ret;
	// insert the balanceNow for the last transaction day
	ret.insertMulti(-9999, balanceNow);
	for (auto futDay : temp.uniqueKeys()) {
		QList<double> transThatDay = temp.values(futDay);
		std::sort(transThatDay.begin(), transThatDay.end());
		// insert from the end (see QMap::values(key) documentation)
		for (int i = transThatDay.size() - 1; i >= 0; --i) {
			balanceNow += transThatDay[i];
			ret.insertMulti(futDay, balanceNow);
//			qDebug() << futDay << transThatDay[i];
			LOG() << "on day " << futDay << ": " << transThatDay[i] << " -> " << balanceNow << endl;
		}
	}
	qDebug() << ret;
	return ret;
}



