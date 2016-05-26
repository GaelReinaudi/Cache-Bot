#include "oracle.h"
#include "AccRegPrimits.h"

QJsonArray revSortedJsonArray(const QString& varName, QList<QJsonObject>& list)
{
	QJsonArray ret;
	if (list.count()) {
		std::sort(list.begin(), list.end(), [varName](const QJsonObject& a, const QJsonObject& b){
			return qAbs(a[varName].toDouble()) > qAbs(b[varName].toDouble());
		});
	}
	for (const auto& obj : list) {
		ret.append(obj);
	}
	return ret;
}

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

	if (includeOracleSummaries) {
	// week summary
	int daysToSunday = 7 - Transaction::currentDay().dayOfWeek();
	if (daysToSunday == 0)
		daysToSunday += 7;
	summary.weekDetails["daysToSunday"] = daysToSunday;
	double dailyFrequent = 0.0;
	double dailyInfrequent = 0.0;
	double dailyNotSureBill = 0.0;
	double totBills = 0.0;
	QJsonArray weekBills;
	double threshAmountFrequent = 0.0;

	static const double perWeekTresh = 1.5;
	for (QSharedPointer<Oracle> pOr : m_subOracles) {
		double daily = pOr->avgDaily();
		if (daily >= 0)
			continue;
		QJsonObject orj = pOr->toJson();
		double avgAmount = orj["avgAmnt"].toDouble();
		if (pOr->feature()->isPeriodic()) {
			double fitness = orj["fitness"].toDouble();
			double consec = orj["consecutive"].toDouble();
			QDate nextDate = QDate::fromString(orj["nextDate"].toString(), "yyyy-MM-dd");
			int inD = Transaction::currentDay().daysTo(nextDate);
			if (inD >= 0 && inD <= daysToSunday) {
				if (fitness >= 15 and consec > 2) {
					Transaction t = pOr->args()->m_bundle.randSmart();
					QJsonObject bill;
					bill["id"] = t.id;
					bill["name"] = t.name;
					bill["amount"] = t.amountDbl();
					bill["date"] = orj["nextDate"];
					bill["fitness"] = fitness;
					bill["consec"] = consec;
					weekBills.append(bill);
					totBills += avgAmount;
				}
				else {
					dailyNotSureBill += daily;
				}
				if (pOr->args()->m_bundle.flagsOR() & Transaction::UserInputFlag::yesRecur) {
				}
				else {
					dailyInfrequent += daily;
				}
			}
			continue;
		}
		double dayProba = orj["dayProba"].toDouble();
		if (!dayProba) {
			ERR() << "dayProba == 0";
		}
		double perDay = dayProba / (1 - dayProba);
		if (perDay * 7 > perWeekTresh) {
			threshAmountFrequent = qMin (threshAmountFrequent, avgAmount);
			dailyFrequent += daily;
		}
		else {
			dailyInfrequent += daily;
		}
	}
	summary.weekDetails["weekBills"] = weekBills;
	summary.weekDetails["_1stPass_dailyFrequent"] = dailyFrequent;
	summary.weekDetails["_1stPass_dailyInfrequent"] = dailyInfrequent;
	summary.weekDetails["_threshAmountFrequent"] = threshAmountFrequent;
	summary.weekDetails["approxThreshAmountFrequent"] = toSignifDigit_2(threshAmountFrequent);
	QList<QJsonObject> yesRecurList;
	QList<QJsonObject> frequentList;
	QList<QJsonObject> infrequentList;
	double totYesRecur = 0.0;
	double totNotYesRecur = 0.0;
	dailyFrequent = 0.0;
	dailyInfrequent = 0.0;
	int index = -1;
	for (QSharedPointer<Oracle> pOr : m_subOracles) {
		++index;
		double daily = pOr->avgDaily();
		if (daily >= 0)
			continue;
		QJsonObject orj = pOr->toJson();
		double avgAmount = orj["avgAmnt"].toDouble();
		if (pOr->feature()->isPeriodic()) {
			QDate nextDate = QDate::fromString(orj["nextDate"].toString(), "yyyy-MM-dd");
			int inD = Transaction::currentDay().daysTo(nextDate);
			if (inD >= 0 && inD <= daysToSunday) {
				QJsonObject perij;
				perij["daily"] = daily;
				perij["_tot"] = avgAmount;
				perij["inD"] = inD;
				perij["descr"] = pOr->description();
				perij["indOracle"] = index;
				perij["nextDate"] = nextDate.toString();
				perij["oracleJson"] = orj;
				if (pOr->args()->m_bundle.flagsOR() & Transaction::UserInputFlag::yesRecur) {
//					totYesRecur += avgAmount;
					yesRecurList.append(perij);
				}
				else {
//					totNotYesRecur += avgAmount;
//					dailyInfrequent += daily;
					infrequentList.append(perij);
				}
			}
			continue;
		}
		if (true) {
			QJsonObject orj = pOr->toJson();
			double dayProba = orj["dayProba"].toDouble();
			double perDay = dayProba / (1 - dayProba);
			QJsonObject perij;
			perij["daily"] = daily;
			perij["_tot"] = daily * (daysToSunday);
			perij["descr"] = pOr->description();
			perij["indOracle"] = index;
			perij["dayProba"] = dayProba;
			perij["oracleJson"] = orj;
			if (perDay * 7 > perWeekTresh || qAbs(avgAmount) < qAbs(threshAmountFrequent)) {
				dailyFrequent += daily;
				frequentList.append(perij);
			}
			else {
				dailyInfrequent += daily;
				infrequentList.append(perij);
			}
		}
	}

//	++daysToSunday;

	summary.weekDetails["yesRecurList"] = revSortedJsonArray("_tot", yesRecurList);
	summary.weekDetails["frequentList"] = revSortedJsonArray("_tot", frequentList);
	summary.weekDetails["infrequentList"] = revSortedJsonArray("_tot", infrequentList);
//	summary.weekDetails["negSumAll"] = summary.negSum;
//	summary.weekDetails["negSumOthers"] = negSumOthers;

//	summary.weekDetails["_totYesRecur"] = totYesRecur;
//	summary.weekDetails["_totNotYesRecur"] = totNotYesRecur;
	summary.weekDetails["_dailyFrequent"] = dailyFrequent;
	summary.weekDetails["_dailyInfrequent"] = dailyInfrequent;
	summary.weekDetails["_dailyNotSureBill"] = dailyNotSureBill;
	summary.weekDetails["_totBills"] = totBills;
	summary.weekDetails["_totFrequent"] = dailyFrequent * daysToSunday;
	summary.weekDetails["_totInfrequent"] = (dailyInfrequent + dailyNotSureBill) * daysToSunday;
	summary.weekDetails["approxTotYesRecur"] = toSignifDigit_2(totYesRecur);
	summary.weekDetails["approxTotNotYesRecur"] = toSignifDigit_2(totNotYesRecur);
	summary.weekDetails["approxDailyFrequent"] = toSignifDigit_2(dailyFrequent);
	summary.weekDetails["approxDailyInfrequent"] = toSignifDigit_2(dailyInfrequent);
	summary.weekDetails["approxTotInfrequent"] = toSignifDigit_2((dailyInfrequent + dailyNotSureBill) * daysToSunday);
	summary.weekDetails["approxTotBill"] = toSignifDigit_2(totBills);
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
