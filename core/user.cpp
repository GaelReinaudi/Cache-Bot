#include "user.h"
#include "bot.h"
#include "botContext.h"
#include "fund.h"
#include "userMetrics.h"
#include "oracle.h"

double User::declaredIncome = 0.0;
double User::declaredRent = 0.0;
double User::balanceAdjust = 0.0;
double User::totalAdjustedBalance = 0.0;

User::User(QString userId, QJsonObject jsonArgs)
	: DBobj(userId, 0)
{
	BotContext::JSON_ARGS = jsonArgs;
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

double User::littleIncome()
{
	SuperOracle::Summary s = smallSummary();
	double flow = s.flow();
	double flowFix = s.flowFix();
	if (s.posSum == 0)
		return 0.01;
	if (s.salary <= 10)
		return 0.01;
//	if (flowFix < -2.0) {
//		double fac = qSqrt(qMax(0.0, flowFix + 3.0));
//		DBG(3) << "little fixed income: fac " << fac;
//		return fac;
//	}
	if (flow < -0.40) {
		double fac = qSqrt(qMax(0.0, flow + 1.0));
		DBG(3) << "little income: fac " << fac;
		return fac;
	}
	return 1.0;
}

double User::matchesDeclaredIncome()
{
	SuperOracle::Summary s = smallSummary();
	double fixInc = s.salary;
	if (declaredIncome) {
		double match = qAbs(fixInc - declaredIncome) / declaredIncome;
		match = 2.0 * qExp(-2.0 * match);
		if (match >= 0.5) {
			//NOTICE() << "declaredIncome matches: " << match;
			return match;
		}
		else {
			//NOTICE() << "not matches declaredIncome: " << declaredIncome << " fixInc:"<<fixInc<<" ";
		}
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

	//////// "user"
	QJsonObject jsonUser = jsonObj["user"].toObject();
	m_email = jsonUser["local"].toObject()["email"].toString();
	declaredIncome = jsonUser["declaredMonthlyIncome"].toDouble() / (365.25 / 12.0);
	declaredRent = jsonUser["declaredMonthlyRent"].toDouble() / (365.25 / 12.0);
	QDate firstCommonTransDate = QDate::fromString(jsonUser["firstCommonTransDate"].toString().left(10), "yyyy-MM-dd");
	int oldEnoughForTransfer = Transaction::onlyAfterDate.daysTo(Transaction::currentDay())-9;
	if (Transaction::onlyAfterDate < firstCommonTransDate) {
		int firstCommmonAgo = int(firstCommonTransDate.daysTo(Transaction::currentDay()));
		if (firstCommmonAgo > 60) {
			Transaction::onlyAfterDate = firstCommonTransDate;
		}
		if (firstCommmonAgo > 99) {
			oldEnoughForTransfer = firstCommmonAgo - 9;
//			Transaction::onlyAfterDate = firstCommonTransDate.addDays(9);
		}
	}
	NOTICE() << "declaredIncome: " << declaredIncome;
	qDebug() << "user" << jsonUser["_id"].toString() << ":" << jsonUser["local"].toObject()["email"].toString();
	Q_ASSERT_X(jsonUser["_id"].toString() == id(), "injectJsonData", jsonUser["_id"].toString().toUtf8() + " != " + id().toUtf8());

	//////// "flow"
	jsonInFlow = jsonObj["flow"].toObject();

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
		if (pInAcc) {
//			if (pInAcc->type() == Account::Type::Checking)
				m_allTransactions.appendNew(jsonTrans, pInAcc);
		}
	}
	INFO() << "maxDaysOldAllTransatcion " << Transaction::maxDaysOldAllTransatcion();
	m_allTransactions.sort();

	WARN() << "daysWithoutCreateDate " << Transaction::daysWithoutCreateDate();
	if (Transaction::daysWithoutCreateDate() > 0) {
//		WARN() << "setting current day to " << Transaction::daysWithoutCreateDate() << " days ago";
//		Transaction::setCurrentDay(Transaction::actualCurrentDay().addDays(-Transaction::daysWithoutCreateDate()));
	}

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
				// dist max 5 days, 2 parts of kla, and no hash sensitivity
				qint64 d = tP.distanceWeighted<5, 2, 1024*1024*1024>(*pN);
				if (d < Transaction::LIMIT_DIST_TRANS
						&& pN->amount() < 0 && !pN->isInternal()
						&& (pT->account != pN->account || pN->userFlag & Transaction::UserInputFlag::IsMatchOff)) {
					if (d < bestDist) {
						bestDist = d;
						pBestMatchN = pN;
					}
				}
			}
			if (pT && pBestMatchN) {
				pT->flags |= Transaction::Flag::Internal;
				pBestMatchN->flags |= Transaction::Flag::Internal;
				if (pT->account->type() == Account::Type::Checking && pBestMatchN->account->type() != Account::Type::Checking) {
					if (pT->amount() > 0) {
						pT->checkPOV |= Transaction::CheckingPOV::FromOtherAcc;
						pBestMatchN->checkPOV |= Transaction::CheckingPOV::OtherToChecking;
					}
					else {
						pT->checkPOV |= Transaction::CheckingPOV::ToOtherAcc;
						pBestMatchN->checkPOV |= Transaction::CheckingPOV::OtherFromChecking;
					}
				}
				else if (pBestMatchN->account->type() == Account::Type::Checking && pT->account->type() != Account::Type::Checking) {
					if (pBestMatchN->amount() > 0) {
						pBestMatchN->checkPOV |= Transaction::CheckingPOV::FromOtherAcc;
						pT->checkPOV |= Transaction::CheckingPOV::OtherToChecking;
					}
					else {
						pBestMatchN->checkPOV |= Transaction::CheckingPOV::ToOtherAcc;
						pT->checkPOV |= Transaction::CheckingPOV::OtherFromChecking;
					}
				}
				else {// if (pT->account->type() != Account::Type::Checking && pBestMatchN->account->type() != Account::Type::Checking) {
					pT->checkPOV |= Transaction::CheckingPOV::OtherToOther;
					pBestMatchN->checkPOV |= Transaction::CheckingPOV::OtherToOther;
				}
				INFO() << "Matching internal transactions";
				INFO() << pT->name << " " << pT->amountDbl() << " " << pT->date.toString();
				INFO() << pBestMatchN->name << " " << pBestMatchN->amountDbl() << " " << pBestMatchN->date.toString();
			}
		}
	}
	//////// "exclude Digit and plaid's label credit card payement
	for (int i = 0; i < m_allTransactions.count(); ++i) {
		Transaction* pT = &m_allTransactions.transArray()[i];
		if (pT->isInternal())
			continue;
		if (pT->name.contains("Hello Digit", Qt::CaseInsensitive)) {
			NOTICE() << "making Digit internal";
//			pT->flags |= Transaction::Flag::Internal;
			if (pT->amount() > 0)
				pT->checkPOV |= Transaction::CheckingPOV::ToDigitLike;
			else
				pT->checkPOV |= Transaction::CheckingPOV::FromDigitLike;
		}
		else {
			int dayOld = pT->date.daysTo(Transaction::currentDay());
			if (qAbs(pT->categoryHash.hash()) == 16001000
					&& (dayOld < 2 || dayOld > oldEnoughForTransfer)) {
				WARN() << "making hash 16001000 internal: " << pT->name << " " << pT->amountDbl() << " " << pT->date.toString();
				pT->flags |= Transaction::Flag::UnMatchedInternal;
			}
			if (qAbs(pT->categoryHash.hash()) == 16000000
					&& (dayOld < 2 || dayOld > oldEnoughForTransfer)) {
				WARN() << "making hash 16000000 internal: " << pT->name << " " << pT->amountDbl() << " " << pT->date.toString();
				pT->flags |= Transaction::Flag::UnMatchedInternal;
			}
			if (pT->isInternal()) {
				User::balanceAdjust -= pT->amountDbl();
				NOTICE() << "transaction 1600**** "
					  << pT->name << " " << pT->amountDbl() << " " << pT->date.toString() << " balanceAdjust = " << User::balanceAdjust;
			}
		}
	}
	User::totalAdjustedBalance = balance(Account::Type::Checking);//All) + User::balanceAdjust;
	//////// mark as "return" in certain conditions
	for (int i = 0; i < m_allTransactions.count(); ++i) {
		Transaction* pT = &m_allTransactions.transArray()[i];
		if (pT->isInternal())
			continue;
		if (pT->account->type() == Account::Type::Credit && pT->kla() > 0) {
			pT->isReturn = true;
			WARN() << "this look like a return: " << pT->name << " " << pT->amountDbl() << " " << pT->date.toString();
		}
	}
	//////// flag as EO/EI/IO/II
	for (int i = 0; i < m_allTransactions.count(); ++i) {
		Transaction* pT = &m_allTransactions.transArray()[i];
		if (!pT->isInternal()) {
			if (pT->account->type() == Account::Type::Checking) {
				if (pT->amount() > 0)
					pT->checkPOV |= Transaction::CheckingPOV::FromExterior;
				else
					pT->checkPOV |= Transaction::CheckingPOV::ToExterior;
			}
			else {
				if (pT->amount() > 0)
					pT->checkPOV |= Transaction::CheckingPOV::OtherExterior;
				else
					pT->checkPOV |= Transaction::CheckingPOV::OtherExterior;
			}
//			WARN() << "checkPOV: " << pT->checkPOV << " " << pT->name << " " << pT->amountDbl() << " " << pT->date.toString();
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
	QString v2 = QString(GIT_VERSION).left(1);
	QString botStr = "bestBot";
	if (v2.toInt() > 1)
		botStr += v2;
	const QJsonObject& jsonObj = jsonDoc.object()[botStr].toObject();

	INFO() << "User::injectJsonBot " << botStr << " " << QString(QJsonDocument(jsonObj).toJson());

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

void User::makeHashBundles() {
	for (int i = 0; i < m_allTransactions.count(); ++i) {
		Transaction& t = m_allTransactions.trans(i);
		qint64 h = t.nameHash.hash();
		if (!m_hashBundles.contains(h))
			m_hashBundles[h] = new TransactionBundle();
		m_hashBundles[h]->append(&t);

		// categories too
		h = t.categoryHash.hash();
		if (!m_hashBundles.contains(h))
			m_hashBundles[h] = new TransactionBundle();
		m_hashBundles[h]->append(&t);

		// all of them
		m_allTransBundle.append(&t);
	}
	//qDebug() << m_hashBundles.count() << m_hashBundles.keys().first() << m_hashBundles.keys().last();
}

TransactionBundle& User::makeFlagBundle(int filterHash, int flags)
{
	WARN() << "filterHash=" << filterHash << ". making flagBundle: flags=" << flags;
	HashedBundles& hashBund = m_flagHashBundles[flags];
	TransactionBundle& allBund = m_flagTransBundle[flags];
	for (int i = 0; i < m_allTransactions.count(); ++i) {
		Transaction& t = m_allTransactions.trans(i);
		qint64 h = t.nameHash.hash();
		if (!hashBund.contains(h))
			hashBund[h] = new TransactionBundle();
		if (t.checkPOV & flags)
			hashBund[h]->append(&t);

		// categories too
		h = t.categoryHash.hash();
		if (!hashBund.contains(h))
			hashBund[h] = new TransactionBundle();
		if (t.checkPOV & flags)
			hashBund[h]->append(&t);

		// all of them
		if (t.checkPOV & flags)
			allBund.append(&t);
	}
	WARN() << "& flags=" << flags << " count = " << allBund.count();
	if (filterHash != -1)
		return *hashBund[filterHash];
	return allBund;
}

