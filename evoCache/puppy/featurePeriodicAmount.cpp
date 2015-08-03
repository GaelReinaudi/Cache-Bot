#include "featurePeriodicAmount.h"

QVector<Transaction> FeatureMonthlyAmount::targetTransactions(QDate iniDate, QDate lastDate)
{
	QVector<Transaction> targetTrans;
	QDate currentDate = iniDate;
	while (currentDate < lastDate) {
		// the target day of this month. If neg, make it count from the end of the month.
		int targetDayThisMonth = qMin(m_dayOfMonth, currentDate.daysInMonth());
		targetDayThisMonth = qMax(targetDayThisMonth, 1 - currentDate.daysInMonth());
		while (targetDayThisMonth <= 0)
			targetDayThisMonth += currentDate.daysInMonth();

		// move current date to the correct target day that month
		currentDate = currentDate.addDays(targetDayThisMonth - currentDate.day());
		// assert that it is 30ish days after the previous date
		assert(targetTrans.count() == 0 || qAbs(targetTrans.last().date.daysTo(currentDate) - 30) <= 2);

		targetTrans.append(Transaction());
		targetTrans.last().date = currentDate;
		targetTrans.last().setKLA(m_kla);
//		targetTrans.last().indexHash = 0;
		targetTrans.last().nameHash.setFromHash(m_b[0]);
		targetTrans.last().flags |= Transaction::Predicted;

		currentDate = currentDate.addMonths(1);
	}

	return targetTrans;
}

void FeatureMonthlyAmount::execute(void *outDatum, Puppy::Context &ioContext)
{
	AccountFeature::execute(outDatum, ioContext);
	double& output = *(double*)outDatum;

	getArgs(ioContext);
	cleanArgs();

	m_fitness = 0.0;

	// will be ALL the transactions if m_filterHash < 0
	auto& allTrans = ioContext.m_pUser->transBundle(m_filterHash);
	QDate lastDate = QDate::currentDate();
	QDate iniDate = lastDate.addMonths(-6);

	QVector<Transaction> targetTrans = targetTransactions(iniDate, lastDate.addDays(BotContext::TARGET_TRANS_FUTUR_DAYS));
	if (targetTrans.count() == 0) {
		LOG() << "MonthlyAmount(0 TARGET): day "<<m_dayOfMonth<<" kla "<< m_kla << endl;
	}
	else if (ioContext.m_summaryJsonObj) {
		LOG() << getName().c_str() << " " << this << targetTrans.count()
			<<" TARGET: day "<<m_dayOfMonth
			<<" kla"<< m_kla <<"="<<targetTrans.first().amountDbl()
			<< " h=" <<targetTrans.first().nameHash.hash()
			<< endl;
	}

	double totalOneOverDistClosest = 0.0;
	double totalOneOverDistOthers = 0.0;
	quint64 localDist = 18446744073709551615ULL;
	Transaction* localTrans = 0;
	// the current target to compare to
	Transaction* iTarg = &targetTrans[0];
	m_bundle.clear();

	m_consecMonth = 0;
	m_consecMonthBeforeMissed = 0;
	m_consecMissed = 0;
	for (int i = 0; i < allTrans.count(); ++i) {
		Transaction& trans = allTrans.trans(i);
		quint64 dist = iTarg->dist(trans);
		if (dist < localDist) {
			localDist = dist;
			localTrans = &trans;
		}
		Q_ASSERT(localDist < 18446744073709551615ULL);
		// if we get further away by approxSpacingPayment() / 2 days, we take the next target, or if last trans
		if (trans.jDay() > approxSpacingPayment() / 2 + iTarg->jDay() || i == allTrans.count() - 1) {
			if (localDist < Transaction::LIMIT_DIST_TRANS) {
				m_bundle.append(localTrans);
				if (ioContext.m_summaryJsonObj) {
					iTarg->dist(*localTrans, true);
				}
				// isolate the transaction that were fitted to the target
				Q_ASSERT(localTrans->dimensionOfVoid == 0);
				localTrans->dimensionOfVoid++;
				iTarg->flags |= Transaction::CameTrue;
				if(m_consecMonth == 0) {
					m_consecMonthBeforeMissed = 0;
				}
				++m_consecMonthBeforeMissed;
				++m_consecMonth;
				m_consecMissed = 0;
				totalOneOverDistClosest += 64.0 / (64 + localDist);
			}
			else {
				if (ioContext.m_summaryJsonObj) {
					LOG() << "missed: ";
					iTarg->dist(*localTrans, true);
				}
				m_consecMonth = 0;
				++m_consecMissed;
				totalOneOverDistClosest += 1.0 / (1 + localDist);
			}

			if (iTarg == &targetTrans.last() || (iTarg + 1)->date >= lastDate)
				break;
			++iTarg;
			localTrans = 0;
			localDist = quint64(-1);
			// keep this last trans in the pool if it was not just added
			if (&trans != localTrans)
				--i;
		}
		totalOneOverDistOthers += 1.0 / (1 + dist);
	}
	totalOneOverDistOthers -= totalOneOverDistClosest;
	// only sum that add up to > $N
	if (qAbs(m_bundle.sumDollar()) > 1) {
		m_fitness += totalOneOverDistClosest;
		m_fitness *= 1.0 * double(m_bundle.count() + m_bundle.count()) / double(targetTrans.count());
		m_fitness *= 1.0 + (1.0 / (1.0 + m_consecMissed));
	}
	m_billProba = billProbability();
	output = m_fitness;

	// summary if the json object exists
	if (ioContext.m_summaryJsonObj) {
		if(ioContext.m_mapPredicted) {
			(*(ioContext.m_mapPredicted))[m_billProba] += targetTrans;
			for (auto& t : targetTrans) {
				Q_UNUSED(t);
				Q_ASSERT(t.flags & Transaction::Predicted);
			}
		}

		if(m_billProba > 0.0) {
			QJsonArray features = (*ioContext.m_summaryJsonObj)["features"].toArray();
			features.append(toJson(ioContext));
			ioContext.m_summaryJsonObj->insert("features", features);
		}

		for (int i = 0; i < targetTrans.count(); ++i) {
			Transaction* iTarg = &targetTrans[i];
			emit ioContext.m_pUser->botContext()->matchedTransaction(iTarg->time_t(), iTarg->amountDbl());
		}
		for (int i = 0; i < m_bundle.count(); ++i) {
			Transaction& t = m_bundle.trans(i);
			emit ioContext.m_pUser->botContext()->matchedTransaction(t.time_t(), t.amountDbl(), 1);
		}
		//qDebug() << targetTrans.count() << m_bundle.count();
		ioContext.m_pUser->oracle()->addSubOracle(this);
	}
}

QVector<Transaction> FeatureBiWeeklyAmount::targetTransactions(QDate iniDate, QDate lastDate)
{
	QVector<Transaction> targetTrans;
	QDate currentDate = iniDate;
	while (currentDate < lastDate) {
		// the target day of this month. If neg, make it count from the end of the month.
		int targetDayThisMonth = qMin(m_dayOfMonth2, currentDate.daysInMonth());
		targetDayThisMonth = qMax(targetDayThisMonth, 1 - currentDate.daysInMonth());
		while (targetDayThisMonth <= 0)
			targetDayThisMonth += currentDate.daysInMonth();

		// move current date to the correct target day that month
		currentDate = currentDate.addDays(targetDayThisMonth - currentDate.day());
		// assert that it is 30ish days after the previous date
		assert(targetTrans.count() == 0 || qAbs(targetTrans.last().date.daysTo(currentDate) - 30) <= 2);

		targetTrans.append(Transaction());
		targetTrans.last().date = currentDate;
		targetTrans.last().setKLA(m_kla);
//		targetTrans.last().indexHash = 0;
		targetTrans.last().nameHash.setFromHash(m_b[0]);
		targetTrans.last().flags |= Transaction::Predicted;

		currentDate = currentDate.addMonths(1);
	}

	targetTrans += (FeatureMonthlyAmount::targetTransactions(iniDate, lastDate));
	qSort(targetTrans.begin(), targetTrans.end(), Transaction::earlierThan);
	return targetTrans;
}
