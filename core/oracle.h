#ifndef ORACLE_H
#define ORACLE_H
#include "core_global.h"
#include "common.h"
#include "transaction.h"
class AccountFeature;

static const int displayDayPast = 31;
static const int displayDayFuture = 62;

template<int Nrun>
struct Simulation
{
private:
	static const int Ntot = Nrun * (displayDayFuture + 1);
	double values[Ntot];

public:
	double val(int run, int day) const {
		return values[run * (displayDayFuture + 1) + day];
	}
	void setVal(int run, int day, double value) {
		values[run * (displayDayFuture + 1) + day] = value;
	}

	int timeToDelta(double deltaBalance, double facPerc = 0.5) const {
		for (int d = 0; d <= displayDayFuture; ++d) {
			int ctBellow = 0;
			for (int r = 0; r < Nrun; ++r) {
				if (val(r, d) <= deltaBalance)
					++ctBellow;
			}
			if (ctBellow >= Nrun * facPerc)
				return d;
		}
		return 9999;
	}
};

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
	virtual QJsonObject toJson() const;
	virtual QString description() const { return "????"; }
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

	bool isPostTreatment = false;
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
		//NOTICE() << "clearSubOracles";
		m_subOracles.clear();
	}
	void addSubOracle(QSharedPointer<Oracle> pOr) {
		DBG() << "addSubOracle ";
		m_subOracles.append(pOr);
	}
	QVector<Transaction> revelation(QDate upToDate) override;
	double avgDaily() const;
	template <int Nrun>
	Simulation<Nrun> simu() {
		Simulation<Nrun> s;
		QMap<double, double> dat[Nrun];
		for (int r = 0; r < Nrun; ++r) {
			dat[r].clear();
			double curBal = 0.0;
			int t = 0;
			dat[r].insert(t, curBal);
			resetDate(Transaction::currentDay());
			const QVector<Transaction>& rev = revelation(Transaction::currentDay().addDays(displayDayFuture));
			for (const Transaction& tr : rev) {
				double amnt = tr.amountDbl();
				curBal += amnt;
				t = Transaction::currentDay().daysTo(tr.date);
				dat[r].insert(t, curBal);
			}
		}
		for (double d = 0; d <= displayDayFuture; ++d) {
			for (int r = 0; r < Nrun; ++r) {
				auto it = dat[r].upperBound(d + 0.1) - 1;
				if (it+1 != dat[r].begin() && it != dat[r].end()) {
					double vi = *it;
					s.setVal(r, d, vi);
				}
				else
					s.setVal(r, d, 0.0);
			}
		}
		return s;
	}

	struct CORESHARED_EXPORT Summary
	{
		double posSum = 0.0;
		double negSum = 0.0;
		double bill = 0.0;
		double salary = 0.0;
		QVector<double> dailyPerOracle;
		QVector<QJsonObject> summaryPerOracle;
		QJsonArray toJson() {
			QJsonArray allSum;
			for (int i = 0; i < summaryPerOracle.count(); ++i) {
				allSum.append(summaryPerOracle[i]);
			}
			return allSum;
		}
		double flow() const {
			if (posSum == 0.0) {
				DBG(3) << "Summary::flow() posAvg == 0.0 ";
				return 0.0;
			}
			return (posSum + negSum) / posSum;
		}
		double flowFix() const {
			if (salary == 0.0) {
				DBG(3) << "Summary::flow() posAvg == 0.0 ";
				return 0.0;
			}
			return (salary + bill) / salary;
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
		Summary effectOf(const Summary& endSummary, int overDays) const;
	};
	Summary computeAvgCashFlow(bool includeOracleSummaries = true) const;

private:
	QVector<QSharedPointer<Oracle> > m_subOracles;
};


#endif // ORACLE_H
