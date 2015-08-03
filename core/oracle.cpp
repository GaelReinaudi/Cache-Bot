#include "oracle.h"



QVector<Transaction> Oracle::revelation(QDate upToDate) {
	Q_UNUSED(upToDate);
	return QVector<Transaction>();
}


QVector<Transaction> SuperOracle::revelation(QDate upToDate)
{
	LOG() << "SuperOracle::revelation from " << m_subOracles.count() << endl;
	QVector<Transaction> ret = Oracle::revelation(upToDate);
	// naive adding up the revelations for now
	for (Oracle* pOr : m_subOracles) {
		ret += pOr->revelation(upToDate);
	}
	std::stable_sort(ret.begin(), ret.end(), Transaction::smallerAmountThan);
	std::stable_sort(ret.begin(), ret.end(), Transaction::earlierThan);
	return ret;
}
