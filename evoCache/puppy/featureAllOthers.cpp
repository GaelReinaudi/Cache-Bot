#include "featureAllOthers.h"



void FeatureAllOthers::execute(void *outDatum, Puppy::Context &ioContext)
{
	AccountFeature::execute(outDatum, ioContext);
	double& output = *(double*)outDatum;

	getArgs(ioContext);
	cleanArgs();

	m_fitness = 0.0;

	// will be ALL the transactions if m_filterHash < 0
	auto& allTrans = ioContext.m_pUser->transBundle(m_filterHash);
	QDate lastDate = QDate::currentDate();
	QDate iniDate = lastDate.addMonths(-6);

	m_localStaticArgs.m_bundle.clear();
	m_localStaticArgs.m_avgDayIn90 = 123456;//ioContext.m_pUser->makeLiving(0.90);
	m_localStaticArgs.m_avgDayOut90 = 123456;//HistoMetric::get("cost2MonthP90")[lastDate];
	m_localStaticArgs.m_avgDayIn95 = 123456;//ioContext.m_pUser->makeLiving(0.95);
	m_localStaticArgs.m_avgDayOut95 = 123456;//HistoMetric::get("cost2MonthP95")[lastDate];
	m_localStaticArgs.m_avgDayIn099 = 123456;//ioContext.m_pUser->makeLiving(0.99);
	m_localStaticArgs.m_avgDayOut099 = 123456;//HistoMetric::get("cost2MonthP99")[lastDate];
	int posTr = 0;
	int negTr = 0;
	int alreadyMatched = 0;
	for (int i = 0; i < allTrans.count(); ++i) {
		Transaction& trans = allTrans.trans(i);
		if (trans.isInternal())
			continue;
		if (trans.dimensionOfVoid) {
			++alreadyMatched;
			continue;
		}
		if (trans.amountInt() > 0) {
			++posTr;
		}
		if (trans.amountInt() < 0) {
			++negTr;
		}
		++trans.dimensionOfVoid;
	}
	m_fitness = double(alreadyMatched * (posTr + negTr)) / allTrans.count();
	m_billProba = 1000000.0;

	if (ioContext.m_summaryJsonObj) {
		LOG() << getName().c_str()
			  << "avgDayIn099" << m_localStaticArgs.m_avgDayIn099
			  << "avgDayOut099" << m_localStaticArgs.m_avgDayOut099
			  << "avgDayIn90" << m_localStaticArgs.m_avgDayIn90
			  << "avgDayOut90" << m_localStaticArgs.m_avgDayOut90
			  << "avgDayIn95" << m_localStaticArgs.m_avgDayIn95
			  << "avgDayOut95" << m_localStaticArgs.m_avgDayOut95
			  << endl;
	}

	output = m_fitness;

	// summary if the json object exists
	if (ioContext.m_summaryJsonObj) {
		if(m_billProba > 0.0) {
			QJsonArray features = (*ioContext.m_summaryJsonObj)["features"].toArray();
			features.append(toJson(ioContext));
			ioContext.m_summaryJsonObj->insert("features", features);
		}
	}
}

