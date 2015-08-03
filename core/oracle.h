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

	virtual QVector<Transaction> revelation(QDate upToDate);

private:
	QDate m_iniDate;
	QDate m_curDate;
};

class SuperOracle : public Oracle
{
public:
	void resetDate(QDate initialDate) override {
		Oracle::resetDate(initialDate);
		for (Oracle* pOr : m_subOracles) {
			pOr->resetDate(initialDate);
		}
	}
	void nextDay() override {
		Oracle::nextDay();
		for (Oracle* pOr : m_subOracles) {
			pOr->nextDay();
		}
	}
	void clearSubOracles() {
		LOG() << "clearSubOracles" << endl;
		m_subOracles.clear();
	}
	void addSubOracle(Oracle* pOr) {
		LOG() << "addSubOracle " << pOr << endl;
		m_subOracles.append(pOr);
	}
	QVector<Transaction> revelation(QDate upToDate) override;

private:
	QVector<Oracle*> m_subOracles;
};

#endif // ORACLE_H
