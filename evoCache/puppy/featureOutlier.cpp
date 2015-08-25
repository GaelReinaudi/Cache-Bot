#include "featureOutlier.h"

static const int THRESHOLD_EFFECT128 = 20;

double FeatureOutlier::apply(TransactionBundle& allTrans)
{
	m_localStaticArgs.m_bundle.clear();
	m_localStaticArgs.m_effect = 0;
	m_localStaticArgs.m_amount = 0;
	for (int i = 0; i < allTrans.count(); ++i) {
		const Transaction& tr = allTrans.trans(i);
		if (tr.isInternal())
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

	m_fitness = double(m_localStaticArgs.m_effect) / double(THRESHOLD_EFFECT128);
	return m_fitness;
}

void FeatureOutlier::execute(void *outDatum, Puppy::Context &ioContext)
{
	AccountFeature::execute(outDatum, ioContext);
	double& output = *(double*)outDatum;

	getArgs(ioContext);
	cleanArgs();

	output = m_fitness = 0.0;

	// will be ALL the transactions if m_filterHash < 0
	TransactionBundle& allTrans = ioContext.m_pUser->transBundle(m_filterHash);

	output = apply(allTrans);
	// isolate the transaction that were fitted to the target
	for (int i = 0; i < m_localStaticArgs.m_bundle.count(); ++i) {
		m_localStaticArgs.m_bundle.trans(i).setDimensionOfVoid();
	}

	// summary if the json object exists
	if (ioContext.m_summaryJsonObj) {
		if(m_fitness > 0.0) {
			QJsonArray features = (*ioContext.m_summaryJsonObj)["features"].toArray();
			features.append(toJson(ioContext));
			ioContext.m_summaryJsonObj->insert("features", features);
		}
		for (int i = 0; i < m_localStaticArgs.m_bundle.count(); ++i) {
			const Transaction& tr = m_localStaticArgs.m_bundle.trans(i);
			emit ioContext.m_pUser->botContext()->matchedTransaction(tr.time_t(), tr.amountDbl(), 4);
		}
		OracleOutlier* pNewOr = new OracleOutlier();
		pNewOr->m_args = m_localStaticArgs;
		// making a shared pointer that will take care of cleaning once the oracle is no longer referenced
		QSharedPointer<Oracle> newOracle(pNewOr);
		ioContext.m_pUser->oracle()->addSubOracle(newOracle);
	}
}
