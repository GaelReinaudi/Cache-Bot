#include "extraCache.h"
#include "account.h"
#include "bot.h"
#include "oracle.h"
#include "cacherest.h"
#include "fund.h"
#include "userMetrics.h"

static const double SLOPE_MULTIPLICATION_EXTRA_TODAY = 0.5;

ExtraCache::ExtraCache(QString userID, QJsonObject jsonArgs)
	: CacheAccountConnector(userID, jsonArgs)
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
	OracleTrend<1>::get(user())->value(Transaction::currentDay());
	SuperOracle::Summary effectsummary = OracleTrend<1>::get(user())->effectSummaries()[Transaction::currentDay()];
	trendObjects.insert("01", effectsummary.toJson());
	OracleTrend<7>::get(user())->value(Transaction::currentDay());
	effectsummary = OracleTrend<7>::get(user())->effectSummaries()[Transaction::currentDay()];
	trendObjects.insert("07", effectsummary.toJson());
	OracleTrend<30>::get(user())->value(Transaction::currentDay());
	effectsummary = OracleTrend<30>::get(user())->effectSummaries()[Transaction::currentDay()];
	trendObjects.insert("30", effectsummary.toJson());

	statObj.insert("trends", trendObjects);

	double flowMA_2 = MetricSmoother<2>::get(OracleSummary::get(user()))->value(Transaction::currentDay());
	double flowMA_3 = MetricSmoother<3>::get(OracleSummary::get(user()))->value(Transaction::currentDay());
	double flowMA_4 = MetricSmoother<4>::get(OracleSummary::get(user()))->value(Transaction::currentDay());

	double flowDif_1 = MetricDiff<1>::get(MetricSmoother<2>::get(OracleSummary::get(user())))->value(Transaction::currentDay());
	double flowDif_2 = MetricDiff<2>::get(MetricSmoother<2>::get(OracleSummary::get(user())))->value(Transaction::currentDay());
	double flowDif_3 = MetricDiff<3>::get(MetricSmoother<2>::get(OracleSummary::get(user())))->value(Transaction::currentDay());
	QJsonObject flowObj;
	flowObj.insert("ma_2", flowMA_2);
	flowObj.insert("ma_3", flowMA_3);
	flowObj.insert("ma_4", flowMA_4);
	flowObj.insert("dif_1", flowDif_1);
	flowObj.insert("dif_2", flowDif_2);
	flowObj.insert("dif_3", flowDif_3);

	static int numCalc = 0;
flowCalc:
	++numCalc;
	bestBot->summarize();
	double d2z50 = Montecarlo<128>::get(user())->value(Transaction::currentDay());
	double d2z20 = Montecarlo<128>::get(user())->d2zPerc(Transaction::currentDay(), 0.20);
	double d2z80 = Montecarlo<128>::get(user())->d2zPerc(Transaction::currentDay(), 0.80);
	double rate = OracleSummary::get(user())->value(Transaction::currentDay());
	SuperOracle::Summary summary = OracleSummary::get(user())->summaries()[Transaction::currentDay()];

	statObj.insert("oracles", summary.toJson());

	makeAdvice(statObj, 0.05);

	flowObj.insert("rate", rate);

	flowObj.insert("dailyPos", summary.posSum);
	flowObj.insert("dailyNeg", summary.negSum);
	flowObj.insert("dailyBill", summary.bill);
	flowObj.insert("dailySalary", summary.salary);

	flowObj.insert("d2z50", d2z50);
	flowObj.insert("d2z20", d2z20);
	flowObj.insert("d2z80", d2z80);

	// flag as kStable/kUp/kDown
	QString changeFlag = "kStable";
	if (flowDif_2 > 0.05)
		changeFlag = "kUp";
	if (flowDif_2 < -0.05)
		changeFlag = "kDown";
	flowObj.insert("change", changeFlag);

	// let's get the index of the oracle that absorbed the hypothetic transaction
	if (numCalc == 2) {
		int indOracleHypo = -1;
		for (int i = 0; i < summary.summaryPerOracle.count(); ++i) {
			for (const QJsonValueRef& t : summary.summaryPerOracle[i]["trans"].toArray()) {
				//WARN() <<"aaaaaaaaaaaaaaa"<<i<<" "<< t.toString();
				if (t.toString().contains("hypothetic")) {
					indOracleHypo = i;
					break;
				}
			}
		}
		flowObj["indOracleHypotheTrans"] = indOracleHypo;
	}
	statObj.insert(numCalc == 1 ? "flow" : "askFlow", flowObj);
	if (jsonArgs().contains("ask") && numCalc < 2) {
		user()->setHypotheTrans(jsonArgs()["ask"].toDouble());
		flowObj = QJsonObject();
		flowObj["ask"] = jsonArgs()["ask"].toDouble();
		goto flowCalc;
	}

	// if critically low flow
	if (summary.flow() <= -0.95) {
		WARN() << "Cache flow critically low: " << summary.flow();
	}

	statObj["_git"] = QString(GIT_VERSION);
	statObj["_currentDay"] = Transaction::currentDay().toString();
//	statObj["_actualCurrentDayTimeUTC"] = Transaction::actualCurrentDayTime().toUTC().toString();
//	statObj["_actualCurrentDayTime"] = Transaction::actualCurrentDayTime().toString();
//	statObj["_currentDateTime"] = QDateTime::currentDateTime().toString();
//	statObj["_toUTC"] = QDateTime::currentDateTime().toUTC().toString();
//	statObj["_toLocalTime"] = QDateTime::currentDateTime().toLocalTime().toString();
//	statObj["_lastTransDate"] = user()->allTrans().count() == 0 ? "none" : user()->allTrans().last().date.toString();


	if (flags & SendExtraCash) {
		statObj.insert("_inArgs", jsonArgs());
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
