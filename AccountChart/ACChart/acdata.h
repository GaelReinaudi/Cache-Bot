#ifndef ACDATA_H
#define ACDATA_H

#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFileInfo>
#include <QDebug>
#include <cmath>
#include <QRectF>

double kindaLog(double amount);
QRectF kindaLog(QRectF rectLinear);
unsigned int proximityHashString(const QString& str);

#define MAX_HASH_LENGTH 64

class Account;


struct Transaction
{
	Account* account = nullptr;
	QString id; // "pKowox9EaKF14mBJ71m3hnmoPgA3Q0T4rjDox"
	QString name; // "YARROW HOTEL GRILL" or "STRIKE TECHNOLOG"
	double amount = 0.0;
	int kamount = 0; // integer = round(amount * 1024)
	QDate date; // "2015-01-28"
	QStringList categories; // ["Food and Drink", "Restaurants"] or ["Transfer", "Payroll"]
	union {
		unsigned int hash = 0;
		uchar b[4];
	} nameHash;

	//! json in
	void read(const QJsonObject &json);
	//! json out
	void write(QJsonObject &json) const;

	double time_t() const {
		return (3600.0 * 24.0) * (double(date.toJulianDay()) + 0.5);
	}
	double amountDbl() const {
		return amount;
	}
	double compressedAmount() const{
		return kindaLog(amountDbl());
	}
	double numDays() const{
		return 1;
	}
	static bool earlierThan(const Transaction first, const Transaction second) {
		return first.date < second.date;
	}
};

class Transactions
{
public:
	Transactions() {}

	//! json in
	void read(const QJsonObject &json, const QVector<QString>& acIds);
	//! json out
	void write(QJsonObject &json) const;

public:
	static bool isSymetric(const Transaction& first, const Transaction& second) {
		if (first.date == second.date) {
			if (first.amountDbl() == -second.amountDbl()) {
				return true;
			}
		}
		return false;
	}

	QVector<Transaction>& list() {
		return m_transList;
	}

private:
	QVector<Transaction> m_transList;
};

class Account
{
public:
	Account() {}

	// loading the json file
	// see this: https://qt-project.org/doc/qt-5-snapshot/qtcore-savegame-example.html
	bool load(QString jsonFile);

	Transactions& transactions() {
		return m_transactions;
	}


private:
	QVector<QString> m_accountIds;
	Transactions m_transactions;
};

class Household
{
	QVector<Account> m_accounts;
};

typedef QVector<QVector<Transaction*> > DailyTransactions;

#endif // ACDATA_H
