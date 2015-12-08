#include "featureAllOthers.h"

double FeatureAllOthers::apply(TransactionBundle& allTrans, bool isPostTreat, bool doLog)
{
	Q_UNUSED(doLog);
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
		if (tr.noUse())
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
		m_localStaticArgs.m_daysBundle = Transaction::maxDaysOld();//firstDate.daysTo(Transaction::currentDay());
		m_localStaticArgs.m_dayProba = numBund / m_localStaticArgs.m_daysBundle;
		// correction for proba not small
		m_localStaticArgs.m_dayProba = m_localStaticArgs.m_dayProba / (1.0 + m_localStaticArgs.m_dayProba);
	}

	// min of the ration already/tot per side Neg/Pos
	double tempFitness = qMin(alreadyMatchedPos / totPos, alreadyMatchedNeg / totNeg);
	tempFitness *= 100.0;

	return tempFitness;
}

QVector<Transaction> OracleFilteredRest::revelation(QDate upToDate)
{
	INFO() << "OracleFilteredRest::revelation sumPos = " << m_args.m_sumPos << " sumNeg = " << m_args.m_sumNeg << " bundle = " << m_args.m_bundle.count();
	static QVector<Transaction> retVect;
	retVect.clear();
	while (curDate() <= upToDate) {
		double prob = 1.0;
		while (prob > 1e-2)
		{
			prob *= m_args.m_dayProba;
			if (randBool(prob)) {
				Transaction randTr = m_args.m_bundle.randSmart();
				randTr.date = curDate();
				double avgAmnt = m_args.m_sumPos + m_args.m_sumNeg;
				// Note that for now the average is done from the oldes date of the account readings
				// to the current date, so it slowly dissolves as the oracle is predicting for latter dates
				avgAmnt /= Transaction::maxDaysOld() + Transaction::currentDay().daysTo(curDate());
				// randTr.setAmount(avgAmnt / m_args.m_dayProba);
				INFO() << QString("allOthTrans(%1) ").arg(prob) << randTr.amountDbl() << " " << randTr.date.toString() << " " << randTr.name;
				retVect.append(randTr);
			}
		}
		nextDay();
	}
	return retVect;
}

double OracleFilteredRest::avgDaily() const
{
	DBG() << m_args.m_dayProba << m_args.m_daysBundle
			 << m_args.m_sumNeg << m_args.m_sumPos;
	return 0.0;
}
