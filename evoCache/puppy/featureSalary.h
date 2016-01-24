#ifndef FEATURESALARY_H
#define FEATURESALARY_H
#include "featurePeriodicAmount.h"

class FeatureMonthlySalary : public FeatureMonthlyAmount
{
public:
	FeatureMonthlySalary()
		: FeatureMonthlyAmount("MonthlySalary")
	{ }
protected:
	FeatureMonthlySalary(QString featureName)
		: FeatureMonthlyAmount(featureName)
	{ }
protected:
	void onJustApplied(TransactionBundle &allTrans, bool doLog) override {
		FeatureMonthlyAmount::onJustApplied(allTrans, doLog);
		m_fitness *= 4;
	}
protected:
	qint64 distance(const Transaction *targ, const Transaction *trans) override {
		// if both are positive
		if (targ->amount() > 0 && trans->amount() > 0) {
			return targ->distanceWeighted<16*2, 512/2, 2*4>(*trans);
		}
		return 1<<20;
	}
};

class FeatureBiWeeklySalary : public FeatureBiWeeklyAmount
{
public:
	FeatureBiWeeklySalary()
		: FeatureBiWeeklyAmount("BiWeeklySalary")
	{ }
protected:
	void onJustApplied(TransactionBundle &allTrans, bool doLog) override {
		FeatureBiWeeklyAmount::onJustApplied(allTrans, doLog);
		m_fitness *= 4;
	}
	qint64 distance(const Transaction *targ, const Transaction *trans) override {
		// if both are positive
		if (targ->amount() > 0 && trans->amount() > 0) {
			return targ->distanceWeighted<16*2, 512/2, 2*4>(*trans);
		}
		return 1<<20;
	}
};

#endif // FEATURESALARY_H
