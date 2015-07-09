#include "featureStatDistrib.h"


void FeatureStatDistrib::getArgs(Puppy::Context &ioContext) {
	AccountFeature::getArgs(ioContext);
	// if we are forcing a given hashed bundle
	int filterHashIndex = ioContext.filterHashIndex;
	if(filterHashIndex >= 0) {
		m_filterHash = ioContext.m_pUser->hashBundles().keys()[filterHashIndex];
		std::string nodeName = QString("h%1").arg(m_filterHash).toStdString();
		bool ok = tryReplaceArgumentNode(0, nodeName.c_str(), ioContext);
		if(!ok) {
			LOG() << "Could not replace the node with " << nodeName.c_str() << endl;
		}
	}
	else {
		m_filterHash = -1;
	}

	double a = 0;
	int ind = -1;
	getArgument(++ind, &a, ioContext);
	m_hash = a;
}

void FeatureStatDistrib::execute(void *outDatum, Puppy::Context &ioContext)
{
	AccountFeature::execute(outDatum, ioContext);
	double& output = *(double*)outDatum;

	getArgs(ioContext);
	cleanArgs();

	// will be ALL the transactions if m_filterHash < 0
	auto& allTrans = ioContext.m_pUser->transBundle(m_filterHash);
	QDate lastDate = QDate::currentDate();
	//	QDate iniDate = lastDate.addMonths(-6);

	// transaction to compare the hash with
	m_modelTrans = Transaction();
	m_modelTrans.nameHash.setFromHash(m_hash);
	m_modelTrans.setAmount(-1.0); // only negative prices will have a usable distance
	m_modelTrans.date = lastDate;
	const int maxHashDist = 5;

	m_bundle.clear();
	for (int i = 0; i < allTrans.count(); ++i) {
		Transaction& trans = allTrans.trans(i);
		quint64 dist = trans.distanceWeighted<1024*1024*1024, 1024*1024*1024, maxHashDist>(m_modelTrans);
		if (dist < Transaction::LIMIT_DIST_TRANS) {
			m_bundle.append(&trans);
			// isolate the transaction that were fitted to the target
			Q_ASSERT(localTrans->dimensionOfVoid == 0);
			trans.dimensionOfVoid++;
		}
	}
	if (m_bundle.count() <= 4) {
		m_fitness = 0.0;
		m_billProba = 0.0;
		output = m_fitness;
		return;
	}
	// get the date those transaction started
	QDate firstDate = m_bundle.trans(0).date;

	m_daysBundle = firstDate.daysTo(lastDate);
	m_dayProba = m_bundle.count() / m_daysBundle;

	double totkla = qAbs(kindaLog(m_bundle.sumDollar()));
	m_fitness = totkla;
	m_billProba = m_dayProba;
	output = m_fitness;
	if (ioContext.m_summaryJsonObj) {
//		LOG() << "FeatureStatDistrib #" << m_bundle.count()
//			  << "fit" << m_fitness
//			  << "hash" << m_hash
//			  << "days" << m_daysBundle
//			  << "proba" << m_dayProba
//			  << endl;
		if(m_billProba > 0.0) {
			QJsonArray features = (*ioContext.m_summaryJsonObj)["features"].toArray();
			features.append(toJson(ioContext));
			ioContext.m_summaryJsonObj->insert("features", features);
		}
	}
}

QVector<Transaction> FeatureStatDistrib::revelation(QDate upToDate) {
	QVector<Transaction> retVect;
	while (curDate() <= upToDate) {
		if (randBool(m_dayProba)) {
			retVect.append(randomTransaction());
		}
		nextDay();
	}
	return retVect;
}
