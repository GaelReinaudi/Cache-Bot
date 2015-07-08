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
	void resetDate(QDate initialDate) {
		m_iniDate = initialDate;
		m_curDate = m_iniDate;
	}
	QDate curDate() const {
		return m_curDate;
	}
	void nextDay() {
		m_curDate.addDays(1);
	}

	virtual QVector<Transaction> revelation(QDate upToDate);

protected:
	//	virtual Transaction devineTransaction() = 0;

private:
	QDate m_iniDate;
	QDate m_curDate;
};

#endif // ORACLE_H
