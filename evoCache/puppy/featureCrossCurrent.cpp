#include "featureCrossCurrent.h"

double FeatureCrossCurrent::apply(TransactionBundle& allTrans, bool isPostTreat, bool doLog)
{
	Q_UNUSED(doLog);
	m_localStaticArgs.m_bundle.clear();
	m_localStaticArgs.m_effect = 0;
	m_localStaticArgs.m_amount = 0;
	for (int i = 0; i < allTrans.count(); ++i) {
		const Transaction& tr = allTrans.trans(i);
		if (tr.noUse())
			continue;
		double ref = 0.3 + kindaLog(qMax(0.0, User::declaredRent) * 30);
		if (ref < 1.0)
			ref = 3.9;
		if (-tr.kla() < ref || tr.userFlag & Transaction::UserInputFlag::Reimbursed)
			continue;
		if (tr.isVoid())
			continue;

		double posKla = qAbs(tr.kla());

		for (int j = 0; j < allTrans.count(); ++j) {
			const Transaction& other = allTrans.trans(j);
			if (other.kla() < 0)
				continue;
			if (qAbs(other.date.daysTo(tr.date)) > 15)
				continue;
			if (qAbs(other.kla() - posKla) > 0.2)
				continue;
			// here we pair
			m_localStaticArgs.m_bundle.clear();
			m_localStaticArgs.m_bundle.append(&tr);
			m_localStaticArgs.m_bundle.append(&other);
			m_localStaticArgs.m_amount = tr.amountDbl();
			return 10 * posKla;
		}
	}

	return 0.0;
}

void FeatureCrossCurrent::emitGraphics(Puppy::Context& ioContext) const
{
	for (int i = 0; i < m_localStaticArgs.m_bundle.count(); ++i) {
		const Transaction& tr = m_localStaticArgs.m_bundle.trans(i);
		emit ioContext.m_pUser->botContext()->matchedTransaction(tr.time_t(), tr.amountDbl(), 4);
	}
}




