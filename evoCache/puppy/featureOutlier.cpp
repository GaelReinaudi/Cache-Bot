#include "featureOutlier.h"

static const int THRESHOLD_EFFECT128 = 20;

double FeatureOutlier::apply(TransactionBundle& allTrans, bool doLog)
{
	m_localStaticArgs.m_bundle.clear();
	m_localStaticArgs.m_effect = 0;
	m_localStaticArgs.m_amount = 0;
	for (int i = 0; i < allTrans.count(); ++i) {
		const Transaction& tr = allTrans.trans(i);
		if (tr.noUse())
			continue;
		if (tr.effect128 < THRESHOLD_EFFECT128)
			continue;
		if (tr.dimensionOfVoid)
			continue;

		if (tr.effect128 > m_localStaticArgs.m_effect) {
			m_localStaticArgs.m_effect = tr.effect128;
			m_localStaticArgs.m_bundle.clear();
			m_localStaticArgs.m_bundle.append(&tr);
			m_localStaticArgs.m_amount = tr.amountDbl();
		}
	}
	Q_ASSERT(m_localStaticArgs.m_bundle.count() <= 1);

	double tempFitness = double(m_localStaticArgs.m_effect) / double(THRESHOLD_EFFECT128);
	return tempFitness;
}

void FeatureOutlier::execute(void* outDatum, Puppy::Context &ioContext)
{
	AccountFeature::execute(outDatum, ioContext);

	if (ioContext.m_summaryJsonObj) {
		for (int i = 0; i < m_localStaticArgs.m_bundle.count(); ++i) {
			const Transaction& tr = m_localStaticArgs.m_bundle.trans(i);
			emit ioContext.m_pUser->botContext()->matchedTransaction(tr.time_t(), tr.amountDbl(), 4);
		}
		OracleOutlier* pNewOr = new OracleOutlier(this);
		pNewOr->m_args = m_localStaticArgs;
		// making a shared pointer that will take care of cleaning once the oracle is no longer referenced
		QSharedPointer<Oracle> newOracle(pNewOr);
		ioContext.m_pUser->oracle()->addSubOracle(newOracle);
	}
}
