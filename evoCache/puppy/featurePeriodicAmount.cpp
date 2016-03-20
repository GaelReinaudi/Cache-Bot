#include "featurePeriodicAmount.h"

static const int SLACK_FOR_LATE_TRANS = 4;

QJsonObject OracleOneDayOfMonth::toJson() const {
	QJsonObject ret = Oracle::toJson();
	ret["approxAmnt"] = toSignifDigit_2(m_args.m_bundle.avgSmart());
	ret["avgAmnt"] = m_args.m_bundle.avgSmart();
	ret["day1"] = (m_args.m_dayOfMonth + 31) % 31;
	ret["day2"] = (m_args.m_dayOfMonth2 + 31) % 31;
	ret["daily"] = (m_args.computeProba() <= 0.0)
				   ? 0.0
				   : m_args.m_bundle.avgSmart() / (365.25 / (12.0 * (m_args.m_dayOfMonth2 != 0 ? 2.0 : 1.0)));
	ret["consMissed"] = m_args.m_consecMissed;
	double fracCat = 0.0;
	ret.insert("mostCatId", m_args.m_bundle.mostCatId(&fracCat));
	ret.insert("mostCatFrac", fracCat);

	QJsonArray transIds;
	for (int i = 0; i < m_args.m_bundle.count(); ++i) {
		const Transaction& tr = m_args.m_bundle.trans(i);
		transIds.append(tr.id);
	}
	ret["trans"] = transIds;

	return ret;
}

void FeatureMonthlyAmount::getArgs(Puppy::Context &ioContext) {
	double a = 0;
	int ind = -1;
	getArgument(++ind, &a, ioContext);
	m_localStaticArgs.m_hash = a;
	getArgument(++ind, &a, ioContext);
	m_localStaticArgs.m_kla = a;
	getArgument(++ind, &a, ioContext);
	a = qBound(-1000.0, a, 1000.0);
	m_localStaticArgs.m_dayOfMonth = a;
}

double FeatureMonthlyAmount::apply(TransactionBundle& allTrans, bool isPostTreat, bool doLog)
{
	QDate iniDate = Transaction::currentDay().addDays(-Transaction::maxDaysOld());
	QDate endDate = Transaction::currentDay().addDays(approxSpacingPayment() / 2);

	m_targetTrans = targetTransactions(iniDate, endDate);
	if (m_targetTrans.count() == 0) {
		WARN() << "MonthlyAmount(0 TARGET): day "<<m_localStaticArgs.m_dayOfMonth<<" kla "<< m_localStaticArgs.m_kla;
	}
	else if (doLog) {
//		NOTICE() << getName().c_str() << " " << m_targetTrans.count()
//			<<" TARGET: day " << m_localStaticArgs.m_dayOfMonth
//			<<" kla"<< m_localStaticArgs.m_kla << "=" << m_targetTrans.first().amountDbl()
//			<< " h=" << m_targetTrans.first().nameHash.hash();
	}

	double totalOneOverDistClosest = 0.0;
	double totalOneOverDistOthers = 0.0;
	qint64 localDist = Q_INT64_C(9223372036854775807);
	const Transaction* localTrans = 0;
	// the current target to compare to
	Transaction* iTarg = &m_targetTrans[0];
	m_localStaticArgs.m_bundle.clear();

	m_localStaticArgs.m_consecMonth = 0;
	m_localStaticArgs.m_consecMonthBeforeMissed = 0;
	m_localStaticArgs.m_consecMissed = 999;
	m_localStaticArgs.m_prevMissed = 0;
	if (m_localStaticArgs.m_dayOfMonth < -13 || m_localStaticArgs.m_dayOfMonth > 31)
		return 0.0;
	for (int i = 0; i < allTrans.count(); ++i) {
		const Transaction& trans = allTrans.trans(i);
		qint64 dist = distance(iTarg, &trans);
		if (trans.noUse() || (trans.userFlag & Transaction::NoRecur)) {
			dist = 1<<20;
		}
		double factOld = 2.0;
		if (dist < localDist && trans.jDay() <= approxSpacingPayment() / 3 + iTarg->jDay()) {
			localDist = dist;
			localTrans = &trans;
			double daysAgo = localTrans->date.daysTo(Transaction::currentDay());
			if (daysAgo > Transaction::maxDaysOld() / 2)
				factOld -= 1.0 * daysAgo / double(Transaction::maxDaysOld() / 2);
		}
//		Q_ASSERT(localDist < 18446744073709551615ULL);
		// if we get further away by approxSpacingPayment() / 2 days, we take the next target, or if last trans
		if (trans.jDay() > approxSpacingPayment() / 2 + iTarg->jDay() || i == allTrans.count() - 1) {
			if (localDist < Transaction::LIMIT_DIST_TRANS && localTrans) {
				m_localStaticArgs.m_bundle.append(localTrans);
				if (doLog) {
					iTarg->dist(*localTrans, true);
				}
//				// isolate the transaction that were fitted to the target
//				Q_ASSERT(localTrans->isVoid() == 0);
//				localTrans->isVoid()++;
				iTarg->flags |= Transaction::CameTrue;
				if(m_localStaticArgs.m_consecMonth == 0) {
					m_localStaticArgs.m_consecMonthBeforeMissed = 0;
				}
				++m_localStaticArgs.m_consecMonthBeforeMissed;
				++m_localStaticArgs.m_consecMonth;
				m_localStaticArgs.m_prevMissed = m_localStaticArgs.m_consecMissed;
				m_localStaticArgs.m_consecMissed = 0;
				// if transaction is in advance
				if (localTrans->date == Transaction::currentDay())
					m_localStaticArgs.m_consecMissed = -1;
				if (localTrans->date > Transaction::currentDay())
					m_localStaticArgs.m_consecMissed = -2;
				double transFit = 8.0 / (8 + localDist);
				totalOneOverDistClosest += factOld * transFit;
			}
			else if (iTarg->date < Transaction::currentDay().addDays(-SLACK_FOR_LATE_TRANS)){
				if (doLog) {
					DBG() << "missed: ";
					if (localTrans)
						iTarg->dist(*localTrans, true);
				}
				m_localStaticArgs.m_consecMonth = 0;
				++m_localStaticArgs.m_consecMissed;
				double transFit = 1.0 / (1 + localDist);
				if (localTrans)
					totalOneOverDistClosest += factOld * transFit;
			}

			if (iTarg == &m_targetTrans.last() || (iTarg + 1)->date >= endDate)
				break;
			++iTarg;
			// keep this last trans in the pool if it was not just added
			if (&trans != localTrans)
				--i;
			localTrans = 0;
			localDist = Q_INT64_C(9223372036854775807);
		}
		totalOneOverDistOthers += factOld * 1.0 / (1 + dist);
	}
	totalOneOverDistOthers -= totalOneOverDistClosest;
	double tempFitness = 0.0;
	// only sum that add up to > $N
	if (qAbs(m_localStaticArgs.m_bundle.sumDollar()) > 1) {
		tempFitness = totalOneOverDistClosest - totalOneOverDistOthers;
	}
	return tempFitness;
}

void FeatureMonthlyAmount::onJustApplied(TransactionBundle& allTrans, Puppy::Context &ioContext)
{
	if (m_fitness <= 0.0)
		return;
	// tries to re-run this periodic and if it has a high vlaue, it is a sign that
	// it is actually more frequent and should have a bad grade
	auto temp = m_localStaticArgs;
	auto tempTarg = m_targetTrans;
	m_localStaticArgs.m_dayOfMonth += approxSpacingPayment() / 2;
	m_localStaticArgs.m_dayOfMonth %= 31;
	cleanArgs();
	double rerun = apply(allTrans, false, false);
	if (ioContext.m_summaryJsonObj) {
		DBG() << "fitness " << m_fitness << "- 2x " << rerun;
	}
	// restore member variables
	m_localStaticArgs = temp;
	m_targetTrans = tempTarg;
	m_localStaticArgs.m_fitRerun = rerun;
	cleanArgs();

	if (m_localStaticArgs.m_prevMissed >= /*-1+*/m_localStaticArgs.m_consecMonthBeforeMissed) {
		m_fitness = 0.0;
		return;
	}
	// recompute fitness
	m_fitness -= 3 * qMax(0.0, rerun);
	m_fitness *= 1.75 * (double(m_localStaticArgs.m_consecMonthBeforeMissed) - 1.5);
	if (m_localStaticArgs.m_consecMissed == 0 && m_localStaticArgs.m_consecMonthBeforeMissed == 2)
		m_fitness *= 5;
//	m_fitness *= 2.0;
	if (qAbs(m_localStaticArgs.m_kla) > 2)
		m_fitness *= qAbs(m_localStaticArgs.m_bundle.klaAverage());
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
		DBG() << QString("dayOfMonth %1/%2 ").arg(m_args.m_dayOfMonth).arg(m_args.m_dayOfMonth2)
			  << tr.amountDbl() << " " << tr.date.toString() << " " << tr.name;
		return tr;
	};
	DBG() << "OracleOneDayOfMonth::revelation. bundle = " << m_args.m_bundle.count();
	QDate iniDate = Transaction::currentDay().addDays(-SLACK_FOR_LATE_TRANS);
	static QVector<Transaction> targetTrans;
	targetTrans.clear();
	if (m_args.computeProba() > 0.0)
	{
		targetTrans = FeatureMonthlyAmount::BlankTransactionsForDayOfMonth(iniDate, upToDate, m_args.m_dayOfMonth, lambdaTrans);
		if (m_args.m_dayOfMonth2) {
			targetTrans += FeatureMonthlyAmount::BlankTransactionsForDayOfMonth(iniDate, upToDate, m_args.m_dayOfMonth2, lambdaTrans);
		}

		qSort(targetTrans.begin(), targetTrans.end(), Transaction::earlierThan);
		// if we have the first transaction, don't predict it
		if (m_args.m_consecMissed == -1) {
			targetTrans.pop_front();
		}
		if (m_args.m_consecMissed == 0 && targetTrans.begin()->date <= Transaction::currentDay()) {
			targetTrans.pop_front();
		}
	}
	return targetTrans;
}

double OracleOneDayOfMonth::avgDaily() const
{
	double avgMonth = 0.0;
	if (m_args.computeProba() > 0.0)
	{
		avgMonth = m_args.m_bundle.avgSmart();
		if (m_args.m_dayOfMonth2) {
			avgMonth *= 2;
		}
	}
	return avgMonth * (12.0 / 365.25);
}

qint64 FeaturePeriodicAmount::distance(const Transaction *targ, const Transaction *trans)
{
	return targ->dist(*trans);
}
