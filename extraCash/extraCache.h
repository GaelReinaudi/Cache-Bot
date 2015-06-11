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
//	int computeMinSlopeOver(int numDays)
//	{
//		m_minSlope = 9999.9;
//		int dayMin = -1;
//		double tToday = m_date.toJulianDay() - m_d0;
//		double yToday = m_slushThreshold;
//		QCPDataMap *pDat = ui->plot->graph(2)->data();
//		QMap<double, QCPData>::iterator it = pDat->begin();
//		while(it != pDat->end() && it->key < tToday + numDays + 1) {
//			if (it->key > tToday) {
//				double effectiveSlushforDay = slushAmmount * (0.5 + 1.0 * (it->key - tToday) / 30.0);
//				double y = it->value - effectiveSlushforDay;
//				double slope = (y - yToday) / qMax(1.0, it->key - tToday);
//				if(slope < m_minSlope) {
//					m_minSlope = slope;
//					dayMin = it->key;
//				}
//			}
//			++it;
//		}
//		if (m_minSlope == 9999.9) {
//			double effectiveSlushforDay = slushAmmount * (0.5 + 1.0 * (numDays) / 30.0);
//			m_minSlope = (m_lastBal - effectiveSlushforDay - m_slushThreshold) / numDays;// / 2.0;
//			dayMin = tToday + numDays;
//		}
//		return dayMin;
//	}

private:
	QDate m_date;
	int m_d0 = 0;
	double m_minSlope = 0.0;
	SparkLine m_spark;
};

#endif // EXTRACACHE_H
