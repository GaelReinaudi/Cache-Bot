#include "featureStatDistrib.h"

#define MIN_TRANSACTIONS_FOR_STAT 3//16
#define EFFECT_RANGE_WIDTH_RATIO 2.0

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
	m_localStaticArgs.m_hash = a;
	getArgument(++ind, &a, ioContext);
	m_localStaticArgs.m_effect = a;
	getArgument(++ind, &a, ioContext);
	m_localStaticArgs.m_kla = a;
}

double FeatureStatDistrib::apply(TransactionBundle& allTrans)
{
	// transaction to compare the hash with
	Transaction modelTrans;
	modelTrans.nameHash.setFromHash(m_localStaticArgs.m_hash);
	modelTrans.setAmount(-1.0); // only negative prices will have a usable distance
	modelTrans.date = Transaction::currentDay();
	const int maxHashDist = 4;

	m_localStaticArgs.m_bundle.clear();
	for (int i = 0; i < allTrans.count(); ++i) {
		const Transaction& tr = allTrans.trans(i);
		if (tr.noUse())
			continue;
		if (tr.dimensionOfVoid)
			continue;
		quint64 dist = tr.distanceWeighted<1024*1024*1024, 1024*1024*1024, maxHashDist>(modelTrans);
		if (passFilter(dist, tr)) {
			m_localStaticArgs.m_bundle.append(&tr);
		}
	}
	int numBund = m_localStaticArgs.m_bundle.count();
	if (numBund <= MIN_TRANSACTIONS_FOR_STAT) {
		m_localStaticArgs.m_bundle.clear();
		m_fitness = 0.0;
		m_billProba = 0.0;
		return m_fitness;
	}

	computeNextDayProba();

	m_billProba = m_localStaticArgs.m_dayProba;
//	m_fitness = qAbs(kindaLog(m_localStaticArgs.m_bundle.sumDollar()));
	m_fitness = 10.0;
//	m_fitness *= numBund * numBund;
//	m_fitness /= MIN_TRANSACTIONS_FOR_STAT * MIN_TRANSACTIONS_FOR_STAT;
	m_fitness *= m_localStaticArgs.m_dayProba;
	return m_fitness;
}

bool FeatureStatDistrib::passFilter(quint64 dist, const Transaction &trans) const {
	return dist < Transaction::LIMIT_DIST_TRANS
			&& trans.effect128 <=  1 + m_localStaticArgs.m_effect * EFFECT_RANGE_WIDTH_RATIO
			&& trans.effect128 >= -1 + m_localStaticArgs.m_effect / EFFECT_RANGE_WIDTH_RATIO;
}

void FeatureStatDistrib::execute(void *outDatum, Puppy::Context &ioContext)
{
	AccountFeature::execute(outDatum, ioContext);
	double& output = *(double*)outDatum;

	getArgs(ioContext);
	cleanArgs();

	// will be ALL the transactions if m_filterHash < 0
	auto& allTrans = ioContext.m_pUser->transBundle(m_filterHash);

	output = apply(allTrans);
	// isolate the transaction that were fitted to the target
	for (int i = 0; i < m_localStaticArgs.m_bundle.count(); ++i) {
		m_localStaticArgs.m_bundle.trans(i).setDimensionOfVoid();
	}

	if (ioContext.m_summaryJsonObj) {
		LOG() << getName().c_str() << " " << this
			<< " p=" << m_billProba
			<< " n=" << m_localStaticArgs.m_bundle.count()
			<< " h=" << m_localStaticArgs.m_hash
			<< endl;
		if (m_billProba > 0.001) {
			QJsonArray features = (*ioContext.m_summaryJsonObj)["features"].toArray();
			features.append(toJson(ioContext));
			ioContext.m_summaryJsonObj->insert("features", features);
		}
		for (int i = 0; i < m_localStaticArgs.m_bundle.count(); ++i) {
			const Transaction& tr = m_localStaticArgs.m_bundle.trans(i);
			emit ioContext.m_pUser->botContext()->matchedTransaction(tr.time_t(), tr.amountDbl(), 2);
		}
		OracleStatDistrib* pNewOr = new OracleStatDistrib();
		pNewOr->m_args = m_localStaticArgs;
		// making a shared pointer that will take care of cleaning once the oracle is no longer referenced
		QSharedPointer<Oracle> newOracle(pNewOr);
		ioContext.m_pUser->oracle()->addSubOracle(newOracle);
	}
}

void FeatureStatDistrib::computeNextDayProba()
{
	double daysTo = m_localStaticArgs.m_bundle.trans(0).date.daysTo(m_localStaticArgs.m_bundle.trans(1).date);
	double EMA_FACTOR = 0.25;
//	LOG() << "daysTo " << daysTo << endl;

	for (int i = 2; i < m_localStaticArgs.m_bundle.count(); ++i) {
		double daysToNext = m_localStaticArgs.m_bundle.trans(i - 1).date.daysTo(m_localStaticArgs.m_bundle.trans(i).date);
		daysTo *= (1.0 - EMA_FACTOR);
		daysTo += daysToNext * EMA_FACTOR;
//		LOG() << "daysToNext " << daysToNext << "daysTo " << daysTo << endl;
	}
	// if time since last is getting larger than when we should have seen one, we take it as a new point
	double daysToEnd = m_localStaticArgs.m_bundle.last().date.daysTo(Transaction::currentDay());
	if (daysToEnd > daysTo) {
		daysTo *= (1.0 - EMA_FACTOR);
		daysTo += daysToEnd * EMA_FACTOR;
	}
//	LOG() << "daysToEnd " << daysToEnd << " final daysTo " << daysTo << endl;
	m_localStaticArgs.m_dayProba = 1.0 / daysTo;
	// correction for proba not small
	m_localStaticArgs.m_dayProba = m_localStaticArgs.m_dayProba / (1.0 + m_localStaticArgs.m_dayProba);
}

QVector<Transaction> OracleStatDistrib::revelation(QDate upToDate)
{
	LOG() << "OracleStatDistrib::revelation proba = " << m_args.m_dayProba << " bundle = " << m_args.m_bundle.count() << endl;
	static QVector<Transaction> retVect;
	retVect.clear();
	if (m_args.m_bundle.count() == 0)
		return retVect;
	while (curDate() <= upToDate) {
		double prob = 1.0;
		while (prob > 1e-2)
		{
			prob *= m_args.m_dayProba;
			if (randBool(prob)) {
				Transaction randTr = m_args.m_bundle.randomTransaction();
				randTr.date = curDate();
				LOG() << QString("randTrans(%1) ").arg(prob) << randTr.amountDbl() << " " << randTr.date.toString() << "" << randTr.name << endl;
				retVect.append(randTr);
			}
		}
		nextDay();
	}
	return retVect;
}

double OracleStatDistrib::avgDaily() const
{
	return m_args.m_bundle.avgSmart() * m_args.m_dayProba / (1 - m_args.m_dayProba);
}
