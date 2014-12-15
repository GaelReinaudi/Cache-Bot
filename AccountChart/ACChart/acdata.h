#ifndef ACDATA_H
#define ACDATA_H

#include <QDateTime>
#include <QJsonObject>

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

class ACData
{
public:
	ACData();
};

#endif // ACDATA_H
