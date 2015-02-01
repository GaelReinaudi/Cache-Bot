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
	void read(const QJsonObject &json) {
		bool ok;
		m_amount = json["amount"].toString().toDouble(&ok);
		uint sec = json["startDate"].toString().toDouble(&ok);
		if(ok)
			m_startDate = QDateTime::fromTime_t(sec);
		else {
			m_amount = -json["amount"].toDouble();
			m_startDate = QDateTime::fromString(json["date"].toString(), "yyyy-MM-dd");
		}
		m_numDays = json["numDays"].toString().toDouble(&ok);
		if(!ok)
			m_numDays = 1;
		m_description = json["descr"].toString();
		qDebug() << m_amount << sec << m_startDate << m_numDays << m_description;
	}
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

private:
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
	void read(const QJsonObject &json) {
		m_transList.clear();
		QJsonArray npcArray = json["transactions"].toArray();
		for (int npcIndex = 0; npcIndex < npcArray.size(); ++npcIndex) {
			QJsonObject npcObject = npcArray[npcIndex].toObject();
			Transaction tra;
			tra.read(npcObject);
			m_transList.append(tra);
		}
		QJsonArray npcArrayOld = json["purchases"].toArray();
		for (int npcIndex = 0; npcIndex < npcArrayOld.size(); ++npcIndex) {
			QJsonObject npcObject = npcArrayOld[npcIndex].toObject();
			Transaction tra;
			tra.read(npcObject);
			m_transList.append(tra);
		}
		qSort(m_transList.begin(), m_transList.end(), Transaction::earlierThan);
		qDebug() << "m_transList count" << m_transList.size();
	}
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

	QList<Transaction>& list() {
		return m_transList;
	}
	void resetAccountFor() {
		for (auto& trans : list()) {
			trans.resetAccountFor();
		}
	}

private:
	QList<Transaction> m_transList;
};

class Account
{
public:
	Account() {}

	// loading the json file
	// see this: https://qt-project.org/doc/qt-5-snapshot/qtcore-savegame-example.html
	bool load(QString jsonFile) {
		QFile loadFile(jsonFile);
		if (!loadFile.open(QIODevice::ReadOnly)) {
			qWarning(QString("Couldn't open file %1").arg(QFileInfo(loadFile).absoluteFilePath()).toUtf8());
			return false;
		}
		QByteArray saveData = loadFile.readAll();
		QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
		m_transactions.read(loadDoc.object());
		return true;
	}

	Transactions& transactions() {
		return m_transactions;
	}


private:
	Transactions m_transactions;
};

typedef QVector<QVector<Transaction> > DailyTransactions;

#endif // ACDATA_H
