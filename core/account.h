#ifndef ACDATA_H
#define ACDATA_H

#include "common.h"
#include "transaction.h"

typedef QMap<qint64, TransactionBundle*> HashedBundles;

class CORESHARED_EXPORT Account : public DBobj
{
public:
	enum Type{Unknown = 0x0
			  , Checking = 0x1
			  , Saving = 0x2
			  , Credit = 0x4
			 , All = 0xffff};
	Account(QJsonObject jsonAcc, QObject* parent = 0);
	~Account() {}

	const QStringList& excludeNameTransContain() const {
		return m_excludeNameTransContain;
	}

	// loading the json file
	// see this: https://qt-project.org/doc/qt-5-snapshot/qtcore-savegame-example.html
	bool loadPlaidJson(QString jsonFile);
	void loadJsonData(QJsonObject json);
	bool toJson(QVector<Transaction> transactions, QString category);

	const QString& plaidId() const { return m_plaidId; }

	void append(Transaction* pT) {
		m_allTrans.append(pT);
	}

	double balance() const {
		if (m_type == Type::Credit)
			return -m_balance;
		return m_balance;
	}

	Type type() const{ return m_type; }

	QString accountID;
private:
	QString m_plaidId;
	QString m_jsonFilePath = "predicted";
	TransactionBundle m_allTrans;
	double m_balance = 0.0;
	Type m_type = Type::Unknown;
	QStringList m_excludeNameTransContain;
};

//class Household : public QObject
//{
//	QVector<User*> m_users;
//};

#endif // ACDATA_H
