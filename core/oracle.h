#ifndef ORACLE_H
#define ORACLE_H
#include "core_global.h"
#include "common.h"
#include "transaction.h"

class CORESHARED_EXPORT Oracle
{
public:
	Oracle() {}
	virtual ~Oracle() {}

public:
	// resets the oracle to a fresh state where it can provide transactions and reseting potential internal states
	virtual void resetDate(QDate initialDate) {
		m_iniDate = initialDate;
		m_curDate = m_iniDate;
	}
	virtual void nextDay() {
		m_curDate = m_curDate.addDays(1);
	}
	QDate curDate() const {
		return m_curDate;
	}

	virtual QVector<Transaction> revelation(QDate upToDate) = 0;
	virtual double avgDaily() const = 0;


private:
	QDate m_iniDate;
	QDate m_curDate;
};

class CORESHARED_EXPORT SuperOracle : public Oracle
{
public:
	void resetDate(QDate initialDate) override {
		Oracle::resetDate(initialDate);
		for (auto pOr : m_subOracles) {
			pOr->resetDate(initialDate);
		}
	}
	void nextDay() override {
		Oracle::nextDay();
		for (auto pOr : m_subOracles) {
			pOr->nextDay();
		}
	}
	void clearSubOracles() {
		LOG() << "clearSubOracles" << endl;
		m_subOracles.clear();
	}
	void addSubOracle(QSharedPointer<Oracle> pOr) {
		LOG() << "addSubOracle " << pOr.data() << endl;
		m_subOracles.append(pOr);
	}
	QVector<Transaction> revelation(QDate upToDate) override;
	double avgDaily() const override;
	double avgCashFlow() const;

private:
	QVector<QSharedPointer<Oracle> > m_subOracles;
};

////! returns a map of dayInTheFuture (0 is today), with multiple values (if on same day) organized by amount
//SparkLine predictedSparkLine(double threshProba) {
//	// will hold temporary relative changes so that they can be organized
//	// with the losses first (to be safe and not predict a salary before a bill if on the same day)
//	SparkLine temp;
//	for (Transaction& t : predictedFutureTransactions(threshProba)) {
//		int futDays = m_today.daysTo(t.date);
//		temp.insertMulti(futDays, t.amountDbl());
//	}

//	// now we sort them and make absolute values in the Sparkline
//	double balanceNow = balance(Account::Type::Checking);
//	LOG() << "predictedSparkLine(" << threshProba << "), temp.size = " << temp.size() << ", temp.uniqueKeys = " << temp.uniqueKeys().size() << ". balanceNow = " << balanceNow << endl;
//	qDebug() << "temp.uniqueKeys()" << temp.uniqueKeys();
//	SparkLine ret;
//	// insert the balanceNow for the last transaction day
//	ret.insertMulti(-9999, balanceNow);
//	for (auto futDay : temp.uniqueKeys()) {
//		QList<double> transThatDay = temp.values(futDay);
//		std::sort(transThatDay.begin(), transThatDay.end());
//		// insert from the end (see QMap::values(key) documentation)
//		for (int i = transThatDay.size() - 1; i >= 0; --i) {
//			balanceNow += transThatDay[i];
//			ret.insertMulti(futDay, balanceNow);
//			LOG() << "on day " << futDay << ": " << transThatDay[i] << " -> " << balanceNow << endl;
//		}
//	}
//	qDebug() << ret;
//	return ret;
//}


#endif // ORACLE_H
