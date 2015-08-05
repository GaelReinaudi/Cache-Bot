#include "featurePeriodicAmount.h"

double FeatureMonthlyAmount::apply(TransactionBundle& allTrans, bool doLog)
{
	QDate lastDate = QDate::currentDate();
	QDate iniDate = lastDate.addMonths(-6);

	m_targetTrans = targetTransactions(iniDate, lastDate.addDays(BotContext::TARGET_TRANS_FUTUR_DAYS));
	if (m_targetTrans.count() == 0) {
		LOG() << "MonthlyAmount(0 TARGET): day "<<m_localStaticArgs.m_dayOfMonth<<" kla "<< m_localStaticArgs.m_kla << endl;
	}
	else if (doLog) {
		LOG() << getName().c_str() << " " << this << m_targetTrans.count()
			<<" TARGET: day " << m_localStaticArgs.m_dayOfMonth
			<<" kla"<< m_localStaticArgs.m_kla << "=" << m_targetTrans.first().amountDbl()
			<< " h=" << m_targetTrans.first().nameHash.hash()
			<< endl;
	}

	double totalOneOverDistClosest = 0.0;
	double totalOneOverDistOthers = 0.0;
	quint64 localDist = 18446744073709551615ULL;
	Transaction* localTrans = 0;
	// the current target to compare to
	Transaction* iTarg = &m_targetTrans[0];
	m_localStaticArgs.m_bundle.clear();

	m_localStaticArgs.m_consecMonth = 0;
	m_localStaticArgs.m_consecMonthBeforeMissed = 0;
	m_localStaticArgs.m_consecMissed = 0;
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
				m_localStaticArgs.m_bundle.append(localTrans);
				if (doLog) {
					iTarg->dist(*localTrans, true);
				}
				// isolate the transaction that were fitted to the target
				Q_ASSERT(localTrans->dimensionOfVoid == 0);
				localTrans->dimensionOfVoid++;
				iTarg->flags |= Transaction::CameTrue;
				if(m_localStaticArgs.m_consecMonth == 0) {
					m_localStaticArgs.m_consecMonthBeforeMissed = 0;
				}
				++m_localStaticArgs.m_consecMonthBeforeMissed;
				++m_localStaticArgs.m_consecMonth;
				m_localStaticArgs.m_consecMissed = 0;
				totalOneOverDistClosest += 64.0 / (64 + localDist);
			}
			else {
				if (doLog) {
					LOG() << "missed: ";
					iTarg->dist(*localTrans, true);
				}
				m_localStaticArgs.m_consecMonth = 0;
				++m_localStaticArgs.m_consecMissed;
				totalOneOverDistClosest += 1.0 / (1 + localDist);
			}

			if (iTarg == &m_targetTrans.last() || (iTarg + 1)->date >= lastDate)
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
	if (qAbs(m_localStaticArgs.m_bundle.sumDollar()) > 1) {
		m_fitness += totalOneOverDistClosest;
		m_fitness *= 1.0 * double(m_localStaticArgs.m_bundle.count() + m_localStaticArgs.m_bundle.count()) / double(m_targetTrans.count());
		m_fitness *= 1.0 + (1.0 / (1.0 + m_localStaticArgs.m_consecMissed));
	}
	m_billProba = billProbability();
	return m_fitness;
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

	output = apply(allTrans, ioContext.m_summaryJsonObj);

	// summary if the json object exists
	if (ioContext.m_summaryJsonObj) {
		if(m_billProba > 0.0) {
			QJsonArray features = (*ioContext.m_summaryJsonObj)["features"].toArray();
			features.append(toJson(ioContext));
			ioContext.m_summaryJsonObj->insert("features", features);
		}

		for (int i = 0; i < m_targetTrans.count(); ++i) {
			Transaction* iTarg = &m_targetTrans[i];
			emit ioContext.m_pUser->botContext()->matchedTransaction(iTarg->time_t(), iTarg->amountDbl());
		}
		for (int i = 0; i < m_localStaticArgs.m_bundle.count(); ++i) {
			Transaction& t = m_localStaticArgs.m_bundle.trans(i);
			emit ioContext.m_pUser->botContext()->matchedTransaction(t.time_t(), t.amountDbl(), 1);
		}
		OracleOneDayOfMonth* pNewOr = new OracleOneDayOfMonth();
		pNewOr->m_args = m_localStaticArgs;
		// making a shared pointer that will take care of cleaning once the oracle is no longer referenced
		QSharedPointer<Oracle> newOracle(pNewOr);
		ioContext.m_pUser->oracle()->addSubOracle(newOracle);
	}
}

QVector<Transaction> FeatureMonthlyAmount::BlankTransactionsForDayOfMonth(QDate iniDate, QDate lastDate, int dayOfMonth)
{
	static QVector<Transaction> targetTrans;
	targetTrans.clear();
	QDate currentDate = iniDate;
	while (currentDate < lastDate) {
		// the target day of this month. If neg, make it count from the end of the month.
		int targetDayThisMonth = qMin(dayOfMonth, currentDate.daysInMonth());
		targetDayThisMonth = qMax(targetDayThisMonth, 1 - currentDate.daysInMonth());
		while (targetDayThisMonth <= 0)
			targetDayThisMonth += currentDate.daysInMonth();
		// now we have a targetDay that is strictly in the month and

		// move current date to the correct target day that month
		currentDate = currentDate.addDays(targetDayThisMonth - currentDate.day());
		// asserts that it is 30ish days after the previous date we made
		assert(targetTrans.count() == 0 || qAbs(targetTrans.last().date.daysTo(currentDate) - 30) <= 2);

		targetTrans.append(Transaction());
		targetTrans.last().date = currentDate;

		currentDate = currentDate.addMonths(1);
	}

	return targetTrans;
}

QVector<Transaction> FeatureMonthlyAmount::targetTransactions(QDate iniDate, QDate lastDate)
{
	static QVector<Transaction> targetTrans;
	int dayOfMonth = m_localStaticArgs.m_dayOfMonth;
	targetTrans = BlankTransactionsForDayOfMonth(iniDate, lastDate, dayOfMonth);
	// loops through to set a few variables of the transactions
	for (Transaction& tr : targetTrans) {
		tr.setKLA(m_localStaticArgs.m_kla);
		tr.nameHash.setFromHash(m_localStaticArgs.m_b[0]);
		tr.flags |= Transaction::Predicted;
	}
	return targetTrans;
}

QVector<Transaction> FeatureBiWeeklyAmount::targetTransactions(QDate iniDate, QDate lastDate)
{
	static QVector<Transaction> targetTrans;
	targetTrans = BlankTransactionsForDayOfMonth(iniDate, lastDate, m_localStaticArgs.m_dayOfMonth);
	targetTrans += BlankTransactionsForDayOfMonth(iniDate, lastDate, m_localStaticArgs.m_dayOfMonth2);
	qSort(targetTrans.begin(), targetTrans.end(), Transaction::earlierThan);
	// loops through to set a few variables of the transactions
	for (Transaction& tr : targetTrans) {
		tr.setKLA(m_localStaticArgs.m_kla);
		tr.nameHash.setFromHash(m_localStaticArgs.m_b[0]);
		tr.flags |= Transaction::Predicted;
	}
	return targetTrans;
}

QVector<Transaction> OracleOneDayOfMonth::revelation(QDate upToDate)
{
	LOG() << "OracleOneDayOfMonth::revelation. bundle = " << m_args.m_bundle.count() << endl;
	QDate iniDate = QDate::currentDate();
	static QVector<Transaction> targetTrans;
	targetTrans = FeatureMonthlyAmount::BlankTransactionsForDayOfMonth(iniDate, upToDate, m_args.m_dayOfMonth);
	if (m_args.m_dayOfMonth2) {
		targetTrans += FeatureMonthlyAmount::BlankTransactionsForDayOfMonth(iniDate, upToDate, m_args.m_dayOfMonth2);
	}
	qSort(targetTrans.begin(), targetTrans.end(), Transaction::earlierThan);
	// loops through to set a few variables of the transactions
	for (Transaction& tr : targetTrans) {
		tr.setKLA(m_args.m_kla);
		tr.nameHash.setFromHash(m_args.m_b[0]);
		tr.flags |= Transaction::Predicted;
		LOG() << "m_args.m_dayOfMonth2 " << m_args.m_dayOfMonth2 << ""
			  << tr.amountDbl() << " " << tr.date.toString() << tr.name << endl;
	}
	return targetTrans;
}
