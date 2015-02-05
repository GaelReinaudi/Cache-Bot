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

class Transaction
{
public:
	enum Type{InternalTransfer = -1, None = 0};
	Transaction() {}
	Transaction(double amount, QDateTime startDate, double numDays, QString &descr)
		: m_amount(amount)
		, m_numDays(numDays)
		, m_startDate(startDate)
		, m_description(descr)
	{
	}
	Transaction(const Transaction& other) {
		m_amount = other.m_amount;
		m_startDate = other.m_startDate;
		m_numDays = other.m_numDays;
		m_description = other.m_description;
		// fresh un-accounted-for copy
		m_isAccountedFor = 0;
	}

	//! json in
	void read(const QJsonObject &json);
	//! json out
	void write(QJsonObject &json) const {
		json["amount"] = m_amount;
		json["startDate"] = int(m_startDate.toTime_t());
		json["numDays"] = m_numDays;
		json["descr"] = m_description;
	}

	double amount() const{
		return m_amount;
	}
	double compressedAmount() const{
		return kindaLog(m_amount);
	}
	double numDays() const{
		return m_numDays;
	}
	uint time() const{
		return m_startDate.toTime_t();
	}

	QDateTime startDate() const {
		return m_startDate;
	}

	void accountFor() {
		++m_isAccountedFor;
	}
	void resetAccountFor() {
		m_isAccountedFor = 0;
	}
	bool isAccountedFor() const {
		return m_isAccountedFor;
	}

public:
	static bool earlierThan(const Transaction& first, const Transaction& second) {
		return first.startDate() < second.startDate();
	}

	int type() const {
		return m_type;
	}

private:
	int m_type = None;
	double m_amount = 0.0;
	double m_numDays = 1.0;
	QDateTime m_startDate;
	QString m_description;
	char m_isAccountedFor = 0;
};

class Transactions
{
public:
	Transactions() {}

	//! json in
	void read(const QJsonObject &json, const QVector<QString>& acIds);
	//! json out
	void write(QJsonObject &json) const {
		QJsonArray npcArray;
		foreach (const auto tra, m_transList) {
			QJsonObject npcObject;
			tra.write(npcObject);
			npcArray.append(npcObject);
		}
		json["purchases"] = npcArray;
	}

	QVector<Transaction>& list() {
		return m_transList;
	}
	void resetAccountFor() {
		for (auto& trans : list()) {
			trans.resetAccountFor();
		}
	}

public:
	static bool isSymetric(const Transaction& first, const Transaction& second) {
		if (first.startDate() == second.startDate()) {
			if (first.amount() == -second.amount()) {
				return true;
			}
		}
		return false;
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

typedef QVector<QVector<Transaction> > DailyTransactions;

#endif // ACDATA_H
