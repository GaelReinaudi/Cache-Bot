#include "user.h"
#include "bot.h"
#include "botContext.h"
#include "fund.h"
#include "userMetrics.h"
#include "oracle.h"

User::User(QString userId, QObject *parent)
	:DBobj(userId, parent)
{
	m_today = QDate::currentDate();

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
		LOG() << "Making new user's Oracle" << endl;
		m_mainOracle = new SuperOracle();
	}
	return m_mainOracle;
}

void User::setHypotheTrans(double amount) {
	m_hypotheTrans.setAmount(amount);
	m_hypotheTrans.date = QDate::currentDate();
	m_hypotheTrans.name += "Hypothetic Transaction, fucking take the red pill dude!";
	m_hypotheTrans.nameHash.setFromString(m_hypotheTrans.name);
	if (&m_allTransBundle.trans(-1) != &m_hypotheTrans)
		m_allTransBundle.append(&m_hypotheTrans);
	if (m_bestBot)
		m_bestBot->summarize();
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
		m_allTransactions.appendNew(jsonTrans, pInAcc);
	}
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
		// for a ppositive transcation (rare)
		if (pT->amountInt() > 0 && !pT->isInternal()) {
			Transaction tP(*pT);
			// take its symetrical and look for some closeby (negative) transaction that matches
			tP.setKLA(-tP.amountInt());
			for (int j = 0; j < m_allTransactions.count(); ++j) {
				Transaction* pN = &m_allTransactions.transArray()[j];
				// dist max 4 days, 2 parts of kla, and no hash sensitivity
				qint64 d = tP.distanceWeighted<3, 2, 1024*1024*1024>(*pN);
				if (d < Transaction::LIMIT_DIST_TRANS
						&& pN->amountInt() < 0 && !pN->isInternal()) {
					if (d < bestDist) {
						bestDist = d;
						pBestMatchN = pN;
					}
				}
			}
			if (pT && pBestMatchN) {
				pT->flags |= Transaction::Flag::Internal;
				pBestMatchN->flags |= Transaction::Flag::Internal;
				LOG() << "Matching internal transactions" << endl;
				LOG() << pT->name << pT->amountDbl() << "" << pT->date.toString() << endl;
				LOG() << pBestMatchN->name << pBestMatchN->amountDbl() << "" << pBestMatchN->date.toString() << endl;
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

	emit injected(this);

	CostRateMonthPercentileMetric<2, 50>::get(this)->value(m_today);
	CostRateMonthPercentileMetric<2, 75>::get(this)->value(m_today);
	CostRateMonthPercentileMetric<2, 90>::get(this)->value(m_today);
	CostRateMonthPercentileMetric<2, 95>::get(this)->value(m_today);
	CostRateMonthPercentileMetric<2, 99>::get(this)->value(m_today);
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
	QJsonDocument jsonDoc(QJsonDocument::fromJson(jsonStr.toUtf8()));
	const QJsonObject& jsonObj = jsonDoc.object();

	LOG() << "User::injectJsonBot " << QJsonDocument(jsonObj).toJson() << endl;

	{
		QFile sampleReturn("jsonBot.json");
		sampleReturn.open(QFile::WriteOnly | QFile::Truncate);
		QTextStream fileout(&sampleReturn);
		fileout << jsonDoc.toJson(QJsonDocument::Indented);
	}

	// remake the context just in case
	makeBotContext();
	// makes the bot and build the tree inside
	m_bestBot = new Bot(jsonObj, this);
	m_bestBot->init(m_botContext);

	emit botInjected(m_bestBot);
}




