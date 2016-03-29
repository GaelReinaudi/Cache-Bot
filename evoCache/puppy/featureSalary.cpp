#include "featureSalary.h"

QVector<Transaction> FeatureBiWeeklySalaryReally::targetTransactions(QDate iniDate, QDate lastDate) {
	static QVector<Transaction> targetTrans;
	targetTrans = BlankTransactionsForJdOffset(iniDate, lastDate, m_localStaticArgs.m_dayOfMonth);
	// loops through to set a few variables of the transactions
	for (Transaction& tr : targetTrans) {
		tr.setKLA(m_localStaticArgs.m_kla);
		tr.nameHash.setFromHash(m_localStaticArgs.m_hash);
		tr.flags |= Transaction::Predicted;
	}
	return targetTrans;
}


