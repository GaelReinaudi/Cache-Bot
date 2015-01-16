#include "AccRegPrimits.h"

void FeatureSalary::execute(void *outDatum, Puppy::Context &ioContext) {
	AccountFeature::execute(outDatum, ioContext);
	double& lResult = *(double*)outDatum;
	if(ioContext.m_isInFeature) {
		lResult = -40e6;
		ioContext.m_hasRecursiveFeature = true;
		return;
	}
	ioContext.m_isInFeature = true;
	getArgument(2, &m_amount, ioContext);
//	getArgument(3, &m_every, ioContext);
	getArgument(4, &m_amountDelta, ioContext);
	getArgument(5, &m_dayDelta, ioContext);
	ioContext.m_isInFeature = false;
	if(ioContext.m_hasRecursiveFeature) {
		lResult = -20e6;
		ioContext.m_hasRecursiveFeature = false;
		return;
	}
	if(lResult < 0.0)
		return;
	lResult = -10e6;
	VectorRectF outVecRec;
	if (m_every < 1 || m_endAgo < 0 || m_dur < 0 || m_amountDelta < 0 || m_amountDelta > m_amount || m_dayDelta < 0) {
		lResult = -8e6;
		return;
	}
	if (m_dur > m_every * 1024 || m_every > 365 || m_dayDelta >= m_every / 4) {
		lResult = -6e6;
		return;
	}
	if (m_dayDelta > 5 || m_endAgo > 1e6 || m_amount > 1e6) {
		lResult = -4e6;
		return;
	}
	lResult = -2e6;
	double fitness = 0.0;
	double gradePerZone = (m_amount - m_amountDelta) / (1+m_dayDelta);
	for (int i = m_endAgo; i < m_endAgo + m_dur; i += m_every) {
		//			// arbitrary grade based on (valid) input values
		//			lResult = m_amount / (1+m_amountDelta) + m_every / (1+m_dayDelta*30);
		//			lResult /= 1e6;
		QRectF zone;
		zone.setBottom(m_amount - m_amountDelta);
		zone.setTop(m_amount + m_amountDelta);
		zone.setLeft((i + m_dayDelta));
		zone.setRight((i - m_dayDelta));
		outVecRec.append(zone);
		QVector<QVector<int> > copyDailyAmounts = ioContext.m_dailyAmounts;

		for (int j = 0; j <= m_dayDelta; ++j) {
			if(i + j >= copyDailyAmounts.size())
				continue;
			else {
				for (int& pr : copyDailyAmounts[i + j]) {
					if (pr && pr <= m_amount + m_amountDelta && pr >= m_amount - m_amountDelta) {
						pr = 0;
						goto foundIt;
					}
				}
			}
			// if j not null, we try negative deltas also
			if (j && j < i) {
				for (int& pr : copyDailyAmounts[i - j]) {
					if (pr && pr <= m_amount + m_amountDelta && pr >= m_amount - m_amountDelta) {
						pr = 0;
						goto foundIt;
					}
				}
			}
		}
		fitness -= 1.2 * gradePerZone;
foundIt:
		fitness += gradePerZone;
	}
	if(fitness)
		lResult = fitness;
	if(ioContext.m_doPlot) {
		emit evoSpinner()->sendMask(outVecRec);
		ioContext.m_doPlot = false;
		QThread::msleep(100);
	}
}
