#ifndef EXTRACACHE_H
#define EXTRACACHE_H

#include "cacheAccountConnector.h"
#include <QThread>

class ExtraCache : public CacheAccountConnector
{
public:
	ExtraCache(QString userID, QJsonObject jsonArgs);
	virtual ~ExtraCache() {};

	double slushNeed() const { return m_slushFundTypicalNeed; }
	double slushBaseStart() const { return m_slushFundStartsAt; }
	double minSlope() const { return m_minSlope; }
	int futDayMinSlope() const { return m_futDayMinSlopeCollision; }

	double calcSummary(Bot* bestBot, QJsonObject &statObj, int doAskThing = 0);

	void makeCategoryTreeSummary(Bot* bestBot, QStringList cats, QJsonObject &statObj, int level = 0);
	QJsonObject orderCategoryTree(QJsonObject &catObj);

protected:
	void onUserInjected(User* pUser) override;
	void onBotInjected(Bot *bestBot) override;
	void onRepliedSendExtraCache(QString strData) override;
	void makeAdvice(QJsonObject& jsonToInject, double thresholdScore) const;
	void addTrend(QJsonObject& jsonToInject, QString strTrend, SuperOracle::Summary) const;
	void askQuestion(QJsonObject &jsonToInject) const;

private:
	QDate m_date;
	double m_minSlope = 0.0;
	int m_futDayMinSlopeCollision = 0;
	double m_slushFundStartsAt = 0.0;
	double m_slushFundTypicalNeed = 1000.0;
	SparkLine m_spark;
};

#endif // EXTRACACHE_H
