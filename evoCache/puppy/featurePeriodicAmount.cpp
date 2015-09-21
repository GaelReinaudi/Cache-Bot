#include "featurePeriodicAmount.h"

static const int SLACK_FOR_LATE_TRANS = 2;

void FeatureMonthlyAmount::getArgs(Puppy::Context &ioContext) {
	double a = 0;
	int ind = -1;
	getArgument(++ind, &a, ioContext);
	m_localStaticArgs.m_hash = a;
	getArgument(++ind, &a, ioContext);
	m_localStaticArgs.m_kla = a;
	getArgument(++ind, &a, ioContext);
	m_localStaticArgs.m_dayOfMonth = a;
}

double FeatureMonthlyAmount::apply(TransactionBundle& allTrans, bool doLog)
{
	QDate iniDate = Transaction::currentDay().addDays(-Transaction::maxDaysOld());
	QDate endDate = Transaction::currentDay().addDays(approxSpacingPayment() / 2);

	m_targetTrans = targetTransactions(iniDate, endDate);
	if (m_targetTrans.count() == 0) {
		WARN() << "MonthlyAmount(0 TARGET): day "<<m_localStaticArgs.m_dayOfMonth<<" kla "<< m_localStaticArgs.m_kla;
	}
	else if (doLog) {
		NOTICE() << getName().c_str() << " " << m_targetTrans.count()
			<<" TARGET: day " << m_localStaticArgs.m_dayOfMonth
			<<" kla"<< m_localStaticArgs.m_kla << "=" << m_targetTrans.first().amountDbl()
			<< " h=" << m_targetTrans.first().nameHash.hash();
	}

	double totalOneOverDistClosest = 0.0;
	double totalOneOverDistOthers = 0.0;
	quint64 localDist = 18446744073709551615ULL;
	const Transaction* localTrans = 0;
	// the current target to compare to
	Transaction* iTarg = &m_targetTrans[0];
	m_localStaticArgs.m_bundle.clear();

	m_localStaticArgs.m_consecMonth = 0;
	m_localStaticArgs.m_consecMonthBeforeMissed = 0;
	m_localStaticArgs.m_consecMissed = 0;
	for (int i = 0; i < allTrans.count(); ++i) {
		const Transaction& trans = allTrans.trans(i);
		quint64 dist = iTarg->dist(trans);
		if (trans.noUse()) {
			dist = 1<<20;
		}
		if (dist < localDist) {
			localDist = dist;
			localTrans = &trans;
		}
		Q_ASSERT(localDist < 18446744073709551615ULL);
		double factOld = 2.0;
		double daysAgo = localTrans->date.daysTo(QDate::currentDate());
		if (daysAgo > Transaction::maxDaysOld() / 2)
			factOld -= 1.0 * daysAgo / double(Transaction::maxDaysOld() / 2);
		// if we get further away by approxSpacingPayment() / 2 days, we take the next target, or if last trans
		if (trans.jDay() > approxSpacingPayment() / 2 + iTarg->jDay() || i == allTrans.count() - 1) {
			if (localDist < Transaction::LIMIT_DIST_TRANS) {
				m_localStaticArgs.m_bundle.append(localTrans);
				if (doLog) {
					iTarg->dist(*localTrans, true);
				}
//				// isolate the transaction that were fitted to the target
//				Q_ASSERT(localTrans->dimensionOfVoid == 0);
//				localTrans->dimensionOfVoid++;
				iTarg->flags |= Transaction::CameTrue;
				if(m_localStaticArgs.m_consecMonth == 0) {
					m_localStaticArgs.m_consecMonthBeforeMissed = 0;
				}
				++m_localStaticArgs.m_consecMonthBeforeMissed;
				++m_localStaticArgs.m_consecMonth;
				m_localStaticArgs.m_consecMissed = 0;
				// if transaction is in advance
				if (iTarg->date >= Transaction::currentDay().addDays(-SLACK_FOR_LATE_TRANS))
					m_localStaticArgs.m_consecMissed = -1;
				double transFit = 8.0 / (8 + localDist);
				totalOneOverDistClosest += factOld * transFit;
			}
			else if (iTarg->date < Transaction::currentDay().addDays(-SLACK_FOR_LATE_TRANS)){
				if (doLog) {
					DBG() << "missed: ";
					iTarg->dist(*localTrans, true);
				}
				m_localStaticArgs.m_consecMonth = 0;
				++m_localStaticArgs.m_consecMissed;
				double transFit = 1.0 / (1 + localDist);
				totalOneOverDistClosest += factOld * transFit;
			}

			if (iTarg == &m_targetTrans.last() || (iTarg + 1)->date >= endDate)
				break;
			++iTarg;
			// keep this last trans in the pool if it was not just added
			if (&trans != localTrans)
				--i;
			localTrans = 0;
			localDist = quint64(-1);
		}
		totalOneOverDistOthers += factOld * 1.0 / (1 + dist);
	}
	totalOneOverDistOthers -= totalOneOverDistClosest;
	double tempFitness = 0.0;
	// only sum that add up to > $N
	if (qAbs(m_localStaticArgs.m_bundle.sumDollar()) > 1) {
		tempFitness = totalOneOverDistClosest - totalOneOverDistOthers;
		tempFitness *= 1.75 * (double(m_localStaticArgs.m_consecMonthBeforeMissed) - 1.5);
	}
	return tempFitness;
}

void FeatureMonthlyAmount::onJustApplied(TransactionBundle& allTrans, bool doLog = false)
{
	// tries to re-run this periodic and if it has a high vlaue, it is a sign that
	// it is actually more frequent and should have a bad grade
	auto temp = m_localStaticArgs;
	auto tempTarg = m_targetTrans;
	m_localStaticArgs.m_dayOfMonth += approxSpacingPayment() / 2;
	m_localStaticArgs.m_dayOfMonth %= 31;
	cleanArgs();
	double rerun = apply(allTrans, false);
	if (doLog) {
		INFO() << "fitness " << m_fitness << "- 2x " << rerun;
	}
	// restore member variables
	m_localStaticArgs = temp;
	m_targetTrans = tempTarg;
	m_localStaticArgs.m_fitRerun = rerun;
	cleanArgs();

	// recompute fitness
	m_fitness -= 2 * qMax(0.0, rerun);
//	m_fitness *= 2.0;
	if (qAbs(m_localStaticArgs.m_kla) > 2)
		m_fitness *= 5.0;
}

void FeatureMonthlyAmount::emitGraphics(Puppy::Context& ioContext) const
{
	for (int i = 0; i < m_targetTrans.count(); ++i) {
		const Transaction* iTarg = &m_targetTrans.at(i);
		Q_ASSERT(iTarg->time_t() > -1e9 && iTarg->time_t() < 10e9 && iTarg->amountDbl() > -1e9 && iTarg->amountDbl() < 1e9);
		emit ioContext.m_pUser->botContext()->matchedTransaction(iTarg->time_t(), iTarg->amountDbl());
	}
	for (int i = 0; i < m_localStaticArgs.m_bundle.count(); ++i) {
		const Transaction& tr = m_localStaticArgs.m_bundle.trans(i);
		emit ioContext.m_pUser->botContext()->matchedTransaction(tr.time_t(), tr.amountDbl(), 1);
	}
}

QVector<Transaction> FeatureMonthlyAmount::BlankTransactionsForDayOfMonth(QDate iniDate, QDate lastDate, int dayOfMonth, std::function<Transaction(void)> lambda)
{
	static QVector<Transaction> targetTrans;
	targetTrans.clear();
	QDate currentDate = iniDate;
	while (currentDate <= lastDate) {
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

		if (currentDate <= lastDate && currentDate >= iniDate) {
			targetTrans.append(lambda());
			targetTrans.last().date = currentDate;
		}
		currentDate = currentDate.addMonths(1);
	}

	return targetTrans;
}

QVector<Transaction> FeatureMonthlyAmount::targetTransactions(QDate iniDate, QDate lastDate)
{
	static QVector<Transaction> targetTrans;
	targetTrans = BlankTransactionsForDayOfMonth(iniDate, lastDate, m_localStaticArgs.m_dayOfMonth);
	// loops through to set a few variables of the transactions
	for (Transaction& tr : targetTrans) {
		tr.setKLA(m_localStaticArgs.m_kla);
		tr.nameHash.setFromHash(m_localStaticArgs.m_hash);
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
		tr.nameHash.setFromHash(m_localStaticArgs.m_hash);
		tr.flags |= Transaction::Predicted;
	}
	return targetTrans;
}

QVector<Transaction> OracleOneDayOfMonth::revelation(QDate upToDate)
{
	auto lambdaTrans = [this](){
		Transaction tr = m_args.m_bundle.randSmart();
		tr.flags |= Transaction::Predicted;
		INFO() << QString("dayOfMonth %1/%2 ").arg(m_args.m_dayOfMonth).arg(m_args.m_dayOfMonth2)
			  << tr.amountDbl() << " " << tr.date.toString() << " " << tr.name;
		return tr;
	};
	DBG() << "OracleOneDayOfMonth::revelation. bundle = " << m_args.m_bundle.count();
	QDate iniDate = Transaction::currentDay().addDays(-SLACK_FOR_LATE_TRANS);
	static QVector<Transaction> targetTrans;
	targetTrans.clear();
	if (m_args.m_consecMissed <= 0)
	{
		targetTrans = FeatureMonthlyAmount::BlankTransactionsForDayOfMonth(iniDate, upToDate, m_args.m_dayOfMonth, lambdaTrans);
		if (m_args.m_dayOfMonth2) {
			targetTrans += FeatureMonthlyAmount::BlankTransactionsForDayOfMonth(iniDate, upToDate, m_args.m_dayOfMonth2, lambdaTrans);
		}
		qSort(targetTrans.begin(), targetTrans.end(), Transaction::earlierThan);
		// if we have the first transaction, don't predict ityy
		if (m_args.m_consecMissed < 0) {
			targetTrans.pop_front();
		}
	}
	return targetTrans;
}

double OracleOneDayOfMonth::avgDaily() const
{
	double avgMonth = 0.0;
	if (m_args.m_consecMissed <= 0)
	{
		avgMonth = m_args.m_bundle.avgSmart();
		if (m_args.m_dayOfMonth2) {
			avgMonth *= 2;
		}
	}
	return avgMonth * (12.0 / 365.25);
}
