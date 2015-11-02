#include "oracle.h"
#include "AccRegPrimits.h"

QVector<Transaction> SuperOracle::revelation(QDate upToDate)
{
	NOTICE() << "SuperOracle::revelation from " << m_subOracles.count();
	QVector<Transaction> ret;
	// naive adding up the revelations for now
	for (auto pOr : m_subOracles) {
		ret += pOr->revelation(upToDate);
	}
	std::stable_sort(ret.begin(), ret.end(), Transaction::smallerAmountThan);
	std::stable_sort(ret.begin(), ret.end(), Transaction::earlierThan);

	for (const Transaction& tr : ret) {
		DBG() << "t =" << Transaction::currentDay().daysTo(tr.date) << "amnt =" << tr.amountDbl()
			  << "    label = " << tr.name;
	}

	return ret;
}

double SuperOracle::avgDaily() const
{
	NOTICE() << "SuperOracle::avgDaily";
	double avg = 0.0;
	for (auto pOr : m_subOracles) {
		avg += pOr->avgDaily();
	}
	return avg;
}

SuperOracle::Summary SuperOracle::computeAvgCashFlow(bool includeOracleSummaries) const
{
	DBG() << "SuperOracle::avgCashFlow";
	SuperOracle::Summary summary;
	int ind = -1;
	for (auto pOr : m_subOracles) {
		++ind;
		double avg = pOr->avgDaily();
		if (avg > 0) {
			summary.posSum += avg;
			if (pOr->feature()->isPeriodic())
				summary.salary += avg;
			INFO() << ind <<  " +daily " << avg << "      " << pOr->feature()->getName();
		}
		else if (avg < 0) {
			summary.negSum += avg;
			if (pOr->feature()->isPeriodic())
				summary.bill += avg;
			INFO() << ind <<  " -daily " << avg << "      " << pOr->feature()->getName();
		}
		else {
			summary.posSum += pOr->avgDailyPos();
			summary.negSum += pOr->avgDailyNeg();
			if (pOr->feature()->isPeriodic()) {
				summary.salary += pOr->avgDailyPos();
				summary.bill += pOr->avgDailyNeg();
			}
			INFO() << ind <<  " 0daily = " << avg << " " << pOr->feature()->getName()
				  << " " << pOr->avgDailyPos()
				  << " " << pOr->avgDailyNeg()
					 ;
		}
		summary.dailyPerOracle.append(avg);
		if (includeOracleSummaries)
			summary.summaryPerOracle.append(pOr->toJson());
	}
	if (summary.posSum == 0.0) {
		WARN() << "SuperOracle::avgCashFlow posAvg == 0.0 ";
		return summary;
	}
	NOTICE() << "SuperOracle::avgCashFlow " << summary.flow();
//	if (summary.flow < -1.0)
//		summary.flow = -1.0;
//	if (summary.flow > 1.0)
//		summary.flow = 1.0;

	return summary;
}

QJsonObject Oracle::toJson() const {
	QJsonObject ret;
	ret["postTreat"] = isPostTreatment;
	ret["type"] = QString::fromStdString(feature()->getName());
	ret["descr"] = description();
	return ret;
}
