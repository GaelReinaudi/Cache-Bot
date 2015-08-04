#include "oracle.h"



QVector<Transaction> SuperOracle::revelation(QDate upToDate)
{
	LOG() << "SuperOracle::revelation from " << m_subOracles.count() << endl;
	QVector<Transaction> ret;
	// naive adding up the revelations for now
	for (auto pOr : m_subOracles) {
		ret += pOr->revelation(upToDate);
	}
	std::stable_sort(ret.begin(), ret.end(), Transaction::smallerAmountThan);
	std::stable_sort(ret.begin(), ret.end(), Transaction::earlierThan);
	return ret;
}
