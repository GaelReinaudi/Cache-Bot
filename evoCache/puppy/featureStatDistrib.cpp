#include "featureStatDistrib.h"

void FeatureStatDistrib::getArgs(Puppy::Context &ioContext) {
	double a = 0;
	int ind = -1;
	getArgument(++ind, &a, ioContext);
	m_localStaticArgs.m_hash = a;
	getArgument(++ind, &a, ioContext);
	m_localStaticArgs.m_kla = a;
	getArgument(++ind, &a, ioContext);
	m_localStaticArgs.m_effect = a;
}

qint64 FeatureStatDistrib::distCalc(const Transaction& tr, const Transaction& modelTrans) const
{
	static const int maxHashDist = 4;
	qint64 dist = tr.distanceWeighted<1024*1024*1024, 1024*1024*1024, maxHashDist>(modelTrans);
	return dist;
}

double FeatureStatDistrib::apply(TransactionBundle& allTrans, bool isPostTreat, bool doLog)
{
	Q_UNUSED(doLog);
	DBG() << "FeatureStatDistrib::apply";
	// transaction to compare the hash with
	Transaction modelTrans;
	modelTrans.nameHash.setFromHash(m_localStaticArgs.m_hash);
	modelTrans.categoryHash.setFromHash(m_localStaticArgs.m_hash);
//	modelTrans.setAmount(-1.0); // only negative prices will have a usable distance
	modelTrans.date = Transaction::currentDay();

	m_localStaticArgs.m_bundle.clear();
	for (int i = 0; i < allTrans.count(); ++i) {
		const Transaction& tr = allTrans.trans(i);
		if (tr.noUse())
			continue;
		if (tr.isVoid())
			continue;
		qint64 dist = distCalc(tr, modelTrans);
		if (passFilter(dist, tr)) {
			m_localStaticArgs.m_bundle.append(&tr);
		}
	}
	int numBund = m_localStaticArgs.m_bundle.count();
	if (numBund < 1 || (!isPostTreat && numBund < minTransactionForBundle())) {
		m_localStaticArgs.m_bundle.clear();
		m_localStaticArgs.m_dayProba = 0.0;
		return 0.0;
	}

	computeNextDayProba();

	double tempFitness = 30.0;
	tempFitness *= m_localStaticArgs.m_dayProba;
	return tempFitness;
}

void FeatureStatDistrib::emitGraphics(Puppy::Context& ioContext) const
{
	if (ioContext.isPostTreatment)
		return;
	for (int i = 0; i < m_localStaticArgs.m_bundle.count(); ++i) {
		const Transaction& tr = m_localStaticArgs.m_bundle.trans(i);
		emit ioContext.m_pUser->botContext()->matchedTransaction(tr.time_t(), tr.amountDbl(), 2);
	}
}

void FeatureStatDistrib::isolateBundledTransactions(bool isPostTreatment /*= false*/)
{
	// isolate the transaction that were fitted to the target
	for (int i = 0; i < localStaticArgs()->m_bundle.count(); ++i) {
		localStaticArgs()->m_bundle.trans(i).setDimensionOfVoid(isPostTreatment ? 2 : 1);
	}
}

void FeatureStatDistrib::computeNextDayProba()
{
	double daysToNext = m_localStaticArgs.m_bundle.daysToNextSmart();
	m_localStaticArgs.m_dayProba = 1.0 / daysToNext;
	// correction for proba not small
	m_localStaticArgs.m_dayProba = m_localStaticArgs.m_dayProba / (1.0 + m_localStaticArgs.m_dayProba);

	double avgDaily = m_localStaticArgs.m_bundle.avgSmart() * m_localStaticArgs.m_dayProba / (1 - m_localStaticArgs.m_dayProba);
	if (User::declaredIncome && avgDaily > User::declaredIncome * 1.0) {
		m_localStaticArgs.m_dayProba = 1.0 / Transaction::maxDaysOld();
	}
}

QVector<Transaction> OracleStatDistrib::revelation(QDate upToDate)
{
	DBG() << "OracleStatDistrib::revelation proba = " << m_args.m_dayProba << " bundle = " << m_args.m_bundle.count();
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
				Transaction randTr = m_args.m_bundle.randSmart();
				randTr.date = curDate();
				DBG() << QString("randTrans(%1) ").arg(prob) << randTr.amountDbl() << " " << randTr.date.toString() << " " << randTr.name;
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
