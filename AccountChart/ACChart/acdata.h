#ifndef ACDATA_H
#define ACDATA_H

#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>

class Transaction
{
public:
	Transaction();
	Transaction(double price, QDateTime time, QString &descr)
		: m_price(price)
		, m_time(time)
		, m_description(descr)
	{
	}

	//! json in
	void read(const QJsonObject &json)
	{
		m_price = json["price"].toDouble();
		m_time = QDateTime::fromTime_t(json["time"].toInt());
		m_description = json["descr"].toString();
	}
	//! json out
	void write(QJsonObject &json) const
	{
		json["price"] = m_price;
		json["time"] = int(m_time.toTime_t());
		json["descr"] = m_description;
	}

private:
	double m_price = 0.0;
	QDateTime m_time;
	QString m_description;
};

class Transactions
{
public:
	Transactions();

	//! json in
	void read(const QJsonObject &json)
	{
		m_trans.clear();
		QJsonArray npcArray = json["transactions"].toArray();
		for (int npcIndex = 0; npcIndex < npcArray.size(); ++npcIndex) {
			QJsonObject npcObject = npcArray[npcIndex].toObject();
			Transaction tra;
			tra.read(npcObject);
			m_trans.append(tra);
		}
	}
	//! json out
	void write(QJsonObject &json) const
	{
		QJsonArray npcArray;
		foreach (const auto tra, m_trans) {
			QJsonObject npcObject;
			tra.write(npcObject);
			npcArray.append(npcObject);
		}
		json["transactions"] = npcArray;
	}
private:
	QList<Transaction> m_trans;
};

class ACData
{
public:
	ACData();
};

#endif // ACDATA_H
