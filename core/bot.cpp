#include "bot.h"
#include "AccRegPrimits.h"
#include "botContext.h"
#include "userMetrics.h"
#include "oracle.h"

Bot::Bot(QJsonObject jsonBot, QObject *parent)
	:DBobj(jsonBot["_id"].toString(), parent)
{
//	Q_ASSERT(!jsonBot["_id"].toString().isEmpty());
	m_created = QDateTime::fromString(jsonBot["createdAt"].toString(), Qt::ISODate);

	m_botStrings.clear();
	m_botStrings += CacheBotRootPrimitive::rootName();
	QJsonArray jsonFeatArray = jsonBot["features"].toArray();
	for (int iF = 0; iF < jsonFeatArray.size(); ++iF) {
		QJsonObject jsonFeat = jsonFeatArray[iF].toObject();
		QString featName = jsonFeat["name"].toString();
		m_botStrings += featName;
		int numArgs = jsonFeat["numArgs"].toInt();
		QJsonArray jsonValArray = jsonFeat["args"].toArray();
		for (int iV = 0; iV < numArgs; ++iV) {
			double val = jsonValArray.at(iV).toDouble();
			QString strVal = QString::number(val, 'f', 20);
			m_botStrings += strVal;
		}
	}
	LOG() << "Bot::Bot from JsonObject" << endl;
	for (auto& s : m_botStrings) {
		LOG() << s << endl;
	}
}

Bot::~Bot()
{
}

void Bot::init(BotContext *context) {
	m_context = context;
	BotContext::LIMIT_NUM_FEATURES = BotContext::MAX_NUM_FEATURES;
	Puppy::initializeTree(m_puppyTree, *m_context, m_botStrings);
	LOG() << "Bot::init" << m_puppyTree.toStr() << endl;
}

double Bot::evaluate()
{
	double fit;
	m_puppyTree.interpret(&fit, *m_context);

	return fit;
}

QJsonObject Bot::summarize()
{
	QJsonObject jsonObj;
	jsonObj.insert("features", QJsonArray());
	m_puppyTree.mValid = false;
	m_context->m_summaryJsonObj = &jsonObj;
	m_context->m_pUser->oracle()->clearSubOracles();
	double fit = evaluate();
	m_context->m_summaryJsonObj = 0;

	jsonObj.insert("fit", fit);

	emit m_context->needsReplot();
	emit m_context->newSummarizedTree(jsonObj);

	QString jsonStr = QJsonDocument(jsonObj).toJson(/*QJsonDocument::Compact*/);
	LOG() << "Bot::summarize() tree (" << fit << "): " << m_puppyTree.toStr() << endl;
	LOG() << "    " << jsonStr << endl;
	return jsonObj;
}

QJsonObject Bot::postTreatment()
{
	QDate lastDate = Transaction::currentDay();

//	double avgDayIn090  = MakeRateMonthPercentileMetric<2, 90>::get(m_context->m_pUser)->value(lastDate);
//	double avgDayOut090 = CostRateMonthPercentileMetric<2, 90>::get(m_context->m_pUser)->value(lastDate);
	double avgDayIn095  = MakeRateMonthPercentileMetric<2, 95>::get(m_context->m_pUser)->value(lastDate);
//	double avgDayOut095 = CostRateMonthPercentileMetric<2, 95>::get(m_context->m_pUser)->value(lastDate);
//	double avgDayIn099  = MakeRateMonthPercentileMetric<2, 99>::get(m_context->m_pUser)->value(lastDate);
	double avgDayOut099 = CostRateMonthPercentileMetric<2, 99>::get(m_context->m_pUser)->value(lastDate);
//	double avgDayIn100  = MakeRateMonthPercentileMetric<2, 100>::get(m_context->m_pUser)->value(lastDate);
//	double avgDayOut100 = CostRateMonthPercentileMetric<2, 100>::get(m_context->m_pUser)->value(lastDate);

	QJsonObject statObj;
//	statObj.insert("avgDayIn099", avgDayIn099);
//	statObj.insert("avgDayOut099", avgDayOut099);
//	statObj.insert("avgDayIn090", avgDayIn090);
//	statObj.insert("avgDayOut090", avgDayOut090);
//	statObj.insert("avgDayIn095", avgDayIn095);
//	statObj.insert("avgDayOut095", avgDayOut095);
//	statObj.insert("avgDayIn100", avgDayIn100);
//	statObj.insert("avgDayOut100", avgDayOut100);

	double flowrate = avgDayIn095 + avgDayOut099;
	flowrate /= avgDayIn095;

	statObj.insert("Flow00", flowrate);

	flowrate = 0.01 * Flow01<95, 99>::get(m_context->m_pUser)->value(lastDate);
	statObj.insert("Flow01", flowrate);

	QString jsonStr = QJsonDocument(statObj).toJson();
	LOG() << "Bot::postTreatment    statObj  " << jsonStr << endl;

	return statObj;
}

User* Bot::user() const
{
	return m_context->m_pUser;
}
