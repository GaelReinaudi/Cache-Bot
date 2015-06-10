#ifndef USER_H
#define USER_H

#include "core_global.h"
#include "common.h"
#include "transaction.h"
#include "account.h"
//class Account;

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
	User(QString userId, QObject* parent = 0)
		:DBobj(userId, parent)
	{}

	// gives the daily average of outcome for the withinPercentileCost outgoing transactions
	double costLiving(double withinPercentileCost);

	TransactionBundle& transBundle(int filterHash) {
		if (filterHash >= 0)
			return *m_hashBundles[filterHash];
		return m_allTransBundle;
	}
	QMap<uint, TransactionBundle*>& hashBundles() {
		return m_hashBundles;
	}
	StaticTransactionArray& allTrans() {
		return m_allTransactions;
	}
	// TEMP
	QVector<Transaction*> predictedTransactions() { return QVector<Transaction*>(); }

public slots:
	void injectJsonData(QString jsonStr);
	void injectJsonBot(QString jsonStr);

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
	void injected();

private:
	//! makes a bundle for each hash value
	void makeHashBundles() {
		for (int i = 0; i < m_allTransactions.count(); ++i) {
			Transaction& t = m_allTransactions.trans(i);
			qint64 h = t.nameHash.hash;
			if (!m_hashBundles.contains(h))
				m_hashBundles[h] = new TransactionBundle();
			m_hashBundles[h]->append(&t);
			m_allTransBundle.append(&t);
		}
		// assigns the index of the hash to the transactions
		for (int i = 0; i < hashBundles().count(); ++i) {
			int h = hashBundles().keys()[i];
			for (int j = 0; j < hashBundles()[h]->count(); ++j) {
				hashBundles()[h]->trans(j).indexHash = i;
			}
		}

		//qDebug() << m_hashBundles.count() << m_hashBundles.keys().first() << m_hashBundles.keys().last();
	}

private:
	QVector<Bank*> m_banks;
	QVector<Account*> m_accounts;
	StaticTransactionArray m_allTransactions;
	HashedBundles m_hashBundles;
	TransactionBundle m_allTransBundle;
};

#endif // USER_H
