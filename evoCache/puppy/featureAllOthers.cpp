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
		Transaction& trans = allTrans.trans(i);
		if (trans.isInternal())
			continue;

		double amnt = trans.amountDbl();
		if (amnt > 0.0 && amnt < 2 * 10000) {
			++totPos;
			if (trans.dimensionOfVoid) {
				++alreadyMatchedPos;
				continue;
			}
			++m_localStaticArgs.m_numPos;
			m_localStaticArgs.m_sumPos += amnt;
		}
		if (amnt < 0.0) {
			++totNeg;
			if (trans.dimensionOfVoid) {
				++alreadyMatchedNeg;
				continue;
			}
			++m_localStaticArgs.m_numNeg;
			m_localStaticArgs.m_sumNeg += amnt;
		}

		++trans.dimensionOfVoid;
		m_localStaticArgs.m_bundle.append(&trans);
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
	auto& allTrans = ioContext.m_pUser->transBundle(m_filterHash);

	output = apply(allTrans);

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
//		if (randBool(m_args.m_dayProba))
		{
			Transaction randTr;// = m_args.m_bundle.randomTransaction();
			randTr.date = curDate();
			double avgAmnt = m_args.m_sumPos + m_args.m_sumNeg;
			// Note that for now the average is done from the oldes date of the account readings
			// to the current date, so it slowly dissolves as the oracle is predicting for latter dates
			avgAmnt /= Transaction::onlyAfterDate.daysTo(curDate());
			randTr.setAmount(avgAmnt);
			LOG() << "avgTrans " << randTr.amountDbl() << " " << randTr.date.toString() << randTr.name << endl;
			retVect.append(randTr);
		}
		nextDay();
	}
	return retVect;
}
