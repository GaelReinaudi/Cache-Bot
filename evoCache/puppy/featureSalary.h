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
	void onJustApplied(TransactionBundle &allTrans, Puppy::Context& ioContext) override {
		FeatureMonthlyAmount::onJustApplied(allTrans, ioContext);
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
	void onJustApplied(TransactionBundle &allTrans, Puppy::Context& ioContext) override {
		FeatureBiWeeklyAmount::onJustApplied(allTrans, ioContext);
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

class FeatureHousing : public FeatureMonthlyAmount
{
public:
	FeatureHousing()
		: FeatureMonthlyAmount("Housing")
	{ }
protected:
	FeatureHousing(QString featureName)
		: FeatureMonthlyAmount(featureName)
	{ }
protected:
	void onJustApplied(TransactionBundle &allTrans, Puppy::Context& ioContext) override {
		FeatureMonthlyAmount::onJustApplied(allTrans, ioContext);
		if (m_fitness) {
			if (ioContext.flags & Puppy::Context::Housing1) {
			}
			else {
				ioContext.flags |= Puppy::Context::Housing1;
//				m_fitness *= -(m_localStaticArgs.m_kla);
			}
		}
		m_fitness *= 4;
	}
protected:
	qint64 distance(const Transaction *targ, const Transaction *trans) override {
		// if both are neg
		if (targ->amount() <= -100 && trans->amount() <= -100) {
			return targ->distanceWeighted<16*2, 512/2, 2*4>(*trans);
		}
		return 1<<20;
	}
};

#endif // FEATURESALARY_H
