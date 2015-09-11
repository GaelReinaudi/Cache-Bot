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
		m_feature.ret["aaaaaaaaaaaaaaaaaaaaaaaaaaaa"] = 123456;
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
//	private:
		double posSum = 0.0;
		double negSum = 0.0;
		QVector<double> dailyPerOracle;
		QVector<QJsonObject> summaryPerOracle;
//	public:
		QJsonObject toJson() {
			QJsonObject jsSum;
			QJsonArray allVal;
			QJsonArray allSum;
			for (int i = 0; i < dailyPerOracle.count(); ++i) {
				allVal.append(dailyPerOracle[i]);
				allSum.append(summaryPerOracle[i]);
			}
			jsSum.insert("contribs", allVal);
			jsSum.insert("oracles", allSum);
			return jsSum;
		}
		double flow() const {
			if (posSum == 0.0) {
				WARN() << "Summary::flow() posAvg == 0.0 ";
				return 0.0;
			}
			return (posSum + negSum) / posSum;
		}
		double posPartialDif() const {
			return -negSum / (posSum * posSum);
		}
		double negPartialDif() const {
			return 1.0 / posSum;
		}
		Summary operator+(const Summary& other) const {
			Q_ASSERT(dailyPerOracle.count() == other.dailyPerOracle.count());
			Summary sum(*this);
			sum.posSum += other.posSum;
			sum.negSum += other.negSum;
			for (int i = 0; i < sum.dailyPerOracle.count(); ++i) {
				sum.dailyPerOracle[i] += other.dailyPerOracle[i];
			}
			return sum;
		}
		Summary operator-(const Summary& other) const {
			Q_ASSERT(dailyPerOracle.count() == other.dailyPerOracle.count());
			Summary dif(*this);
			dif.posSum -= other.posSum;
			dif.negSum -= other.negSum;
			for (int i = 0; i < dif.dailyPerOracle.count(); ++i) {
				dif.dailyPerOracle[i] -= other.dailyPerOracle[i];
			}
			return dif;
		}
		Summary operator*(const double& fac) const {
			Summary mul(*this);
			mul.posSum *= fac;
			mul.negSum *= fac;
			for (int i = 0; i < mul.dailyPerOracle.count(); ++i) {
				mul.dailyPerOracle[i] *= fac;
			}
			return mul;
		}
		Summary effectOf(const Summary& deltaSummary) const {
			Q_ASSERT(dailyPerOracle.count() == deltaSummary.dailyPerOracle.count());
			Summary eff(*this);
			for (int i = 0; i < eff.dailyPerOracle.count(); ++i) {
				double slope = eff.dailyPerOracle[i] > 0 ? posPartialDif() : negPartialDif();
				eff.dailyPerOracle[i] = slope * deltaSummary.dailyPerOracle[i];
			}

			return eff;
		}
	};
	Summary computeAvgCashFlow() const;

private:
	QVector<QSharedPointer<Oracle> > m_subOracles;
};


#endif // ORACLE_H
