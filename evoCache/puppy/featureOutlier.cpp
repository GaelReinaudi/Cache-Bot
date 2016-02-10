#include "featureOutlier.h"

static const int THRESHOLD_EFFECT128 = 30;

double FeatureOutlier::apply(TransactionBundle& allTrans, bool isPostTreat, bool doLog)
{
	Q_UNUSED(doLog);
	m_localStaticArgs.m_bundle.clear();
	m_localStaticArgs.m_effect = 0;
	m_localStaticArgs.m_amount = 0;
	for (int i = 0; i < allTrans.count(); ++i) {
		const Transaction& tr = allTrans.trans(i);
		if (tr.noUse())
			continue;
		if (tr.effect128 < THRESHOLD_EFFECT128)
			continue;
		if (tr.dimOfVoid)
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

void FeatureOutlier::emitGraphics(Puppy::Context& ioContext) const
{
	for (int i = 0; i < m_localStaticArgs.m_bundle.count(); ++i) {
		const Transaction& tr = m_localStaticArgs.m_bundle.trans(i);
		emit ioContext.m_pUser->botContext()->matchedTransaction(tr.time_t(), tr.amountDbl(), 4);
	}
}




