#ifndef USERMETRICS_H
#define USERMETRICS_H
#include "histoMetric.h"
#include "user.h"
#include "oracle.h"

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
class CostRateMonthPercentileMetric : public UserMetric
{
protected:
	CostRateMonthPercentileMetric(User* pUser)
		: UserMetric(Name(), pUser)
	{
		Q_ASSERT(Percentile >= 0 && Percentile <= 100);
	}
	CostRateMonthPercentileMetric(const QString& name, User* pUser)
		: UserMetric(name, pUser)
	{
		Q_ASSERT(Percentile >= 0 && Percentile <= 100);
	}

	static QString Name() {
		return QString("cost%1MonthP%2").arg(PastMonth).arg(Percentile);
	}

public:
	static CostRateMonthPercentileMetric<PastMonth, Percentile>* get(User* pUser) {
		auto pMet = HistoMetric::get(Name());
		if (pMet)
			return reinterpret_cast<CostRateMonthPercentileMetric<PastMonth, Percentile>*>(pMet);
		return new CostRateMonthPercentileMetric<PastMonth, Percentile>(pUser);
	}

protected:
	double computeFor(const QDate& date, bool& isValid) override {
		return templateComputeFor<-1>(date, isValid);
	}
	template <int Multiplicator>
	double templateComputeFor(const QDate& date, bool& isValid) {
		QVector<double> costs;
		QDate startDate = date.addMonths(-PastMonth);
		for (int i = 0; i < user()->allTrans().count(); ++i) {
			const Transaction& tr = user()->allTrans().trans(i);
			double amnt = tr.amountDbl();
			amnt *= Multiplicator;
			// filter on date
			if (tr.date <= date && tr.date > startDate) {
				if (amnt > 0.0 && !tr.isInternal()) {
					costs.append(amnt);
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
		if (!isValid) {
			LOG() << "templateComputeFor<" << PastMonth << Percentile << "> not valid " << lastCostsInd << costs.count() << endl;
			return 0.0;
		}
		double numDays = startDate.daysTo(date);
		if (numDays) {
			avg /= numDays;
		}
		else
			return 0.0;
		return avg * Multiplicator;
	}
};

template <int PastMonth, int Percentile>
class MakeRateMonthPercentileMetric : public CostRateMonthPercentileMetric<PastMonth, Percentile>
{
protected:
	MakeRateMonthPercentileMetric(User* pUser)
		: CostRateMonthPercentileMetric<PastMonth, Percentile>(Name(), pUser)
	{
	}

	static QString Name() {
		return QString("rateMake%1MonthP%2").arg(PastMonth).arg(Percentile);
	}

public:
	static MakeRateMonthPercentileMetric<PastMonth, Percentile>* get(User* pUser) {
		auto pMet = HistoMetric::get(Name());
		if (pMet)
			return reinterpret_cast<MakeRateMonthPercentileMetric<PastMonth, Percentile>*>(pMet);
		return new MakeRateMonthPercentileMetric<PastMonth, Percentile>(pUser);
	}

protected:
	double computeFor(const QDate& date, bool& isValid) override {
		return this->template templateComputeFor<1>(date, isValid);
	}
};

template <int InPercentile, int OutPercentile>
class Flow01 : public UserMetric
{
protected:
	Flow01(User* pUser)
		: UserMetric(Name(), pUser)
	{
		inMet =  MetricSmoother<7>::get(MakeRateMonthPercentileMetric<2, InPercentile>::get(user()));
		outMet = MetricSmoother<7>::get(CostRateMonthPercentileMetric<2, OutPercentile>::get(user()));
	}
//	Flow01(const QString& name, User* pUser)
//		: UserMetric(name, pUser)
//	{ }
	static QString Name() {
		return QString("Flow01InP%1OutP%2").arg(InPercentile).arg(OutPercentile);
	}

public:
	static Flow01<InPercentile, OutPercentile>* get(User* pUser) {
		auto pMet = HistoMetric::get(Name());
		if (pMet)
			return reinterpret_cast<Flow01<InPercentile, OutPercentile>*>(pMet);
		return new Flow01<InPercentile, OutPercentile>(pUser);
	}

protected:
	double computeFor(const QDate& date, bool& isValid) override {
		double in = inMet->value(date);
		double out = outMet->value(date);
		isValid = inMet->isValid(date) && outMet->isValid(date);
		isValid &= in > 0.0;
		if (!isValid) {
			LOG() << "in or out not valid" << endl;
			return 0.0;
		}
		double flow = in + out;
		flow /= in;
		flow *= 100;
		return flow;
	}
private:
	HistoMetric* inMet;
	HistoMetric* outMet;
};

class Flow02 : public UserMetric
{
protected:
	Flow02(User* pUser)
		: UserMetric(Name(), pUser)
		, m_pUser(pUser)
	{
	}
	static QString Name() {
		return QString("Flow02");
	}

public:
	static Flow02* get(User* pUser) {
		auto pMet = HistoMetric::get(Name());
		if (pMet)
			return reinterpret_cast<Flow02*>(pMet);
		return new Flow02(pUser);
	}

protected:
	double computeFor(const QDate& date, bool& isValid) override {
		isValid = true;
		double flow = m_pUser->oracle()->avgCashFlow();
		return flow;
	}
private:
	User* m_pUser = 0;
};



#endif // USERMETRICS_H
