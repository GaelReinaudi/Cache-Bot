#include "oracle.h"
#include "AccRegPrimits.h"

QVector<Transaction> SuperOracle::revelation(QDate upToDate)
{
	DBG(2) << "SuperOracle::revelation from " << m_subOracles.count();
	QVector<Transaction> ret;
	// naive adding up the revelations for now
	for (auto pOr : m_subOracles) {
		if (pOr->avgDaily(60))
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

double SuperOracle::avgDaily(int limDayProba) const
{
	NOTICE() << "SuperOracle::avgDaily";
	double avg = 0.0;
	for (auto pOr : m_subOracles) {
		avg += pOr->avgDaily(limDayProba);
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
		double avg45 = pOr->avgDaily(45);
		double avg60 = pOr->avgDaily(60);
		double avg90 = pOr->avgDaily(90);
		if (avg > 0) {
			summary.posSum += avg;
			summary.posSum45 += avg45;
			summary.posSum60 += avg60;
			summary.posSum90 += avg90;
			if (pOr->feature()->isPeriodic())
				summary.salary += avg;
			DBG(2) << ind <<  " +daily " << avg << "      " << pOr->feature()->getName();
		}
		else if (avg < 0) {
			summary.negSum += avg;
			summary.negSum45 += avg45;
			summary.negSum60 += avg60;
			summary.negSum90 += avg90;
			if (pOr->feature()->isPeriodic())
				summary.bill += avg;
			DBG(2) << ind <<  " -daily " << avg << "      " << pOr->feature()->getName();
		}
		summary.dailyPerOracle.append(avg);
		if (includeOracleSummaries)
			summary.summaryPerOracle.append(pOr->toJson());
	}

	// week summary
	int daysToSunday = 7 - Transaction::currentDay().dayOfWeek();
	if (daysToSunday == 0)
		daysToSunday += 7;
	summary.weekDetails["daysToSunday"] = daysToSunday;
	QJsonArray largePeriodic;
	QJsonArray probables;
	double negSumOthers = 0.0;
	int index = -1;
	for (QSharedPointer<Oracle> pOr : m_subOracles) {
		++index;
		double avg = pOr->avgDaily();
		if (avg >= 0)
			continue;
		if (pOr->feature()->isPeriodic()) {
			if (qAbs(avg) < 0.04 * qAbs(summary.negSum)) {
				negSumOthers += avg;
				continue;
			}
			QJsonObject orj = pOr->toJson();
			QDate nextDate = QDate::fromString(orj["nextDate"].toString(), "yyyy-MM-dd");
			int inD = Transaction::currentDay().daysTo(nextDate);
			if (inD > 0 && inD <= daysToSunday) {
				QJsonObject perij;
				perij["avg"] = avg;
				perij["inD"] = inD;
				perij["descr"] = pOr->description();
				perij["indOracle"] = index;
				perij["nextDate"] = nextDate.toString();
				perij["oracleJson"] = orj;
				largePeriodic.append(perij);
			}
		}
		if (true) {
			if (qAbs(avg) < 0.04 * qAbs(summary.negSum)) {
				negSumOthers += avg;
				continue;
			}
			QJsonObject orj = pOr->toJson();
			double dayOccur = orj["dayOccur"].toDouble();
			if (dayOccur * daysToSunday > 1.5) {
				QJsonObject perij;
				perij["avg"] = avg;
				perij["descr"] = pOr->description();
				perij["indOracle"] = index;
				perij["oracleJson"] = orj;
				probables.append(perij);
			}
		}
		else {
			negSumOthers += avg;
			continue;
		}
	}
	summary.weekDetails["largePeriodic"] = largePeriodic;
	summary.weekDetails["probables"] = probables;
	summary.weekDetails["negSumAll"] = summary.negSum;
	summary.weekDetails["negSumOthers"] = negSumOthers;

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
//		else
//			continue;
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
