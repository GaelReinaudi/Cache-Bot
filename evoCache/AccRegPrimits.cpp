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
	// the current target to compare to
	Transaction* iTarg = &targetTrans[0];
	for (int i = 0; i < allTrans.count(); ++i) {
		Transaction& trans = allTrans.trans(i);
		localDist = qMin(localDist, iTarg->dist(trans));

		// if we get further away by 15 days, we take the next target, or if last trans
		if (trans.jDay() > 15 + iTarg->jDay() || i == allTrans.count() - 1) {
			++iTarg;
			totalOneOverExpDist += expoInt<128>(-localDist);
			localDist = quint64(-1);
			continue;
		}
	}
	fitness += totalOneOverExpDist;
}
