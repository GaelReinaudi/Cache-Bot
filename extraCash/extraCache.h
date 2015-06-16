#ifndef EXTRACACHE_H
#define EXTRACACHE_H

#include "cacheAccountConnector.h"
#include <QThread>

class ExtraCache : public CacheAccountConnector
{
	//Q_OBJECT

public:
	ExtraCache(QString userID);
	virtual ~ExtraCache() {};

	double checkingBalance() const { return user()->balance(Account::Type::Checking); }
	double slushNeed() const { return m_slushFundTypicalNeed; }
	double slushBaseStart() const { return m_slushFundStartsAt; }
	double minSlope() const { return m_minSlope; }
	int futDayMinSlope() const { return m_futDayMinSlopeCollision; }


protected:
	void onUserInjected(User* pUser) override;
	void onBotInjected() override;
	void onRepliedExtraCache(QString strData) override;

private:
	void computeMinSlopeOver(int numDays);

private:
	QDate m_date;
	int m_d0 = 0;
	double m_minSlope = 0.0;
	int m_futDayMinSlopeCollision = 0;
	double m_slushFundStartsAt = 0.0;
	double m_slushFundTypicalNeed = 1000.0;
	SparkLine m_spark;
public:
	bool sendExtraCash = true;
};

#endif // EXTRACACHE_H
