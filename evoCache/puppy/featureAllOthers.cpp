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
	m_avgDayIn90 = ioContext.m_pUser->makeLiving(0.90);
	m_avgDayOut90 = ioContext.m_pUser->costLiving(0.90);
	m_avgDayIn95 = ioContext.m_pUser->makeLiving(0.95);
	m_avgDayOut95 = ioContext.m_pUser->costLiving(0.95);
	m_avgDayIn099 = ioContext.m_pUser->makeLiving(0.099);
	m_avgDayOut099 = ioContext.m_pUser->costLiving(0.099);
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

	QVector<Transaction> targetTrans = targetTransactions(iniDate, lastDate.addDays(BotContext::TARGET_TRANS_FUTUR_DAYS));
	if (ioContext.m_summaryJsonObj) {
		LOG() << getName().c_str() << targetTrans.count()
			  << "avgDayIn099" << m_avgDayIn099
			  << "avgDayOut099" << m_avgDayOut099
			  << "avgDayIn90" << m_avgDayIn90
			  << "avgDayOut90" << m_avgDayOut90
			  << "avgDayIn95" << m_avgDayIn95
			  << "avgDayOut95" << m_avgDayOut95
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
	Q_UNUSED(iniDate);
	Q_UNUSED(lastDate);
	QVector<Transaction> targetTrans;
//	QDate currentDate = iniDate;
//	while (currentDate < lastDate) {
//		targetTrans.append(Transaction());
//		targetTrans.last().date = currentDate;
//		targetTrans.last().setAmount(m_avgDayIn90 - m_avgDayOut90);
//		targetTrans.last().indexHash = 0;
//		targetTrans.last().nameHash.setFromHash(0);
//		targetTrans.last().flags |= Transaction::Predicted;

//		currentDate = currentDate.addDays(1);
//	}
	return targetTrans;
}
