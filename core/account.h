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

	void append(Transaction* pT) {
		m_allTrans.append(pT);
	}

//	// gives the daily average of outcome for the withinPercentileCost outgoing transactions
//	double costLiving(double withinPercentileCost);

private:
	QString m_plaidId;
	QString m_jsonFilePath = "predicted";
	TransactionBundle m_allTrans;
};

//class Household : public QObject
//{
//	QVector<User*> m_users;
//};

#endif // ACDATA_H
