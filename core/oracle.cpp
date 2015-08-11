#include "oracle.h"

QVector<Transaction> SuperOracle::revelation(QDate upToDate)
{
	LOG() << "SuperOracle::revelation from " << m_subOracles.count() << endl;
	QVector<Transaction> ret;
	// naive adding up the revelations for now
	for (auto pOr : m_subOracles) {
		ret += pOr->revelation(upToDate);
	}
	std::stable_sort(ret.begin(), ret.end(), Transaction::smallerAmountThan);
	std::stable_sort(ret.begin(), ret.end(), Transaction::earlierThan);

	for (const Transaction& tr : ret) {
		LOG() << "t =" << QDate::currentDate().daysTo(tr.date) << "amnt =" << tr.amountDbl()
			  << "    label = " << tr.name << endl;
	}

	return ret;
}

double SuperOracle::avgDaily() const
{
	LOG() << "SuperOracle::avgDaily" << endl;
	double avg = 0.0;
	for (auto pOr : m_subOracles) {
		avg += pOr->avgDaily();
	}
	return avg;
}

double SuperOracle::avgCashFlow() const
{
	LOG() << "SuperOracle::avgCashFlow" << endl;
	double posAvg = 0.0;
	double negAvg = 0.0;
	for (auto pOr : m_subOracles) {
		double avg = pOr->avgDaily();
		if (avg > 0) {
			posAvg += avg;
		}
		else if (avg < 0) {
			negAvg += avg;
		}
		else {
			posAvg += pOr->avgDailyPos();
			negAvg += pOr->avgDailyNeg();
		}
	}
	if (posAvg == 0)
		return -1.0;
	return (posAvg + negAvg) / posAvg;
}
