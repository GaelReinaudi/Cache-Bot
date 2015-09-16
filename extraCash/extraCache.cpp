#include "extraCache.h"
#include "account.h"
#include "bot.h"
#include "oracle.h"
#include "cacherest.h"
#include "fund.h"
#include "userMetrics.h"

static const double SLOPE_MULTIPLICATION_EXTRA_TODAY = 0.5;

ExtraCache::ExtraCache(QString userID)
	: CacheAccountConnector(userID)
{
}

void ExtraCache::onUserInjected(User* pUser)
{
	CacheAccountConnector::onUserInjected(pUser);

	// transaction at the starting date of the playback
	auto& real = user()->allTrans();
	m_date = Transaction::currentDay();
	NOTICE() << "currentDay " << m_date.toString();

	for (int i = 0; i < real.count(); ++i) {
		if (real.trans(i).date >= m_date) {
			NOTICE() << "initial trans("<<i<<")" << real.trans(i).date.toString() << real.trans(i).name;
			break;
		}
	}

	// some arbitrary slush need to (try to) never go under of
	m_slushFundTypicalNeed = 0.0;
	m_slushFundTypicalNeed += 0.5 * user()->balance(Account::Type::Checking);
	m_slushFundTypicalNeed += 0.5 * -CostRateMonthPercentileMetric<6, 75>::get(user())->value(m_date) * 30;

	NOTICE() << "m_slushFundTypicalNeed" << m_slushFundTypicalNeed;

	// the amount of money on the extraCash fund already
	Fund* extraFund = user()->extraCacheFund();
	double extraTotal = 0.0;
//	for (const Cash& c : extraFund->cashes()) {
//		extraTotal += c.amount;
//	}
	m_slushFundStartsAt = extraTotal;
	NOTICE() << "extraTotal" << m_slushFundStartsAt;

	CacheRest::Instance()->getBestBot(userID(), user());
}

void ExtraCache::onBotInjected(Bot* bestBot)
{
	NOTICE() << "ExtraCache::onBotInjected";
	bestBot->summarize();

	QJsonObject statObj = bestBot->postTreatment();

	static bool alreadyOnce = false;
	if (!alreadyOnce) {
		alreadyOnce = true;
		OracleTrend<7>::get(user())->value(Transaction::currentDay());
		SuperOracle::Summary effectsummary = OracleTrend<7>::get(user())->effectSummaries()[Transaction::currentDay()];
		statObj.insert("trend7", effectsummary.toJson());
		INFO() << QString(QJsonDocument(statObj).toJson());
		OracleTrend<30>::get(user())->value(Transaction::currentDay());
		effectsummary = OracleTrend<30>::get(user())->effectSummaries()[Transaction::currentDay()];
		statObj.insert("trend30", effectsummary.toJson());
		INFO() << QString(QJsonDocument(statObj).toJson());
	}

	if (!statObj.contains("trend7"))
		return;
	if (!statObj.contains("trend30"))
		return;

	SuperOracle::Summary summary = user()->oracle()->computeAvgCashFlow();
	QJsonObject flowObj;
	flowObj.insert("rate", summary.flow());
	flowObj.insert("state", QString("kFlow"));

	statObj.insert("flow", flowObj);

	// if critically low flow
	if (summary.flow() <= -0.95) {
		WARN() << "Cache flow critically low: " << summary.flow();
	}

	if (flags & SendExtraCash) {
		CacheRest::Instance()->sendExtraCash(user()->id(), 0.0, statObj);
	}
}

void ExtraCache::onRepliedSendExtraCache(QString strData)
{
	CacheAccountConnector::onRepliedSendExtraCache(strData);
	if(CacheAccountConnector::flags & CacheAccountConnector::AutoExit)
		qApp->exit();
}


