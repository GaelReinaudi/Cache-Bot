#ifndef EXTRACACHE_H
#define EXTRACACHE_H

#include "cacheAccountConnector.h"
#include <QThread>

class extraCache : public CacheAccountConnector
{
	Q_OBJECT

public:
	extraCache(QString userID);

protected:
	void onUserInjected() override;
	void onBotInjected() override;

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
