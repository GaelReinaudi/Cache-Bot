#include "AccRegPrimits.h"

void FeatureMonthlyAmount::execute(void *outDatum, Puppy::Context &ioContext)
{
	AccountFeature::execute(outDatum, ioContext);
	double& fitness = *(double*)outDatum;

	getArgs(ioContext);
	cleanArgs();

	fitness = 0.0;

	int filterHashIndex = ioContext.filterHashIndex;
	int filterHash = filterHashIndex < 0 ? -1 : ioContext.m_pAccount->hashBundles().keys()[filterHashIndex];

	TransactionBundle& allTrans = ioContext.m_pAccount->allTrans(filterHash);
	QDate lastDate = ioContext.m_pAccount->lastTransactionDate();
	QDate iniDate = ioContext.m_pAccount->firstTransactionDate();

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
		targetTrans.last().indexHash = 0;
		targetTrans.last().indexHash += m_b[0] * 1 << 0;
		targetTrans.last().indexHash += m_b[1] * 1 << 4;
		targetTrans.last().indexHash += m_b[2] * 1 << 8;
		targetTrans.last().indexHash += m_b[3] * 1 << 12;
		targetTrans.last().nameHash.hash = m_b[0];
		// if we are forcing a given hashed bundle
		if (filterHashIndex >= 0) {
			targetTrans.last().indexHash = filterHashIndex;
			targetTrans.last().nameHash.hash = filterHash;
		}
		currentDate = currentDate.addMonths(1);
	}
	if (targetTrans.count() == 0) {
		LOG() << "MonthlyAmount(0 TARGET): day "<<m_dayOfMonth<<" kla "<< m_kla << endl;
	}
	else {
//		LOG() << "MonthlyAmount("<<targetTrans.count()<<" TARGET): day "<<m_dayOfMonth<<" kla"<< m_kla <<"="<<targetTrans.first().amountDbl() << " h=" <<targetTrans.first().nameHash.hash << endl;
	}
	if (double(m_kla) < -6 * KLA_MULTIPLICATOR || double(m_kla) > 6 * KLA_MULTIPLICATOR) {
		fitness = -1;
	}

	double totalOneOverExpDist = 0.0;
	quint64 localDist = 18446744073709551615ULL;
	Transaction* localTrans = 0;
	// the current target to compare to
	Transaction* iTarg = &targetTrans[0];
	m_bundle.clear();
	if (ioContext.m_sumamryStrList) {
		m_bundle.clear();
	}
	for (int i = 0; i < allTrans.count(); ++i) {
		Transaction& trans = allTrans.trans(i);
		quint64 dist = iTarg->dist(trans);
		if (dist < localDist) {
			localDist = dist;
			localTrans = &trans;
		}
		Q_ASSERT(localDist < 18446744073709551615ULL);
		static const int LIMIT_DIST_TRANS = 32;
		// if we get further away by 15 days, we take the next target, or if last trans
		if (trans.jDay() > 15 + iTarg->jDay() || i == allTrans.count() - 1) {
			if (localDist < LIMIT_DIST_TRANS) {
				m_bundle.append(localTrans);
				// isolate the transaction that were fitted to the target
				Q_ASSERT(localTrans->dimensionOfVoid == 0);
				localTrans->dimensionOfVoid++;
			}
//				totalOneOverExpDist += expoInt<64>(-localDist);
				totalOneOverExpDist += 1.0 / (1 + localDist);
			if (iTarg == &targetTrans.last())
				break;
			++iTarg;
			localTrans = 0;
			localDist = quint64(-1);
			// keep this last trans in the pool if it was not just added
			if (&trans != localTrans)
				--i;
		}
	}
	// only sum that add up to > $10
	if (qAbs(m_bundle.sumDollar()) > 10) {
		fitness += totalOneOverExpDist;
		fitness += qAbs(kindaLog(m_bundle.sumDollar())) * totalOneOverExpDist / m_bundle.count();
		fitness /= targetTrans.count();
	}

//	// isolate the transaction that were fitted to the target
//	for (int i = 0; i < m_bundle.count(); ++i) {
//		Transaction& t = m_bundle.trans(i);
//		Q_ASSERT(t.dimensionOfVoid == 0);
//		++t.dimensionOfVoid;
//	}

	// summary if the QStringList exists
	if (ioContext.m_sumamryStrList && m_bundle.count()) {
		QString str;
		str = QString::fromStdString(getName()) + " ("
			+ QString::number(m_bundle.count()) +  ") "
			+ " kl$ " + QString::number(double(m_kla) / KLA_MULTIPLICATOR)
			+ " / " + QString::number(kindaLog(m_bundle.sumDollar() / m_bundle.count()))
			+ " = " + QString::number(unKindaLog(double(m_kla) / KLA_MULTIPLICATOR))
			+ " / " + QString::number(m_bundle.sumDollar() / m_bundle.count());
		ioContext.m_sumamryStrList->append(str);
		str = QString("On the ") + QString::number(m_dayOfMonth) + "th, ";
		str += QString("hash: ") + QString::number(m_bundle.trans(0).nameHash.hash);
		str += QString("  fitness: ") + QString::number(fitness);
		ioContext.m_sumamryStrList->append(str);
//		str = QString("avg label: ") + m_bundle.averageName();
//		ioContext.m_sumamryStrList->append(str);
		str = QString("all label: ") + m_bundle.uniqueNames().join(" | ");
		ioContext.m_sumamryStrList->append(str);
		str = QString("tot amount: ") + QString::number(m_bundle.sumDollar());
		ioContext.m_sumamryStrList->append(str);
		str = QString("----------------------------");
		ioContext.m_sumamryStrList->append(str);

		for (int i = 0; i < targetTrans.count(); ++i) {
			Transaction* iTarg = &targetTrans[i];
			emit evoSpinner()->sendMask(iTarg->time_t(), iTarg->amountDbl(), true);
		}
		for (int i = 0; i < m_bundle.count(); ++i) {
			Transaction& t = m_bundle.trans(i);
			emit evoSpinner()->sendMask(t.time_t(), t.amountDbl(), false);
		}
	}
}
