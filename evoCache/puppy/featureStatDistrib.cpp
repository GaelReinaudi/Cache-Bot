#include "featureStatDistrib.h"


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
	const int maxHashDist = 5;

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
	LOG() << "FeatureStatDistrib #" << m_bundle.count()
		  << "days" << m_daysBundle
		  << "dayProba" << m_dayProba
		  << endl;
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
