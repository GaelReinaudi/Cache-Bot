#ifndef ACDATA_H
#define ACDATA_H

#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFileInfo>
#include <QDebug>

class Transaction
{
public:
	Transaction() {}
	Transaction(double amount, QDateTime startDate, QString &descr)
		: m_amount(amount)
		, m_startDate(startDate)
		, m_description(descr)
	{
	}

	//! json in
	void read(const QJsonObject &json) {
		m_amount = json["amount"].toString().toDouble();
		uint sec = json["startDate"].toString().toDouble();
		m_startDate = QDateTime::fromTime_t(sec);
		m_description = json["descr"].toString();
		qDebug() << m_amount << sec << m_startDate << m_description;
	}
	//! json out
	void write(QJsonObject &json) const {
		json["amount"] = m_amount;
		json["startDate"] = int(m_startDate.toTime_t());
		json["descr"] = m_description;
	}

	double amount() const{
		return m_amount;
	}
	double compressedAmount() const{
		if (m_amount < 0)
			return -log10(-m_amount+1.0);
		return log10(m_amount+1.0);
	}
	uint time() const{
		return m_startDate.toTime_t();
	}

private:
	double m_amount = 0.0;
	QDateTime m_startDate;
	QString m_description;
};

class Transactions
{
public:
	Transactions() {}

	//! json in
	void read(const QJsonObject &json) {
		m_transList.clear();
		QJsonArray npcArray = json["purchases"].toArray();
		for (int npcIndex = 0; npcIndex < npcArray.size(); ++npcIndex) {
			QJsonObject npcObject = npcArray[npcIndex].toObject();
			Transaction tra;
			tra.read(npcObject);
			m_transList.append(tra);
		}
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

#endif // ACDATA_H
