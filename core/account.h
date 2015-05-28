#ifndef ACDATA_H
#define ACDATA_H

#include "common.h"
#include "transaction.h"

typedef QMap<uint, TransactionBundle*> HashedBundles;

class CORESHARED_EXPORT Account : public DBobj
{
public:
	Account();

	// loading the json file
	// see this: https://qt-project.org/doc/qt-5-snapshot/qtcore-savegame-example.html
	bool loadPlaidJson(QString jsonFile, int afterJday = 0, int beforeJday = 0);
	void loadJsonData(QByteArray jsonData, int afterJday = 0, int beforeJday = 0);
	bool toJson(QVector<Transaction> transactions, QString category);

	QMap<uint, TransactionBundle*>& hashBundles() {
		return m_hashBundles;
	}

	TransactionBundle& allTrans(int filterHash = -1) {
		if (filterHash >= 0)
			return *m_hashBundles[filterHash];
		return m_allTrans;
	}
	QDate lastTransactionDate() {
		return m_allTransactions.transArray()[m_allTransactions.count() - 1].date;
	}
	QDate firstTransactionDate() {
		return m_allTransactions.transArray()[0].date;
	}
	// gives the daily average of outcome for the withinPercentileCost outgoing transactions
	double costLiving(double withinPercentileCost);

private:
	QVector<QString> m_accountIds;
	struct Transactions
	{
		Transactions() {}
		//! json in
		void read(const QJsonArray& npcArray, int afterJday = 0, int beforeJday = 0, const QVector<QString>& onlyAcIds = QVector<QString>());
		//! json out
		void write(QJsonArray &npcArray) const;
		Transaction* transArray() { return &m_transArray[0]; }
		void clear() { m_numTrans = 0; }
		int count() const { return m_numTrans; }
		Transaction* appendNew() { return &m_transArray[m_numTrans++]; }
		Transaction* last() { return &m_transArray[m_numTrans - 1]; }
		void removeLast() { m_numTrans--; }
	private:
		std::array<Transaction, MAX_TRANSACTION_PER_ACCOUNT> m_transArray;
		int m_numTrans = 0;
	};
	//! makes a bundle for each hash value
	void makeHashBundles() {
		for (int i = 0; i < allTrans().count(); ++i) {
			Transaction& t = allTrans().trans(i);
			uint h = t.nameHash.hash;
			if (!m_hashBundles.contains(h))
				m_hashBundles[h] = new TransactionBundle();
			m_hashBundles[h]->append(&t);
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
	QString m_jsonFilePath = "predicted";
	Transactions m_allTransactions;
	TransactionBundle m_allTrans;
	HashedBundles m_hashBundles;

public:
	// for predicted transcations
	Transactions m_predicted;
};

class Household : public QObject
{
	QVector<Account*> m_accounts;
};

#endif // ACDATA_H
