#include "user.h"
#include "bot.h"
#include "botContext.h"
#include "fund.h"
#include "userMetrics.h"
#include "oracle.h"

User::User(QString userId, QJsonObject jsonArgs)
	: DBobj(userId, 0)
	, m_jsonArgs(jsonArgs)
{
	//makes the oracle
	oracle();
}

User::~User()
{
	if (m_mainOracle)
		delete m_mainOracle;
}

SuperOracle* User::oracle()
{
	if (!m_mainOracle) {
		NOTICE() << "Making new user's Oracle";
		m_mainOracle = new SuperOracle();
	}
	return m_mainOracle;
}

SuperOracle::Summary User::smallSummary()
{
	return oracle()->computeAvgCashFlow(false);
}

double  User::littleIncome()
{
	SuperOracle::Summary s = smallSummary();
	double flow = s.flow();
	double flowFix = s.flowFix();
	if (s.posSum == 0)
		return 0.01;
	if (s.salary <= 10)
		return 0.01;
	if (flowFix < -2.0) {
		double fac = qSqrt(qMax(0.0, flowFix + 3.0));
		WARN() << "little fixed income: fac " << fac;
		return fac;
	}
	if (flow < 0.0) {
		double fac = qSqrt(qMax(0.0, flow + 1.0));
		WARN() << "little income: fac " << fac;
		return fac;
	}
	return 1.0;
}

bool User::setHypotheTrans(int amount) {
	//	if (qRound(Transaction::s_hypotheTrans.amountDbl()) == amount)
//		return false;
	// clears all the cached results
	HistoMetric::clearAll();

	Transaction::s_hypotheTrans.setAmount(amount);
	Transaction::s_hypotheTrans.date = Transaction::currentDay();
	Transaction::s_hypotheTrans.name += "hypothetic transaction, fucking take the red pill dude!";
	Transaction::s_hypotheTrans.nameHash.setFromString(Transaction::s_hypotheTrans.name, Transaction::s_hypotheTrans.kla());
	Transaction::s_hypotheTrans.id = Transaction::s_hypotheTrans.name;
	if (&m_allTransBundle.last() != &Transaction::s_hypotheTrans)
		m_allTransBundle.append(&Transaction::s_hypotheTrans);
	return true;
}

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

//	//////// "user"
//	QJsonObject jsonUser = jsonObj["user"].toObject();
//	m_email = jsonUser["local"].toObject()["email"].toString();
//	qDebug() << "user" << jsonUser["_id"].toString() << ":" << jsonUser["local"].toObject()["email"].toString();
//	Q_ASSERT_X(jsonUser["_id"].toString() == id(), "injectJsonData", jsonUser["_id"].toString().toUtf8() + " != " + id().toUtf8());

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
		m_allTransactions.appendNew(jsonTrans, pInAcc);
	}
	INFO() << "maxDaysOldAllTransatcion " << Transaction::maxDaysOldAllTransatcion();
	m_allTransactions.sort();

	//////// complete Accounts with transaction pointers in bundles
	for (int i = 0; i < m_allTransactions.count(); ++i) {
		Transaction* pT = &m_allTransactions.transArray()[i];
		pT->account->append(pT);
	}

	//////// anihilates transactions based on certain rules, such as internal transfers
	for (int i = 0; i < m_allTransactions.count(); ++i) {
		Transaction* pT = &m_allTransactions.transArray()[i];
		Transaction* pBestMatchN = 0;
		qint64 bestDist = 999999999;
		// for a positive transcation (rare)
		if (pT->amount() > 0 && !pT->isInternal()) {
			Transaction tP(*pT);
			// take its symetrical and look for some closeby (negative) transaction that matches
			tP.setAmount(-tP.amountDbl());
			for (int j = 0; j < m_allTransactions.count(); ++j) {
				Transaction* pN = &m_allTransactions.transArray()[j];
				// dist max 4 days, 2 parts of kla, and no hash sensitivity
				qint64 d = tP.distanceWeighted<3, 2, 1024*1024*1024>(*pN);
				if (d < Transaction::LIMIT_DIST_TRANS
						&& pN->amount() < 0 && !pN->isInternal()
						&& pT->account != pN->account) {
					if (d < bestDist) {
						bestDist = d;
						pBestMatchN = pN;
					}
				}
			}
			if (pT && pBestMatchN) {
				pT->flags |= Transaction::Flag::Internal;
				pBestMatchN->flags |= Transaction::Flag::Internal;
				INFO() << "Matching internal transactions";
				INFO() << pT->name << " " << pT->amountDbl() << " " << pT->date.toString();
				INFO() << pBestMatchN->name << " " << pBestMatchN->amountDbl() << " " << pBestMatchN->date.toString();
			}
		}
	}
	m_allTransactions.stampAllTransactionEffect();

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

	// remake the context just in case
	makeBotContext();

	emit injected(this);

//	CostRateMonthPercentileMetric<2, 50>::get(this)->value(Transaction::currentDay());
//	CostRateMonthPercentileMetric<2, 75>::get(this)->value(Transaction::currentDay());
//	CostRateMonthPercentileMetric<2, 90>::get(this)->value(Transaction::currentDay());
//	CostRateMonthPercentileMetric<2, 95>::get(this)->value(Transaction::currentDay());
//	CostRateMonthPercentileMetric<2, 99>::get(this)->value(Transaction::currentDay());
}

BotContext* User::makeBotContext()
{
	if(m_botContext) {
		WARN() << "Remaking BotContext";
		delete m_botContext;
	}
	m_botContext = new BotContext(this);
	return m_botContext;
}

void User::injectJsonBot(QString jsonStr)
{
	QJsonDocument jsonDoc(QJsonDocument::fromJson(jsonStr.toUtf8()));
	const QJsonObject& jsonObj = jsonDoc.object();

	INFO() << "User::injectJsonBot " << QString(QJsonDocument(jsonObj).toJson());

	{
		QFile sampleReturn("jsonBot.json");
		sampleReturn.open(QFile::WriteOnly | QFile::Truncate);
		QTextStream fileout(&sampleReturn);
		fileout << jsonDoc.toJson(QJsonDocument::Indented);
	}

	// makes the bot and build the tree inside
	m_bestBot = new Bot(jsonObj, this);
	m_bestBot->init(m_botContext);

	emit botInjected(m_bestBot);
}

void User::reComputeBot()
{
	m_bestBot->summarize();
}

