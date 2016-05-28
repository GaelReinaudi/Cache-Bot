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
	NOTICE() << "extraTotal" << m_slushFundStartsAt << " curDay " << Transaction::currentDay().toString();

	CacheRest::Instance()->getBestBot(userID(), user());
}

double ExtraCache::calcSummary(Bot* bestBot, QJsonObject& statObj, int doAskThing /*= 0*/)
{
	double flowMA_2 = MetricSmoother<2>::get(OracleSummary::get(user()))->value(Transaction::currentDay());
	double flowMA_3 = MetricSmoother<3>::get(OracleSummary::get(user()))->value(Transaction::currentDay());
	double flowMA_4 = MetricSmoother<4>::get(OracleSummary::get(user()))->value(Transaction::currentDay());

	double flowDif_1 = MetricDiff<1>::get(MetricSmoother<2>::get(OracleSummary::get(user())))->value(Transaction::currentDay());
	double flowDif_2 = MetricDiff<2>::get(MetricSmoother<2>::get(OracleSummary::get(user())))->value(Transaction::currentDay());
	double flowDif_3 = MetricDiff<3>::get(MetricSmoother<2>::get(OracleSummary::get(user())))->value(Transaction::currentDay());
	QJsonObject flowObj = statObj["flow"].toObject();
	flowObj.insert("ma_2", flowMA_2);
	flowObj.insert("ma_3", flowMA_3);
	flowObj.insert("ma_4", flowMA_4);
	flowObj.insert("dif_1", flowDif_1);
	flowObj.insert("dif_2", flowDif_2);
	flowObj.insert("dif_3", flowDif_3);

	int numCalc = 0;
flowCalc:
	++numCalc;
	bestBot->summarize();
	double rate = OracleSummary::get(user())->value(Transaction::currentDay());
	SuperOracle::Summary summary = OracleSummary::get(user())->summaries()[Transaction::currentDay()];

	statObj.insert("oracles", summary.toJson());

	flowObj.insert("rate", rate);

	flowObj.insert("dailyPos", summary.posSum);
	flowObj.insert("dailyNeg", summary.negSum);
	flowObj.insert("dailyPos45", summary.posSum45);
	flowObj.insert("dailyNeg45", summary.negSum45);
	flowObj.insert("dailyPos60", summary.posSum60);
	flowObj.insert("dailyNeg60", summary.negSum60);
	flowObj.insert("dailyPos90", summary.posSum90);
	flowObj.insert("dailyNeg90", summary.negSum90);
	flowObj.insert("dailyBill", summary.bill);
	flowObj.insert("dailySalary", summary.salary);

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
				if (t.toString().contains("hypothetic")) {
					indOracleHypo = i;
					break;
				}
			}
		}
		flowObj["indOracleHypotheTrans"] = indOracleHypo;
	}
	statObj.insert(numCalc == 1 ? "flow" : "askFlow", flowObj);
	if (doAskThing && jsonArgs().contains("ask") && numCalc < 2) {
		user()->setHypotheTrans(jsonArgs()["ask"].toDouble());
//		HistoMetric::clearAll();
//		user()->reInjectBot();
		flowObj = QJsonObject();
		flowObj["ask"] = jsonArgs()["ask"].toDouble();
		goto flowCalc;
	}

	return summary.flow();
}

void ExtraCache::makeCategoryTreeSummary(Bot* bestBot, QStringList cats, QJsonObject& statObj, int level /*= 0*/)
{
	if (level == 0) {
		Transaction::s_magicFilter = 0;
		int nT = user()->setMagic(Transaction::s_magicFilter);
		WARN() << "magic 0 applied to " << nT << " trans";
	}
	QJsonObject categoryObject;
	for (const QString& strCat : cats) {
		++Transaction::s_magicFilter;
		WARN() << "topCat: " << strCat << " s_magicFilter " << Transaction::s_magicFilter;
		Transaction::makeCatRegExps(strCat);

		int nT = user()->setMagic(Transaction::s_magicFilter, [&](const NameHashVector& hashCat){
			QString strHashCat = QString("%1").arg(qAbs(hashCat.hash()), 8, 10, QChar('0'));
			bool match = false;
			for (QRegExp r : Transaction::rootCatRegExp) {
				if (r.pattern().startsWith("!")) {
					r.setPattern(r.pattern().mid(1));
					match &= !r.exactMatch(strHashCat);
					//WARN() << "r.setPattern: " << r.pattern() << " match " << match;
				}
				else
					match |= r.exactMatch(strHashCat);
			}
			return match;
		});
		WARN() << "magic applied to " << nT << " trans";

		QJsonObject jsonBranch;
		HistoMetric::clearAll();
		calcSummary(bestBot, jsonBranch);
		QJsonObject subCategoryObject;
		for (const QString& subStr : Transaction::subCatRegExp.keys()) {
			QVector<QRegExp>& regexList = Transaction::subCatRegExp[subStr];
			++Transaction::s_magicFilter;
			WARN() << "subCat: " << subStr << " s_magicFilter " << Transaction::s_magicFilter;

			int nTT = user()->setMagic(Transaction::s_magicFilter, [&](const NameHashVector& hashCat){
				QString strHashCat = QString("%1").arg(qAbs(hashCat.hash()), 8, 10, QChar('0'));
				bool match = false;
				for (QRegExp r : regexList) {
					if (r.pattern().startsWith("!")) {
						r.setPattern(r.pattern().mid(1));
						match &= !r.exactMatch(strHashCat);
					}
					else
						match |= r.exactMatch(strHashCat);
				}
				return match;
			});
			WARN() << "magic applied to " << nTT << " trans";

			QJsonObject jsonSubBranch;
			HistoMetric::clearAll();
			calcSummary(bestBot, jsonSubBranch);
			subCategoryObject.insert(subStr, jsonSubBranch);
			jsonBranch.insert("categories", subCategoryObject);
		}
		categoryObject.insert(strCat, jsonBranch);
	}
	// for "Other"
	if (level == 0) {
		Transaction::s_magicFilter = 0;
		WARN() << "topCat: " << "Other" << " s_magicFilter " << Transaction::s_magicFilter;
		QJsonObject jsonBranch;
		HistoMetric::clearAll();
		calcSummary(bestBot, jsonBranch);
		categoryObject.insert("Other", jsonBranch);
	}
	categoryObject = orderCategoryTree(categoryObject);
	statObj.insert("categories", categoryObject);
	Transaction::s_magicFilter = 0;
	int nT = user()->setMagic(Transaction::s_magicFilter);
	WARN() << "eventually, magic 0 applied to " << nT << " trans";
	HistoMetric::clearAll();
//	calcSummary(bestBot, jsonSubBranch);
}

QJsonObject ExtraCache::orderCategoryTree(QJsonObject &catObj)
{
	QJsonObject newObj;
	QMap<double, QString> costKeys;
	for (const QString& k : catObj.keys()) {
		double cost = catObj[k].toObject()["flow"].toObject()["dailyBill"].toDouble();
		cost += catObj[k].toObject()["flow"].toObject()["dailyNeg"].toDouble();
		costKeys.insertMulti(cost, k);
	}
	const int maxNumTop = 4;
	int i = 0;
	QJsonObject otherCatCat;
	for (auto it = costKeys.cbegin(); it != costKeys.cend(); ++it) {
		//WARN() << it.key() << " ggggggg " << it .value();
		if (it.value() == "Other")
			continue;
		QJsonObject topCat = catObj[it.value()].toObject();
		if (i < maxNumTop) {
			topCat["rank"] = i;
			++i;
			newObj[it.value()] = topCat;
		}
		else {
			otherCatCat[it.value()] = topCat;
		}
	}
	QJsonObject otherCat = catObj["Other"].toObject();
	otherCat["categories"] = otherCatCat;
	newObj["Other"] = otherCat;
	return newObj;
}

void ExtraCache::onBotInjected(Bot* bestBot)
{
	NOTICE() << "ExtraCache::onBotInjected";
//	bestBot->summarize();

	QJsonObject statObj;// = bestBot->postTreatment();

	QJsonArray accounts;
	for (const auto a : user()->accounts()) {
		QJsonObject theAc;
		theAc["_id"] = a->accountID;
		theAc["type"] = a->type();
		accounts.append(theAc);
	}
	statObj["accounts"] = accounts;

//	QJsonObject trendObjects;
	OracleTrend<1>::get(user())->value(Transaction::currentDay());
	SuperOracle::Summary trend01 = OracleTrend<1>::get(user())->effectSummaries()[Transaction::currentDay()];
	OracleTrend<7>::get(user())->value(Transaction::currentDay());
	SuperOracle::Summary trend07 = OracleTrend<7>::get(user())->effectSummaries()[Transaction::currentDay()];
	OracleTrend<30>::get(user())->value(Transaction::currentDay());
	SuperOracle::Summary trend30 = OracleTrend<30>::get(user())->effectSummaries()[Transaction::currentDay()];

//	statObj.insert("trends", trendObjects);

//	makeAdvice(statObj, 0.05);

	HistoMetric::clearAll();
	QStringList topCats = {"Food", "Transit", "Shops", "Recreation", "Rent", "Mortgage"};
	makeCategoryTreeSummary(bestBot, topCats, statObj);

	HistoMetric::clearAll();
	double flowRate = calcSummary(bestBot, statObj, true);

	//	bestBot->summarize();
	HistoMetric::clearAll();
	bestBot->summarize();
	double rate = OracleSummary::get(user())->value(Transaction::currentDay());
	SuperOracle::Summary summary = OracleSummary::get(user())->summaries()[Transaction::currentDay()];
	Montecarlo<128>::get(user())->value(Transaction::currentDay());
	double d2z50 = Montecarlo<128>::get(user())->d2zPerc(Transaction::currentDay(), 0.50);
	double d2z20 = Montecarlo<128>::get(user())->d2zPerc(Transaction::currentDay(), 0.20);
	double d2z80 = Montecarlo<128>::get(user())->d2zPerc(Transaction::currentDay(), 0.80);
	double valMin50 = 0.0;
	double valMin20 = 0.0;
	double valMin80 = 0.0;
	double d2M50 = Montecarlo<128>::get(user())->d2MinPerc(Transaction::currentDay(), 0.50, &valMin50);
	double d2M20 = Montecarlo<128>::get(user())->d2MinPerc(Transaction::currentDay(), 0.20, &valMin20);
	double d2M80 = Montecarlo<128>::get(user())->d2MinPerc(Transaction::currentDay(), 0.80, &valMin80);
	double d2Min50Delta1Month = valMin50;
	d2Min50Delta1Month += BalanceMetric::get(user())->value(Transaction::currentDay());
	d2Min50Delta1Month -= BalanceMetric::get(user())->value(Transaction::currentDay().addDays(d2M50).addMonths(-1));
	QJsonObject flowObj = statObj["flow"].toObject();
	flowObj.insert("d2z50", d2z50);
	flowObj.insert("d2z20", d2z20);
	flowObj.insert("d2z80", d2z80);
	flowObj.insert("d2Min50", d2M50);
	flowObj.insert("d2Min20", d2M20);
	flowObj.insert("d2Min80", d2M80);
	flowObj.insert("d2Min50Amnt", valMin50);
	flowObj.insert("d2Min20Amnt", valMin20);
	flowObj.insert("d2Min80Amnt", valMin80);
	flowObj.insert("d2Min50Delta1Month", d2Min50Delta1Month);

	QJsonArray simuBal;
	QJsonArray simuBalApprox;
	QJsonArray simuDev;
	QJsonArray simuVal;
	Montecarlo<128>::get(user())->simuValDev(Transaction::currentDay(), simuVal, simuDev);
	for (int i = 0; i < simuVal.count(); ++i) {
		double b = simuVal[i].toDouble() + User::totalAdjustedBalance;
		double d = simuDev[i].toDouble();
		simuBal.append(b);
		simuBalApprox.append(roundByBaseSystem(b, d));
	}

	flowObj.insert("simuVal", simuVal);
	flowObj.insert("simuBal", simuBal);
	flowObj.insert("simuBalApprox", simuBalApprox);
	flowObj.insert("simuDev", simuDev);
	flowObj.insert("weekDetail", summary.weekDetails);
	flowObj["simuDaysToSunday"] = Transaction::daysToSunday();

	double montheDelta = BalanceMetric::get(user())->value(Transaction::currentDay());
	montheDelta -= BalanceMetric::get(user())->value(Transaction::currentDay().addDays(-Transaction::currentDay().day()));
	flowObj.insert("monthDelta", montheDelta);
	double endMonthAmnt = 0.0;
	double d2EndMonth = Montecarlo<128>::get(user())->d2EndMonthPerc(0.5, &endMonthAmnt);
	double endMonthDelta1Month = endMonthAmnt;
	endMonthDelta1Month += BalanceMetric::get(user())->value(Transaction::currentDay());
	endMonthDelta1Month -= BalanceMetric::get(user())->value(Transaction::currentDay().addDays(d2EndMonth).addMonths(-1));
	flowObj.insert("endMonthAmnt", endMonthAmnt);
	flowObj.insert("d2EndMonth", d2EndMonth);
	flowObj.insert("endMonthDelta1Month", endMonthDelta1Month);

	statObj["flow"] = flowObj;

	addTrend(statObj, "01", trend01);
	addTrend(statObj, "07", trend07);

//	askQuestion(statObj);

	// if critically low flow
	if (flowRate <= -0.95) {
		WARN() << "Cache flow critically low: " << flowRate;
	}

	statObj["_git"] = QString(GIT_VERSION);
	statObj["_actualCurrentDayTime"] = Transaction::actualCurrentDayTime().toString();
	statObj["_actualCurrentDay"] = Transaction::actualCurrentDay().toString();
	statObj["_currentDay"] = Transaction::currentDay().toString();
	statObj["_daysWithoutCreateDate"] = Transaction::daysWithoutCreateDate();
	statObj["_balanceAdjust"] = User::balanceAdjust;
	statObj["_totalAdjustedBalance"] = User::totalAdjustedBalance;

	if (flags & SendExtraCash) {
		statObj.insert("_inArgs", jsonArgs());
		CacheRest::Instance()->sendExtraCash(user()->id(), 0.0, statObj);
	}
	else {
		INFO() << QString(QJsonDocument(statObj).toJson());
		QJsonDocument jsonDoc(statObj);
		QFile sampleReturn("jsonExtra.json");
		sampleReturn.open(QFile::WriteOnly | QFile::Truncate);
		QTextStream fileout(&sampleReturn);
		fileout << jsonDoc.toJson(QJsonDocument::Indented);
		if (flags & AutoExit)
			qApp->exit(0);
	}
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

void ExtraCache::addTrend(QJsonObject& jsonToInject, QString strTrend, SuperOracle::Summary trendSummary) const
{
	QJsonArray allOr;
	for (int i = 0; i < trendSummary.summaryPerOracle.count(); ++i) {
		QJsonObject obj = jsonToInject["oracles"].toArray()[i].toObject();
		QJsonObject objTrend = obj["trend"].toObject();
		objTrend[strTrend] = trendSummary.summaryPerOracle[i];
		obj["trend"] = objTrend;
		allOr.append(obj);
	}
	jsonToInject["oracles"] = allOr;
}

void ExtraCache::askQuestion(QJsonObject& jsonToInject) const
{
	QJsonArray allOr = jsonToInject["oracles"].toArray();
	double badestEffect = 0.0;
	int indexEffect = -1;
	for (int i = 0; i < allOr.count(); ++i) {
		QJsonObject orObj = allOr[i].toObject();
		QJsonObject trObj = orObj["trend"].toObject();
		for (const QString& k : trObj.keys()) {
			double eff = trObj[k].toObject()["flowEffect"].toDouble();
			if (eff < badestEffect) {
				badestEffect = eff;
				indexEffect = i;
			}
		}
	}

//	jsonToInject["question"] = "q";
}

void ExtraCache::onRepliedSendExtraCache(QString strData)
{
	CacheAccountConnector::onRepliedSendExtraCache(strData);
	if(CacheAccountConnector::flags & CacheAccountConnector::AutoExit)
		qApp->exit();
}
