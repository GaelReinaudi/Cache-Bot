#include "oracle.h"
#include "AccRegPrimits.h"

QVector<Transaction> SuperOracle::revelation(QDate upToDate)
{
	DBG(2) << "SuperOracle::revelation from " << m_subOracles.count();
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
			DBG(2) << ind <<  " +daily " << avg << "      " << pOr->feature()->getName();
		}
		else if (avg < 0) {
			summary.negSum += avg;
			if (pOr->feature()->isPeriodic())
				summary.bill += avg;
			DBG(2) << ind <<  " -daily " << avg << "      " << pOr->feature()->getName();
		}
		else {
			summary.posSum += pOr->avgDailyPos();
			summary.negSum += pOr->avgDailyNeg();
			if (pOr->feature()->isPeriodic()) {
				summary.salary += pOr->avgDailyPos();
				summary.bill += pOr->avgDailyNeg();
			}
			DBG(2) << ind <<  " 0daily = " << avg << " " << pOr->feature()->getName()
				  << " " << pOr->avgDailyPos()
				  << " " << pOr->avgDailyNeg()
					 ;
		}
		summary.dailyPerOracle.append(avg);
		if (includeOracleSummaries)
			summary.summaryPerOracle.append(pOr->toJson());
	}
	if (summary.posSum == 0.0) {
		DBG(3) << "SuperOracle::avgCashFlow posAvg == 0.0 ";
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

SuperOracle::Summary SuperOracle::Summary::effectOf(const SuperOracle::Summary &endSummary, int overDays) const {
	Q_ASSERT(dailyPerOracle.count() == endSummary.dailyPerOracle.count());
	// *this is the starting Summary, ie the N days ago Summary
	// effect is constructed from the avg in order to save memory
	Summary effect = (*this + endSummary) * 0.5;
	Summary delta = endSummary - *this;
	double posSlope = effect.posPartialDif();
	double negSlope = effect.negPartialDif();
	for (int i = 0; i < effect.dailyPerOracle.count(); ++i) {

		QJsonObject newSum = endSummary.summaryPerOracle[i];
		effect.summaryPerOracle[i] = QJsonObject();
		QJsonArray agoTrans = summaryPerOracle[i]["trans"].toArray();
		QJsonArray newTrans = newSum["trans"].toArray();
		for (int i = newTrans.count() - 1; i >= 0; --i) {
			if (agoTrans.contains(newTrans.at(i))) {
				newTrans.removeAt(i);
			}
		}
		effect.summaryPerOracle[i]["transNew"] = newTrans;

//		newTrans = newSum["trans"].toArray();
//		for (int i = agoTrans.count() - 1; i >= 0; --i) {
//			if (newTrans.contains(agoTrans.at(i))) {
//				agoTrans.removeAt(i);
//			}
//		}
//		effect.summaryPerOracle[i]["transGone"] = agoTrans;

		double iAvgValFlow = effect.dailyPerOracle[i];
		double iSlope = iAvgValFlow > 0 ? posSlope : negSlope;
		effect.dailyPerOracle[i] = iSlope * delta.dailyPerOracle[i];
		// try to make a fact
		QString fact = "";
		QString factStr = "";
		// negative effect from an outcome
		if (effect.dailyPerOracle[i] < -0.01) {
			if (iAvgValFlow < 0) {
				fact += "?-";
				factStr += "?Spending increased ";
			}
			// negative effect from an income
			if (dailyPerOracle[i] > 0) {
				fact += "?+";
				factStr += "?Income decreased ";
			}
		}
		// positive effect from an outcome
		else if (effect.dailyPerOracle[i] > 0.01) {
			if (dailyPerOracle[i] < 0) {
				fact += "!-";
				factStr += "!Spending decreased ";
			}
			// positive effect from an income
			if (dailyPerOracle[i] > 0) {
				fact += "!+";
				factStr += "!Income increased ";
			}
		}
		else
			continue;
		effect.summaryPerOracle[i]["factStr"] = factStr;
		effect.summaryPerOracle[i]["dailyOld"] = this->dailyPerOracle[i];
		effect.summaryPerOracle[i]["dailyNew"] = endSummary.dailyPerOracle[i];
		effect.summaryPerOracle[i]["dailyDif"] = delta.dailyPerOracle[i];
		effect.summaryPerOracle[i]["flowEffect"] = effect.dailyPerOracle[i];
		effect.summaryPerOracle[i]["overDays"] = overDays;
		effect.summaryPerOracle[i]["score"] = delta.dailyPerOracle[i] / overDays;
	}

	return effect;
}
