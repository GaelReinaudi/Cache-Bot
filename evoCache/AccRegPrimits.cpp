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
	QDate firstDate = ioContext.m_pAccount->firstTransactionDate();

	QVector<Transaction> targetTransReverse(1024);
	QDate currentDate = lastDate;
	while (currentDate > firstDate) {
		// the target day of this moth. If neg, make it count from the end of the month.
		int targetDayThisMonth = m_dayOfMonth;
		if (targetDayThisMonth < 0)
			targetDayThisMonth == currentDate.daysInMonth();
		// if we are above that day, we expect our monthly target right there
		int targetToCurrent = currentDate.day() - targetDayThisMonth;
		if (targetToCurrent > 0) {
			targetTransReverse.append(Transaction);
			targetTransReverse.last().date = currentDate.addDays(-targetToCurrent);
			targetTransReverse.last().kamount = m_kamount.
		}
		else { //
		}
		currentDate
	}


	for (int i = 0; i < allTrans.count(); ++i) {
		Transaction& trans = allTrans.trans(i);


		fitness += trans.amountDbl();
	}
}
