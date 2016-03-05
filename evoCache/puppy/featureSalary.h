#ifndef FEATURESALARY_H
#define FEATURESALARY_H
#include "featurePeriodicAmount.h"

class FeatureMonthlySalary : public FeatureMonthlyAmount
{
public:
	FeatureMonthlySalary()
		: FeatureMonthlyAmount("MonthlySalary")
	{
		m_localStaticArgs.m_maxMissesAllowed = 1;
	}
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
		if (targ->amount() > 0 && trans->amount() > 0) {
			// if trans bellow target, probably not this
			if (targ->amount() > trans->amount() * 1.2)
				return targ->distanceWeighted<16, 512/2, 2*4>(*trans);
			else { // if above
				return targ->distanceWeighted<16*2, 512/2, 2*4*16>(*trans);
			}
		}
		return 1<<20;
	}
	Oracle* makeNewOracle() override {
		OracleOneDayOfMonth* pNewOr = new OracleOneDayOfMonth(this);
		pNewOr->m_args = m_localStaticArgs;
//		pNewOr->m_args.m_maxMissesAllowed = 1;
		return pNewOr;
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
