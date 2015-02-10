#include "AccRegPrimits.h"

void FeatureFixedIncome::getArgs(Puppy::Context &ioContext)
{
	getArgument(2, &m_amount, ioContext);
	getArgument(3, &m_every, ioContext);
	getArgument(4, &m_amountDelta, ioContext);
	getArgument(5, &m_dayDelta, ioContext);
}

void FeatureFixedIncome::cleanArgs()
{
	m_amount = qAbs(m_amount);
	m_every = qAbs(m_every);
	m_amountDelta = qAbs(m_amountDelta);
	m_dayDelta = qAbs(m_dayDelta);
	m_every = qMax(qMin(m_every, 365.0), 1.0);
	m_dur =  qMax(qMin(m_dur, m_every * 256), 50.0);
	m_amountDelta = qMin(m_amountDelta, m_amount * 0.5);
	m_dayDelta = qMin(m_dayDelta, m_every * 0.3);
}

void FeatureFixedIncome::execute(void *outDatum, Puppy::Context &ioContext) {
	AccountFeature::execute(outDatum, ioContext);
	double& lResult = *(double*)outDatum;
	ZoneVector outVecZone;
	getArgs(ioContext);
	cleanArgs();
	lResult = -2e6;
	double fitness = 0.0;
	int found = 0;
	int notFound = 0;
	DailyTransactions& copyDailyAmounts = ioContext.m_dailyAmounts;
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
				for (auto& t : copyDailyAmounts[dayAgo + j]) {
					if (!t->isAccountedFor() && t->amount() <= m_amount + m_amountDelta && t->amount() >= m_amount - m_amountDelta) {
						fitness += qAbs(t->amount());
						fitness -= qAbs(t->amount() - m_amount);
						t->accountFor(m_every);
						++found;
						goto foundIt;
					}
				}
			}
			// if j not null, we try negative deltas also
			if (j && j < dayAgo) {
				for (auto& tr : copyDailyAmounts[dayAgo - j]) {
					if (!tr->isAccountedFor() && tr->amount() <= m_amount + m_amountDelta && tr->amount() >= m_amount - m_amountDelta) {
						fitness += qAbs(tr->amount());
						fitness -= qAbs(tr->amount() - m_amount);
						tr->accountFor(m_every);
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

	fitness -= found * qAbs(m_amountDelta);
	fitness -= found * qAbs(m_dayDelta * 256);
	fitness -= kindaLog(m_endAgo);
	if(notFound > found / 12)
		fitness -= (notFound - found / 12) * (qAbs(m_amount) + m_amountDelta + m_dayDelta * 64);

	fitness *= kindaLog(found);
	if(fitness)
		lResult = fitness;
	if(ioContext.m_doPlot) {
		emit evoSpinner()->sendMask(outVecZone);
		qDebug() << m_endAgo << m_dur << m_amount << m_every << m_amountDelta << m_dayDelta;
		//QThread::msleep(10);
	}
}

void MonthlyPayments::execute(void *outDatum, Puppy::Context &ioContext) {
	AccountFeature::execute(outDatum, ioContext);
	double& lResult = *(double*)outDatum;
	getArgument(2, &m_amount, ioContext);
	m_amount = -qAbs(m_amount);
	m_amountDelta = -0.031 * m_amount;
//	getArgument(3, &m_every, ioContext);
	getArgument(3, &m_amountDelta, ioContext);
	getArgument(4, &m_dayDelta, ioContext);
	lResult = -10e6;
	ZoneVector outVecZone;
	m_every = qAbs(m_every);
	m_amountDelta = qAbs(m_amountDelta);
	m_dayDelta = qAbs(m_dayDelta);
	m_every = qMax(qMin(m_every, 365.0), 1.0);
	m_dur =  qMax(qMin(m_dur, m_every * 256), 30.0);
	m_amountDelta = qMin(m_amountDelta, m_amount * 0.5);
	m_dayDelta = qMin(m_dayDelta, m_every * 0.3);
	if (m_dayDelta > 17 || m_endAgo > 1e6 || m_amount < -1e6) {
		lResult = -4e6;
		return;
	}
	lResult = -2e6;
	double fitness = 0.0;
	int found = 0;
	int notFound = 0;
	DailyTransactions& copyDailyAmounts = ioContext.m_dailyAmounts;
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
				for (auto& t : copyDailyAmounts[dayAgo + j]) {
					if (!t->isAccountedFor() && t->amount() <= m_amount + m_amountDelta && t->amount() >= m_amount - m_amountDelta) {
						fitness += qAbs(t->amount());
						fitness -= qAbs(t->amount() - m_amount);
						t->accountFor(m_every);
						++found;
						goto foundIt;
					}
				}
			}
			// if j not null, we try negative deltas also
			if (j && j < dayAgo) {
				for (auto& tr : copyDailyAmounts[dayAgo - j]) {
					if (!tr->isAccountedFor() && tr->amount() <= m_amount + m_amountDelta && tr->amount() >= m_amount - m_amountDelta) {
						fitness += qAbs(tr->amount());
						fitness -= qAbs(tr->amount() - m_amount);
						tr->accountFor(m_every);
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
	fitness -= found * qAbs(3*m_amountDelta);
	fitness /= kindaLog(qMax(1.0,(m_dayDelta - 5)));
	if(notFound > found / 12)
		fitness -= (notFound - found / 12) * (qAbs(m_amount) + m_amountDelta + m_dayDelta * 64);

	fitness *= kindaLog(found);
	if(fitness)
		lResult = fitness;
	if(ioContext.m_doPlot) {
		emit evoSpinner()->sendMask(outVecZone);
		qDebug() << m_endAgo << m_dur << m_amount << m_every << m_amountDelta << m_dayDelta;
		//QThread::msleep(10);
	}
}
