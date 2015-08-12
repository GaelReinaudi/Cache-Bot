#include "featureOutlier.h"

static const int THRESHOLD_EFFECT128 = 20;

double FeatureOutlier::apply(TransactionBundle& allTrans)
{
	m_localStaticArgs.m_bundle.clear();
	for (int i = 0; i < allTrans.count(); ++i) {
		Transaction& trans = allTrans.trans(i);
		if (trans.isInternal())
			continue;
		if (trans.effect128 < THRESHOLD_EFFECT128)
			continue;
		if (trans.dimensionOfVoid)
			continue;

		++trans.dimensionOfVoid;
		m_localStaticArgs.m_bundle.append(&trans);
		m_localStaticArgs.m_amount = trans.amountDbl();
		m_localStaticArgs.m_effect = trans.effect128;
		m_fitness = m_localStaticArgs.m_effect;

		break;
	}
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

	// summary if the json object exists
	if (ioContext.m_summaryJsonObj) {
		if(m_fitness > 0.0) {
			QJsonArray features = (*ioContext.m_summaryJsonObj)["features"].toArray();
			features.append(toJson(ioContext));
			ioContext.m_summaryJsonObj->insert("features", features);
		}
		for (int i = 0; i < m_localStaticArgs.m_bundle.count(); ++i) {
			Transaction& t = m_localStaticArgs.m_bundle.trans(i);
			emit ioContext.m_pUser->botContext()->matchedTransaction(t.time_t(), t.amountDbl(), 4);
		}
		OracleOutlier* pNewOr = new OracleOutlier();
		pNewOr->m_args = m_localStaticArgs;
		// making a shared pointer that will take care of cleaning once the oracle is no longer referenced
		QSharedPointer<Oracle> newOracle(pNewOr);
		ioContext.m_pUser->oracle()->addSubOracle(newOracle);
	}
}
