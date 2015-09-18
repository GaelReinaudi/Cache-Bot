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
//	Fund* extraFund = user()->extraCacheFund();
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
//	bestBot->summarize();

	QJsonObject statObj;// = bestBot->postTreatment();

	QJsonObject trendObjects;
	OracleTrend<7>::get(user())->value(Transaction::currentDay());
	SuperOracle::Summary effectsummary = OracleTrend<7>::get(user())->effectSummaries()[Transaction::currentDay()];
	trendObjects.insert("07", effectsummary.toJson());
	OracleTrend<30>::get(user())->value(Transaction::currentDay());
	effectsummary = OracleTrend<30>::get(user())->effectSummaries()[Transaction::currentDay()];
	trendObjects.insert("30", effectsummary.toJson());

	statObj.insert("trends", trendObjects);

	bestBot->summarize();
	SuperOracle::Summary summary = OracleSummary::get(user())->summaries()[Transaction::currentDay()];

	statObj.insert("oracles", summary.toJson());

	makeAdvice(statObj, 0.05);

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
	else
		INFO() << QString(QJsonDocument(statObj).toJson());
}

void ExtraCache::makeAdvice(QJsonObject &jsonToInject, double thresholdScore) const
{
	QJsonObject trends = jsonToInject.value("trends").toObject();
	// list organized by index instead of by "overDays"
	QList<std::vector<QJsonObject> > indexDaysList;
	bool firstPass = true;
	for (const auto d : trends.keys()) {
		QJsonArray allFeat = trends[d].toArray();
		for (int i = 0; i < allFeat.count(); ++i) {
			if (firstPass)
				indexDaysList.append(std::vector<QJsonObject>());
			indexDaysList[i].push_back(allFeat[i].toObject());
		}
		firstPass = false;
	}
	// now we go through that list to cherry pick the json we want on that level
	QList<QJsonObject> advice;
	for (int i = 0; i < indexDaysList.count(); ++i) {
		auto result = std::minmax_element(indexDaysList[i].begin(), indexDaysList[i].end()
							   , [](const QJsonObject& a, const QJsonObject& b) {
			return qAbs(a["score"].toDouble()) < qAbs(b["score"].toDouble());
		});
		QJsonObject mostEffect = indexDaysList[i][result.second - indexDaysList[i].begin()];
		mostEffect["ind"] = i;
		if (qAbs(mostEffect["score"].toDouble()) >= thresholdScore)
			advice.append(mostEffect);
	}
	std::sort(advice.begin(), advice.end(), [](const QJsonObject& a, const QJsonObject& b) {
		return qAbs(a["score"].toDouble()) > qAbs(b["score"].toDouble());
	});
	QJsonArray jsonAdvice;
	for (int i = 0; i < advice.count(); ++i) {
		jsonAdvice.append(advice[i]);
	}
	jsonToInject.insert("advices", jsonAdvice);
}


void ExtraCache::onRepliedSendExtraCache(QString strData)
{
	CacheAccountConnector::onRepliedSendExtraCache(strData);
	if(CacheAccountConnector::flags & CacheAccountConnector::AutoExit)
		qApp->exit();
}
