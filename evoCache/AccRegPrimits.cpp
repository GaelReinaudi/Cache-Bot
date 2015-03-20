#include "AccRegPrimits.h"

void FeatureMonthlyAmount::execute(void *outDatum, Puppy::Context &ioContext)
{
	AccountFeature::execute(outDatum, ioContext);
	double& fitness = *(double*)outDatum;

	getArgs(ioContext);
	cleanArgs();

	fitness = 0.0;

	TransactionBundle& allTrans = ioContext.m_pAccount->allTrans();
	QDate lastDate = ioContext.m_pAccount->lastTransactionDate();
	QDate iniDate = ioContext.m_pAccount->firstTransactionDate().addMonths(1);

	QVector<Transaction> targetTrans;
	QDate currentDate = iniDate;
	while (currentDate < lastDate) {
		// the target day of this month. If neg, make it count from the end of the month.
		int targetDayThisMonth = qMin(m_dayOfMonth, currentDate.daysInMonth());
		while (targetDayThisMonth <= 0)
			targetDayThisMonth += currentDate.daysInMonth();

		// move current date to the correct target day that month
		currentDate = currentDate.addDays(targetDayThisMonth - currentDate.day());
		targetTrans.append(Transaction());
		targetTrans.last().date = currentDate;
		targetTrans.last().kamount = m_kamount;
		targetTrans.last().nameHash.b[0] = m_b[0];
		targetTrans.last().nameHash.b[1] = m_b[1];
		targetTrans.last().nameHash.b[2] = m_b[2];
		targetTrans.last().nameHash.b[3] = m_b[3];
		currentDate = currentDate.addMonths(1);
	}
	if (targetTrans.count() == 0) {
		LOG() << "MonthlyAmount(0 TARGET): day "<<m_dayOfMonth<<" amount "<<(m_kamount/1024) << endl;
	}
	else {
		//LOG() << "MonthlyAmount("<<targetTrans.count()<<" TARGET): day "<<m_dayOfMonth<<" amount "<<(m_kamount/1024)<<" hash "<<targetTrans.first().nameHash.hash << endl;
	}

	float totalOneOverExpDist = 0.0;
	quint64 localDist = quint64(-1);
	Transaction* localTrans = 0;
	// the current target to compare to
	Transaction* iTarg = &targetTrans[0];
	m_bundle.clear();
	for (int i = 0; i < allTrans.count(); ++i) {
		Transaction& trans = allTrans.trans(i);
		quint64 dist = iTarg->dist(trans);
		if (dist < localDist) {
			localDist = dist;
			localTrans = &trans;
		}
		// if we get further away by 15 days, we take the next target, or if last trans
		if (trans.jDay() > 15 + iTarg->jDay() || i == allTrans.count() - 1) {
			m_bundle.append(localTrans);
			totalOneOverExpDist += expoInt<64>(-localDist);
			if (iTarg == &targetTrans.last())
				break;
			++iTarg;
			localTrans = 0;
			localDist = quint64(-1);
		}
	}
	fitness += totalOneOverExpDist;

	// isolate the transaction that were fitted to the target
	for (int i = 0; i < m_bundle.count(); ++i) {
		Transaction& t = m_bundle.trans(i);
		t.dimensionOfVoid = 1;
	}

	// summary if the QStringList exists
	if (ioContext.m_sumamryStrList) {
		QString str;
		str = QString::fromStdString(getName()) + " ("
			  + QString::number(m_bundle.count()) +  ") "
			  + " $" + QString::number(m_kamount / 1024)
			  + " on the " + QString::number(m_dayOfMonth) + "th";
		ioContext.m_sumamryStrList->append(str);
		str = QString("avg label: ") + m_bundle.averageName();
		ioContext.m_sumamryStrList->append(str);
		str = QString("tot amount: ") + QString::number(m_bundle.sumDollar());
		ioContext.m_sumamryStrList->append(str);
		str = QString("----------------------------");
		ioContext.m_sumamryStrList->append(str);

		for (int i = 0; i < m_bundle.count(); ++i) {
			Transaction* iTarg = &targetTrans[i];
			Transaction& t = m_bundle.trans(i);
			emit evoSpinner()->sendMask(iTarg->time_t(), iTarg->amountDbl());
			emit evoSpinner()->sendMask(t.time_t(), t.amountDbl());
		}
	}
}
