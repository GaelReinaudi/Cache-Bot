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
	if (m_dayDelta > 7 || m_endAgo > 1e6 || m_amount > 1e6) {
		lResult = -4e6;
		return;
	}
	lResult = -2e6;
	double fitness = 0.0;
	int found = 0;
	int notFound = 0;
	for (double dblDay = m_endAgo; dblDay < m_endAgo + m_dur; dblDay += m_every) {
		int dayAgo = dblDay;
		QVector<QVector<int> > copyDailyAmounts = ioContext.m_dailyAmounts;

		for (int j = 0; j <= m_dayDelta; ++j) {
			if(dayAgo + j >= copyDailyAmounts.size())
				continue;
			else {
				for (int& pr : copyDailyAmounts[dayAgo + j]) {
					if (pr && pr <= m_amount + m_amountDelta && pr >= m_amount - m_amountDelta) {
						fitness += pr;
						pr = 0;
						++found;
						goto foundIt;
					}
				}
			}
			// if j not null, we try negative deltas also
			if (j && j < dayAgo) {
				for (int& pr : copyDailyAmounts[dayAgo - j]) {
					if (pr && pr <= m_amount + m_amountDelta && pr >= m_amount - m_amountDelta) {
						fitness += pr;
						pr = 0;
						++found;
						goto foundIt;
					}
				}
			}
		}
		++notFound;
		continue;
// C++ goto
foundIt:
		QRectF zone;
		zone.setBottom(m_amount - m_amountDelta);
		zone.setTop(m_amount + m_amountDelta);
		zone.setLeft((dayAgo + m_dayDelta));
		zone.setRight((dayAgo - m_dayDelta));
		outVecRec.append(zone);
		continue;
	}
	if(notFound > 0)
		fitness -= notFound * (m_amount + m_amountDelta + m_dayDelta * 64);

	//fitness /= 1 + m_amountDelta;

	if(fitness)
		lResult = fitness;
	if(ioContext.m_doPlot) {
		emit evoSpinner()->sendMask(outVecRec);
		ioContext.m_doPlot = false;
		QThread::msleep(100);
	}
}
