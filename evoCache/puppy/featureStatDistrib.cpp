#include "featureStatDistrib.h"

#define MIN_TRANSACTIONS_FOR_STAT 6

void FeatureStatDistrib::getArgs(Puppy::Context &ioContext) {
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

	// transaction to compare the hash with
	Transaction m_modelTrans;
	m_modelTrans.nameHash.setFromHash(m_hash);
	m_modelTrans.setAmount(-1.0); // only negative prices will have a usable distance
	m_modelTrans.date = lastDate;
	const int maxHashDist = 4;

	m_bundle.clear();
	for (int i = 0; i < allTrans.count(); ++i) {
		Transaction& trans = allTrans.trans(i);
		quint64 dist = trans.distanceWeighted<1024*1024*1024, 1024*1024*1024, maxHashDist>(m_modelTrans);
		if (dist < Transaction::LIMIT_DIST_TRANS) {
			m_bundle.append(&trans);
			// isolate the transaction that were fitted to the target
			Q_ASSERT(trans.dimensionOfVoid == 0);
			trans.dimensionOfVoid++;
		}
	}
	int numBund = m_bundle.count();
	if (numBund <= MIN_TRANSACTIONS_FOR_STAT) {
		m_fitness = 0.0;
		m_billProba = 0.0;
		output = m_fitness;
		return;
	}
	// get the date those transaction started
	QDate firstDate = m_bundle.trans(0).date;

	m_daysBundle = firstDate.daysTo(lastDate);
	m_dayProba = numBund / m_daysBundle;

	m_billProba = m_dayProba;
	m_fitness = qAbs(kindaLog(m_bundle.sumDollar()));
	m_fitness = 5.0;
	m_fitness *= numBund * numBund;
	m_fitness /= MIN_TRANSACTIONS_FOR_STAT * MIN_TRANSACTIONS_FOR_STAT;
	m_fitness *= m_dayProba;
	output = m_fitness;

	if (ioContext.m_summaryJsonObj) {
		LOG() << getName().c_str() << " " << this
			<< " p=" << m_billProba
			<< " n=" << numBund
			<< " h=" << m_modelTrans.nameHash.hash()
			<< endl;
		if(m_billProba > 0.001) {
			QJsonArray features = (*ioContext.m_summaryJsonObj)["features"].toArray();
			features.append(toJson(ioContext));
			ioContext.m_summaryJsonObj->insert("features", features);
		}
		for (int i = 0; i < numBund; ++i) {
			Transaction& t = m_bundle.trans(i);
			emit ioContext.m_pUser->botContext()->matchedTransaction(t.time_t(), t.amountDbl(), 2);
		}
		OracleStatDistrib* pNewOr = new OracleStatDistrib();
		pNewOr->m_bundle = m_bundle;
		pNewOr->m_hash = m_hash;
		pNewOr->m_dayProba = m_dayProba;
		pNewOr->m_daysBundle = m_daysBundle;
		QSharedPointer<Oracle> newOracle(pNewOr);
		ioContext.m_pUser->oracle()->addSubOracle(newOracle);
	}
}

QVector<Transaction> OracleStatDistrib::revelation(QDate upToDate) {
	LOG() << "FeatureStatDistrib::revelation proba = " << m_dayProba << " bundle = " << m_bundle.count() << endl;
	QVector<Transaction> retVect;
	while (curDate() <= upToDate) {
		if (randBool(m_dayProba)) {
			Transaction randTr = m_bundle.randomTransaction();
			randTr.date = curDate();
			LOG() << "randomTransaction " << randTr.amountDbl() << " " << randTr.date.toString() << randTr.name << endl;
			retVect.append(randTr);
		}
		nextDay();
	}
	return retVect;
}
