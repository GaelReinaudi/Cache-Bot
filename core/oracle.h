#ifndef ORACLE_H
#define ORACLE_H
#include "core_global.h"
#include "common.h"
#include "transaction.h"
class AccountFeature;

class CORESHARED_EXPORT Oracle
{
public:
	Oracle(AccountFeature* pCreatingFeature)
	 : m_feature(pCreatingFeature)
	{}
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

	AccountFeature* feature() const {
		return m_feature;
	}
	QJsonObject toJson() const {
		QJsonObject ret;
		ret["aaaaaaaaaaaaaaaaaaaaaaaaaaaa"] = 123456;
		return ret;
	}

private:
	QDate m_iniDate;
	QDate m_curDate;
	AccountFeature* m_feature = 0;
};

class CORESHARED_EXPORT SuperOracle : public Oracle
{
public:
	SuperOracle()
		: Oracle(0)
	{}

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
		DBG() << "addSubOracle ";
		m_subOracles.append(pOr);
	}
	QVector<Transaction> revelation(QDate upToDate) override;
	double avgDaily() const override;

	struct Summary
	{
		double flow = 0.0;
		QVector<double> dailyPerOracle;
		QVector<QJsonObject> summaryPerOracle;
	};
	Summary computeAvgCashFlow() const;

private:
	QVector<QSharedPointer<Oracle> > m_subOracles;
};


#endif // ORACLE_H
