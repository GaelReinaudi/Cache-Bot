#ifndef FUND_H
#define FUND_H
#include "common.h"

struct Cash
{
	QDateTime created;
	double amount;
};

class CORESHARED_EXPORT Fund : public DBobj
{
public:
	Fund(QJsonObject jsonFund, QObject* parent = 0)
		:DBobj(jsonFund["_id"].toString(), parent)
	{
		Q_ASSERT(!jsonFund["_id"].toString().isEmpty());
		Q_ASSERT(!jsonFund["name"].toString().isEmpty());
		m_name = jsonFund["name"].toString();
	}

	void populate(QJsonArray jsonCashes) {
		for (int i = 0; i < jsonCashes.size(); ++i) {
			QJsonObject jCsh = jsonCashes.at(i).toObject();
			QString fund_id = jCsh["fund_id"].toString();
			Cash newCash;
			newCash.amount = jCsh["amount"].toDouble();
			newCash.created = QDateTime::fromString(jCsh["createdAt"].toString(), Qt::ISODate);
			if (fund_id == id()) {
				m_cashes.append(newCash);
			}
		}
	}

	QString name() const{
		return m_name;
	}
	QList<Cash> cashes() const{
		return m_cashes;
	}

private:
	QString m_name;
	QList<Cash> m_cashes;
};

#endif // FUND_H
