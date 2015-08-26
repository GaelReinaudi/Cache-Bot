#include "featureAllOthers.h"

double FeatureAllOthers::apply(TransactionBundle& allTrans)
{
	m_billProba = 1000000.0;
	m_localStaticArgs.m_bundle.clear();
	m_localStaticArgs.m_numPos = 0;
	m_localStaticArgs.m_numNeg = 0;
	m_localStaticArgs.m_sumPos = 0;
	m_localStaticArgs.m_sumNeg = 0;
	double totPos = 0;
	double totNeg = 0;
	double alreadyMatchedPos = 0;
	double alreadyMatchedNeg = 0;
	for (int i = 0; i < allTrans.count(); ++i) {
		const Transaction& tr = allTrans.trans(i);
		if (tr.isInternal())
			continue;
		if (tr.isFuture())
			continue;

		double amnt = tr.amountDbl();
		// BONUS OUT HACK
		if (amnt > 2 * 10000)
			continue;
		if (amnt > 0.0) {
			++totPos;
			if (tr.dimensionOfVoid) {
				++alreadyMatchedPos;
				continue;
			}
			++m_localStaticArgs.m_numPos;
			m_localStaticArgs.m_sumPos += amnt;
		}
		if (amnt < 0.0) {
			++totNeg;
			if (tr.dimensionOfVoid) {
				++alreadyMatchedNeg;
				continue;
			}
			++m_localStaticArgs.m_numNeg;
			m_localStaticArgs.m_sumNeg += amnt;
		}

		m_localStaticArgs.m_bundle.append(&tr);
	}

	int numBund = m_localStaticArgs.m_bundle.count();
	int MIN_TRANSACTIONS_FOR_RAND = 3;
	if (numBund <= MIN_TRANSACTIONS_FOR_RAND) {
		m_localStaticArgs.m_dayProba = 0.0;
	}
	else {
		// get the date those transaction started
		QDate firstDate = m_localStaticArgs.m_bundle.trans(0).date;

		m_localStaticArgs.m_daysBundle = firstDate.daysTo(Transaction::currentDay());
		m_localStaticArgs.m_dayProba = numBund / m_localStaticArgs.m_daysBundle;
		// correction for proba not small
		m_localStaticArgs.m_dayProba = m_localStaticArgs.m_dayProba / (1.0 + m_localStaticArgs.m_dayProba);
	}

	// min of the ration already/tot per side Neg/Pos
	m_fitness = qMin(alreadyMatchedPos / totPos, alreadyMatchedNeg / totNeg);
	m_fitness *= 100.0;
//	m_fitness = qMax(m_fitness, 1.0);
	return m_fitness;
}

void FeatureAllOthers::execute(void *outDatum, Puppy::Context &ioContext)
{
	AccountFeature::execute(outDatum, ioContext);
	double& output = *(double*)outDatum;

	getArgs(ioContext);
	cleanArgs();

	output = m_fitness = 0.0;

	// if we already have aplied this feature, nothing to be done here.
	if (ioContext.flags & Puppy::Context::AllOthers) {
		return;
	}
	ioContext.flags |= Puppy::Context::AllOthers;

	// will be ALL the transactions if m_filterHash < 0
	TransactionBundle& allTrans = ioContext.m_pUser->transBundle(m_filterHash);

	output = apply(allTrans);
	// isolate the transaction that were fitted to the target
	for (int i = 0; i < m_localStaticArgs.m_bundle.count(); ++i) {
		m_localStaticArgs.m_bundle.trans(i).setDimensionOfVoid();
	}

	// summary if the json object exists
	if (ioContext.m_summaryJsonObj) {
		if(m_billProba > 0.0) {
			QJsonArray features = (*ioContext.m_summaryJsonObj)["features"].toArray();
			features.append(toJson(ioContext));
			ioContext.m_summaryJsonObj->insert("features", features);
		}
		OracleFilteredRest* pNewOr = new OracleFilteredRest();
		pNewOr->m_args = m_localStaticArgs;
		// making a shared pointer that will take care of cleaning once the oracle is no longer referenced
		QSharedPointer<Oracle> newOracle(pNewOr);
		ioContext.m_pUser->oracle()->addSubOracle(newOracle);
	}
}



QVector<Transaction> OracleFilteredRest::revelation(QDate upToDate)
{
	LOG() << "OracleFilteredRest::revelation sumPos = " << m_args.m_sumPos << " sumNeg = " << m_args.m_sumNeg << " bundle = " << m_args.m_bundle.count() << endl;
	static QVector<Transaction> retVect;
	retVect.clear();
	while (curDate() <= upToDate) {
		double prob = 1.0;
		while (prob > 1e-2)
		{
			prob *= m_args.m_dayProba;
			if (randBool(prob)) {
				Transaction randTr = m_args.m_bundle.randomTransaction();
				randTr.date = curDate();
				double avgAmnt = m_args.m_sumPos + m_args.m_sumNeg;
				// Note that for now the average is done from the oldes date of the account readings
				// to the current date, so it slowly dissolves as the oracle is predicting for latter dates
				avgAmnt /= Transaction::onlyAfterDate.daysTo(curDate());
				// randTr.setAmount(avgAmnt / m_args.m_dayProba);
				LOG() << QString("allOthTrans(%1) ").arg(prob) << randTr.amountDbl() << " " << randTr.date.toString() << "" << randTr.name << endl;
				retVect.append(randTr);
			}
		}
		nextDay();
	}
	return retVect;
}

double OracleFilteredRest::avgDaily() const
{
	return 0.0;
}
