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
	virtual double avgDailyPos() const {
		double avg = avgDaily();
		return avg >= 0 ? avg : 0.0;
	}
	virtual double avgDailyNeg() const {
		double avg = avgDaily();
		return avg <= 0 ? avg : 0.0;
	}


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
		NOTICE() << "clearSubOracles";
		m_subOracles.clear();
	}
	void addSubOracle(QSharedPointer<Oracle> pOr) {
		NOTICE() << "addSubOracle ";
		m_subOracles.append(pOr);
	}
	QVector<Transaction> revelation(QDate upToDate) override;
	double avgDaily() const override;
	double avgCashFlow() const;

private:
	QVector<QSharedPointer<Oracle> > m_subOracles;
};


#endif // ORACLE_H
