#ifndef USERMETRICS_H
#define USERMETRICS_H
#include "histoMetric.h"
#include "user.h"

class UserMetric : public HistoMetric
{
public:
	UserMetric(const QString& name, User* pUser)
		: HistoMetric(name, pUser)
		, m_user(pUser)
	{}

	User* user() const {return m_user; }

private:
	User* m_user = 0;
};

template <int PastMonth, int Percentile>
class CostMonthPercentileMetric : public UserMetric
{
protected:
	CostMonthPercentileMetric(User* pUser)
		: UserMetric(Name(), pUser)
	{
		Q_ASSERT(Percentile >= 0 && Percentile <= 100);
	}

	static QString Name() {
		return QString("cost%1MonthP%2").arg(PastMonth).arg(Percentile);
	}

public:
	static CostMonthPercentileMetric<PastMonth, Percentile>* get(User* pUser) {
		auto pMet = HistoMetric::get(Name());
		if (pMet)
			return reinterpret_cast<CostMonthPercentileMetric<PastMonth, Percentile>*>(pMet);
		return new CostMonthPercentileMetric<PastMonth, Percentile>(pUser);
	}

protected:
	double computeFor(const QDate& date, bool& isValid) override {
		QVector<double> costs;
		QDate startDate = date.addMonths(-PastMonth);
		for (int i = 0; i < user()->allTrans().count(); ++i) {
			Transaction& tr = user()->allTrans().trans(i);
			double amnt = tr.amountDbl();
			// filter on date
			if (tr.date <= date && tr.date > startDate) {
				if (amnt < 0.0 && !tr.isInternal()) {
					costs.append(-amnt);
				}
			}
		}
		qSort(costs);
		double avg = 0.0;
		int lastCostsInd = costs.count() * Percentile / 100;
		for (int i = 0; i < lastCostsInd; ++i) {
			avg += costs[i];
			// set as valid if we had at least one value
			isValid |= true;
		}
		double numDays = startDate.daysTo(date);
		if (numDays) {
			avg /= numDays;
		}
		else
			return 0.0;
		return avg;
	}

};

#endif // USERMETRICS_H
