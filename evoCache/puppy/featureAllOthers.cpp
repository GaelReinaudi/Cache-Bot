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

	m_bundle.clear();
	m_avgDayIncome = 0.0;
	m_avgDayOutcome = 0.0;
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
			m_avgDayIncome +=trans.amountDbl();
			++trans.dimensionOfVoid;
		}
		if (trans.amountInt() < 0) {
			++negTr;
			m_avgDayOutcome +=trans.amountDbl();
			++trans.dimensionOfVoid;
		}
		m_avgDayIncome /= double(qMax(1, inilastDate.daysTo(lastDate)));
		m_avgDayOutcome /= double(qMax(1, inilastDate.daysTo(lastDate)));
	}
	m_fitness = double(alreadyMatched) / allTrans.count();
	m_billProba = 0.5;

	QVector<Transaction> targetTrans = targetTransactions(iniDate, lastDate.addDays(BotContext::TARGET_TRANS_FUTUR_DAYS));
	if (ioContext.m_summaryJsonObj) {
		LOG() << getName().c_str() << targetTrans.count()
			  << "avgDayIncome" << m_avgDayIncome
			  << "avgDayOutcome" << m_avgDayOutcome
			  << endl;
	}

	output = m_fitness;

	// summary if the json object exists
	if (ioContext.m_summaryJsonObj) {
		if(ioContext.m_mapPredicted) {
			(*(ioContext.m_mapPredicted))[m_billProba] += targetTrans;
		}

		if(m_billProba > 0.0) {
			QJsonArray features = (*ioContext.m_summaryJsonObj)["features"].toArray();
			features.append(toJson(ioContext));
			ioContext.m_summaryJsonObj->insert("features", features);
		}
	}
}

QVector<Transaction> FeatureAllOthers::targetTransactions(QDate iniDate, QDate lastDate)
{
	QVector<Transaction> targetTrans;
	return targetTrans;
}
