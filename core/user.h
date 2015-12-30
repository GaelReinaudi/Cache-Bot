#ifndef USER_H
#define USER_H

#include "core_global.h"
#include "common.h"
#include "transaction.h"
#include "account.h"
#include "oracle.h"
class Bot;
class BotContext;
class Fund;

typedef QMap<int, double> SparkLine;

class CORESHARED_EXPORT Bank : public DBobj
{
public:
	Bank(QJsonObject jsonBank, QObject* parent = 0)
		:DBobj(jsonBank["_id"].toString(), parent)
	{
		Q_ASSERT(!jsonBank["_id"].toString().isEmpty());
		m_accessToken = jsonBank["_id"].toString();
	}
	QString accessToken() const { return m_accessToken; }

private:
	QString m_accessToken;
};

class CORESHARED_EXPORT User : public DBobj
{
	Q_OBJECT

public:
	User(QString userId, QJsonObject jsonArgs = QJsonObject());
	~User();

public:
	TransactionBundle& transBundle(int filterHash) {
		if (filterHash != -1)
			return *m_hashBundles[filterHash];
		return m_allTransBundle;
	}
	QMap<qint64, TransactionBundle*>& hashBundles() {
		return m_hashBundles;
	}
	TransactionBundle& allTrans() {
		return m_allTransBundle;
	}
	void resetDimensionOfVoid() {
		for(int i = 0; i < m_allTransactions.count(); ++i) {
			// reset the dimensionOfVoid
			m_allTransactions.trans(i).dimensionOfVoid = 0;
		}
		Transaction::s_hypotheTrans.dimensionOfVoid = 0;
	}

	double balance(int flagType) const {
		double bal = 0.0;
		for (Account* pAcc : m_accounts) {
			if (pAcc->type() & flagType) {
				bal += pAcc->balance();
			}
		}
		if (flagType & Account::Type::Checking) {
			bal += Transaction::s_hypotheTrans.amountDbl();
		}
		return bal;
	}

	Fund *extraCacheFund() const {
		return m_extraCacheFund;
	}

	QString email() const{
		return m_email;
	}

	BotContext *botContext() const{
		return m_botContext;
	}
	BotContext* makeBotContext();
	SuperOracle* oracle();
	SuperOracle::Summary smallSummary();
	double littleIncome();

public slots:
	bool setHypotheTrans(int amount = 0);
	void injectJsonData(QString jsonStr);
	void injectJsonBot(QString jsonStr);
	//! simulates a re-injection of the bot in order ot recalculate downstream slots
	void reInjectBot() { emit botInjected(m_bestBot); }
	void reComputeBot();

protected:
	Bank* getBankByToken(QString bankTok) const {
		for (Bank* pB : m_banks) {
			if(pB->accessToken() == bankTok)
				return pB;
		}
		return 0;
	}
	Account* getAccountByPlaidId(QString plaidId) const {
		for (Account* pA : m_accounts) {
			if(pA->plaidId() == plaidId)
				return pA;
		}
		return 0;
	}

signals:
	void injected(User* pUser);
	void botInjected(Bot* bestBot);

private:
	//! makes a bundle for each hash value
	void makeHashBundles() {
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

private:
	QVector<Bank*> m_banks;
	QVector<Account*> m_accounts;
	StaticTransactionArray m_allTransactions;
	HashedBundles m_hashBundles;
	TransactionBundle m_allTransBundle;
	Bot* m_bestBot = 0;
	BotContext* m_botContext = 0;
	Fund* m_extraCacheFund = 0;
	QString m_email;
	SuperOracle* m_mainOracle = 0;
};

#endif // USER_H
