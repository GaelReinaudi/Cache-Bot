#ifndef ACDATA_H
#define ACDATA_H

#include "common.h"
#include "transaction.h"

typedef QMap<uint, TransactionBundle*> HashedBundles;

class CORESHARED_EXPORT Account : public DBobj
{
public:
	Account(QJsonObject jsonAcc, QObject* parent = 0);

	// loading the json file
	// see this: https://qt-project.org/doc/qt-5-snapshot/qtcore-savegame-example.html
	bool loadPlaidJson(QString jsonFile, int afterJday = 0, int beforeJday = 0);
	void loadJsonData(QJsonObject json, int afterJday = 0, int beforeJday = 0);
	bool toJson(QVector<Transaction> transactions, QString category);

	QString plaidId() const { return m_plaidId; }
	QMap<uint, TransactionBundle*>& hashBundles() {
		return m_hashBundles;
	}

	void append(Transaction* pT) {
		m_allTrans.append(pT);
	}

	TransactionBundle& allTrans(int filterHash = -1) {
		if (filterHash >= 0)
			return *m_hashBundles[filterHash];
		return m_allTrans;
	}
	// gives the daily average of outcome for the withinPercentileCost outgoing transactions
	double costLiving(double withinPercentileCost);

private:
	//! makes a bundle for each hash value
	void makeHashBundles() {
		for (int i = 0; i < allTrans().count(); ++i) {
			Transaction& t = allTrans().trans(i);
			qint64 h = t.nameHash.hash;
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
	QString m_plaidId;
	QString m_jsonFilePath = "predicted";
	TransactionBundle m_allTrans;
	HashedBundles m_hashBundles;

public:
	// for predicted transcations
	StaticTransactionArray m_predicted;
};

//class Household : public QObject
//{
//	QVector<User*> m_users;
//};

#endif // ACDATA_H
