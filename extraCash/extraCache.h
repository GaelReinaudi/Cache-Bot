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

protected:
	void onUserInjected(User* pUser) override;
	void onBotInjected() override;
	void onRepliedExtraCache(QString strData) override;

private:
	int computeMinSlopeOver(int numDays);

private:
	QDate m_date;
	int m_d0 = 0;
	double m_minSlope = 0.0;
	double m_slushFundStartsAt = 0.0;
	double m_slushFundTypicalNeed = 1000.0;
	SparkLine m_spark;
};

#endif // EXTRACACHE_H
