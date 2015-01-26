#include "AccRegPrimits.h"

void FeatureFixedIncome::execute(void *outDatum, Puppy::Context &ioContext) {
	AccountFeature::execute(outDatum, ioContext);
	double& lResult = *(double*)outDatum;
	if(ioContext.m_isInFeature) {
		lResult = -40e6;
		ioContext.m_hasRecursiveFeature = true;
		return;
	}
	ioContext.m_isInFeature = true;
	getArgument(2, &m_amount, ioContext);
	getArgument(3, &m_every, ioContext);
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
	ZoneVector outVecZone;
	m_amount = qAbs(m_amount);
	m_every = qAbs(m_every);
	m_amountDelta = qAbs(m_amountDelta);
	m_dayDelta = qAbs(m_dayDelta);
//	if (m_every < 0 || m_endAgo < 0 || m_dur < 0 || m_amountDelta < 0 || m_dayDelta < 0) {
//		lResult = -8e6;
//		return;
//	}
	m_dur = qMin(m_dur, m_every * 256);
	m_amountDelta = qMin(m_amountDelta, m_amount * 0.5);
	m_dayDelta = qMin(m_dayDelta, m_every * 0.3);
//	if (m_dayDelta > m_every / 2) {
//		lResult = -6e6;
//		return;
//	}
	if (m_dayDelta > 17 || m_endAgo > 1e6 || m_amount > 1e6) {
		lResult = -4e6;
		return;
	}
	lResult = -2e6;
	double fitness = 0.0;
	int found = 0;
	int notFound = 0;
	DailyTransactions copyDailyAmounts = ioContext.m_dailyAmounts;
	for (double dblDay = m_endAgo; dblDay < m_endAgo + m_dur; dblDay += m_every) {
		int dayAgo = dblDay;

		Zone zone;
		zone.setBottom(m_amount - m_amountDelta);
		zone.setTop(m_amount + m_amountDelta);
		zone.setLeft((dayAgo + m_dayDelta));
		zone.setRight((dayAgo - m_dayDelta));
		outVecZone.append(zone);

		for (int j = 0; j <= m_dayDelta; ++j) {
			if(dayAgo + j >= copyDailyAmounts.size())
				continue;
			else {
				for (Transaction& tr : copyDailyAmounts[dayAgo + j]) {
					if (!tr.isAccountedFor() && tr.amount() <= m_amount + m_amountDelta && tr.amount() >= m_amount - m_amountDelta) {
						fitness += tr.amount();
						fitness -= qAbs(tr.amount() - m_amount);
						tr.accountFor();
						++found;
						goto foundIt;
					}
				}
			}
			// if j not null, we try negative deltas also
			if (j && j < dayAgo) {
				for (Transaction& tr : copyDailyAmounts[dayAgo - j]) {
					if (!tr.isAccountedFor() && tr.amount() <= m_amount + m_amountDelta && tr.amount() >= m_amount - m_amountDelta) {
						fitness += tr.amount();
						fitness -= qAbs(tr.amount() - m_amount);
						tr.accountFor();
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
		outVecZone.last().m_isFilled = true;
		continue;
	}
	//fitness /= qLn(3 + m_dayDelta) + qLn(3 + m_amountDelta);
//	fitness -= m_dayDelta * 100;
//	fitness -= m_amountDelta;
	if(notFound > 0)
		fitness -= notFound * (m_amount + m_amountDelta + m_dayDelta * 64);

	if(fitness)
		lResult = fitness;
	if(ioContext.m_doPlot) {
		emit evoSpinner()->sendMask(outVecZone);
		qDebug() << m_endAgo << m_dur << m_amount << m_every << m_amountDelta << m_dayDelta;
		ioContext.m_doPlot = false;
		//QThread::msleep(10);
	}
}

void MonthlyPayments::execute(void *outDatum, Puppy::Context &ioContext) {
	AccountFeature::execute(outDatum, ioContext);
	double& lResult = *(double*)outDatum;
	ioContext.m_isInFeature = true;
	getArgument(2, &m_amount, ioContext);
//	getArgument(3, &m_every, ioContext);
	getArgument(3, &m_amountDelta, ioContext);
	getArgument(4, &m_dayDelta, ioContext);
	lResult = -10e6;
	ZoneVector outVecZone;
	m_amount = -qAbs(m_amount);
	m_every = qAbs(m_every);
	m_amountDelta = qAbs(m_amountDelta);
	m_dayDelta = qAbs(m_dayDelta);
	m_dur = qMin(m_dur, m_every * 256);
	m_amountDelta = qMin(m_amountDelta, -m_amount * 0.5);
	m_dayDelta = qMin(m_dayDelta, m_every * 0.3);
	if (m_dayDelta > 17 || m_endAgo > 1e6 || m_amount < -1e6) {
		lResult = -4e6;
		return;
	}
	lResult = -2e6;
	double fitness = 0.0;
	int found = 0;
	int notFound = 0;
	DailyTransactions copyDailyAmounts = ioContext.m_dailyAmounts;
	for (double dblDay = m_endAgo; dblDay < m_endAgo + m_dur; dblDay += m_every) {
		int dayAgo = dblDay;

		Zone zone;
		zone.setBottom(m_amount - m_amountDelta);
		zone.setTop(m_amount + m_amountDelta);
		zone.setLeft((dayAgo + m_dayDelta));
		zone.setRight((dayAgo - m_dayDelta));
		outVecZone.append(zone);

		for (int j = 0; j <= m_dayDelta; ++j) {
			if(dayAgo + j >= copyDailyAmounts.size())
				continue;
			else {
				for (Transaction& tr : copyDailyAmounts[dayAgo + j]) {
					if (!tr.isAccountedFor() && tr.amount() <= m_amount + m_amountDelta && tr.amount() >= m_amount - m_amountDelta) {
						fitness += qAbs(tr.amount());
						fitness -= qAbs(tr.amount() - m_amount);
						tr.accountFor();
						++found;
						goto foundIt;
					}
				}
			}
			// if j not null, we try negative deltas also
			if (j && j < dayAgo) {
				for (Transaction& tr : copyDailyAmounts[dayAgo - j]) {
					if (!tr.isAccountedFor() && tr.amount() <= m_amount + m_amountDelta && tr.amount() >= m_amount - m_amountDelta) {
						fitness += qAbs(tr.amount());
						fitness -= qAbs(tr.amount() - m_amount);
						tr.accountFor();
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
		outVecZone.last().m_isFilled = true;
		continue;
	}
	if(notFound > 0)
		fitness -= notFound * (qAbs(m_amount) + m_amountDelta + m_dayDelta * 64);

	if(fitness)
		lResult = fitness;
	if(ioContext.m_doPlot) {
		emit evoSpinner()->sendMask(outVecZone);
		qDebug() << m_endAgo << m_dur << m_amount << m_every << m_amountDelta << m_dayDelta;
		ioContext.m_doPlot = false;
		//QThread::msleep(10);
	}
}
