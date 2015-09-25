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

protected:
	User* m_user = 0;
};

template <int PastMonth, int Percentile>
class CostRateMonthPercentileMetric : public UserMetric
{
protected:
	CostRateMonthPercentileMetric(User* pUser) : UserMetric(Name(), pUser)
	{
		Q_ASSERT(Percentile >= 0 && Percentile <= 100);
	}
	CostRateMonthPercentileMetric(const QString& name, User* pUser) : UserMetric(name, pUser)
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
				if (amnt > 0.0 && !tr.noUse()) {
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
//			WARN() << "templateComputeFor<" << PastMonth << Percentile << "> not valid " << lastCostsInd << costs.count();
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
	MakeRateMonthPercentileMetric(User* pUser) : CostRateMonthPercentileMetric<PastMonth, Percentile>(Name(), pUser)
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
	Flow01(User* pUser) : UserMetric(Name(), pUser)
	{
		inMet =  MetricSmoother<7>::get(MakeRateMonthPercentileMetric<2, InPercentile>::get(user()));
		outMet = MetricSmoother<7>::get(CostRateMonthPercentileMetric<2, OutPercentile>::get(user()));
	}
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
			WARN() << "in or out not valid";
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

class BalanceMetric : public UserMetric
{
protected:
	BalanceMetric(User* pUser) : UserMetric(Name(), pUser)
	{
	}
	static QString Name() {
		return QString("BalanceMetric");
	}

public:
	static BalanceMetric* get(User* pUser) {
		auto pMet = HistoMetric::get(Name());
		if (pMet)
			return reinterpret_cast<BalanceMetric*>(pMet);
		return new BalanceMetric(pUser);
	}

protected:
	double computeFor(const QDate& date, bool& isValid) override {
		double bal = user()->balance(Account::Type::Checking);
		isValid = false;
		// transaction at the starting date of the playback
		auto& real = user()->allTrans();
		for (int i = 0; i < real.count(); ++i) {
			// finds the index of the last transaction within the playback date
			if (real.trans(i).date > date) {
				// incrementally finds out the balance at the playback date
				if (!real.trans(i).isInternal()) {
					bal -= real.trans(i).amountDbl();
				}
			}
			else // if at least one transaction before that date
				isValid = true;
		}
		return bal;
	}
};

class OracleSummary : public UserMetric
{
protected:
	OracleSummary(User* pUser) : UserMetric(Name(), pUser)
	{
	}
	static QString Name() {
		return QString("OracleSummary");
	}

public:
	static OracleSummary* get(User* pUser) {
		auto pMet = HistoMetric::get(Name());
		if (pMet)
			return reinterpret_cast<OracleSummary*>(pMet);
		return new OracleSummary(pUser);
	}

	QMap<QDate, SuperOracle::Summary> summaries() const{ return m_summaries; }

protected:
	double computeFor(const QDate& date, bool& isValid) override {
		NOTICE() << "OracleSummary computeFor " << date.toString();
		if (m_summaries.contains(date)) {
			return m_summaries[date].flow();
		}
		QDate oldCurrentDate = Transaction::currentDay();
		// set computation date
		Transaction::setCurrentDay(date);
		user()->reComputeBot();
		SuperOracle::Summary summary = user()->oracle()->computeAvgCashFlow();
		m_summaries[date] = summary;

		isValid = true;
		// back to where we were
		Transaction::setCurrentDay(oldCurrentDate);
		return summary.flow();
	}
private:
	QMap<QDate, SuperOracle::Summary> m_summaries;
};

template<int Nrun>
class Montecarlo : public UserMetric
{
protected:
	Montecarlo(User* pUser) : UserMetric(Name(), pUser)
	{
	}
	static QString Name() {
		return QString("Montecarlo");
	}

public:
	static Montecarlo<Nrun>* get(User* pUser) {
		auto pMet = HistoMetric::get(Name());
		if (pMet)
			return reinterpret_cast<Montecarlo<Nrun>*>(pMet);
		return new Montecarlo<Nrun>(pUser);
	}
	double t2zPerc(const QDate &date, double facPerc) {
		double curBal = BalanceMetric::get(user())->value(date);
		return m_simulations[date].timeToDelta(-curBal, facPerc);
	}

protected:
	double computeFor(const QDate& date, bool& isValid) override {
		NOTICE() << "Montecarlo<"<<Nrun<<"> computeFor " << date.toString();
		QDate oldCurrentDate = Transaction::currentDay();
		// set computation date
		Transaction::setCurrentDay(date);
		user()->reComputeBot();
		m_simulations[date] = user()->oracle()->template simu<Nrun>();

		isValid = true;
		// back to where we were
		Transaction::setCurrentDay(oldCurrentDate);

		double curBal = BalanceMetric::get(user())->value(date);
		return m_simulations[date].timeToDelta(-curBal);
	}
private:
	QMap<QDate, Simulation<Nrun> > m_simulations;
};

template <int overLastDays>
class OracleTrend : public UserMetric
{
protected:
	OracleTrend(User* pUser)
		: UserMetric(Name(), pUser)
	{
		summaryMet =  OracleSummary::get(pUser);
	}
	static QString Name() {
		return QString("OracleTrend%1").arg(overLastDays);
	}

public:
	static OracleTrend<overLastDays>* get(User* pUser) {
		auto pMet = HistoMetric::get(Name());
		if (pMet)
			return reinterpret_cast<OracleTrend<overLastDays>*>(pMet);
		return new OracleTrend<overLastDays>(pUser);
	}

	QMap<QDate, SuperOracle::Summary> effectSummaries() const {
		return m_effectSummaries;
	}

protected:
	double computeFor(const QDate& date, bool& isValid) override {
		NOTICE() << "OracleTrend<" << overLastDays << ">computeFor " << date.toString();
		if (m_effectSummaries.contains(date)) {
			return m_effectSummaries[date].flow();
		}

		QDate dateAgo = date.addDays(-overLastDays);
		summaryMet->value(dateAgo);
		summaryMet->value(date);
		isValid = summaryMet->isValid(date) && summaryMet->isValid(dateAgo);
		if (!isValid) {
			WARN() << "summaryMet not valid";
			return 0.0;
		}
		SuperOracle::Summary sumarEnd = summaryMet->summaries()[date];
		SuperOracle::Summary sumarAgo = summaryMet->summaries()[dateAgo];

		// compute average
		m_effectSummaries[date] = sumarAgo.effectOf(sumarEnd, overLastDays);

		for (int i = 0; i < m_effectSummaries[date].dailyPerOracle.count(); ++i) {
			double effect = m_effectSummaries[date].dailyPerOracle[i];
			if (qAbs(effect) > 0.001) {
				INFO() << i
					   << " effect: " << effect
							 ;
			}
		}

		return m_effectSummaries[date].flow();
	}
private:
	OracleSummary* summaryMet;
	QMap<QDate, SuperOracle::Summary> m_effectSummaries;
};

#endif // USERMETRICS_H
