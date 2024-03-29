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
		localStaticArgs()->m_fitness *= 1 + m_localStaticArgs.m_bundle.flagsCount(Transaction::UserInputFlag::yesIncome);
	}
protected:
	bool passFilter(qint64 dist, const Transaction& trans) const override {
		Q_UNUSED(dist);
		bool ok = FeatureMonthlyAmount::passFilter(dist, trans);
		ok &= !(trans.userFlag & Transaction::UserInputFlag::noIncome);
		return ok;
	}
	qint64 distance(const Transaction *targ, const Transaction *trans) override {
		if (targ->amount() > 0 && trans->amount() > 0) {
			// if trans bellow target, probably not this
			if (targ->amount() > trans->amount() * 1.2)
				return targ->distanceWeighted<16, 512/2, 2*4>(*trans);
//			else if (trans->userFlag & Transaction::UserInputFlag::yesIncome)
//				return targ->distanceWeighted<16*2*2, 512, 4*4*16>(*trans);
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
	FeatureBiWeeklySalary(QString featureName)
		: FeatureBiWeeklyAmount(featureName)
	{ }
protected:
	bool passFilter(qint64 dist, const Transaction& trans) const override {
		Q_UNUSED(dist);
		bool ok = FeatureBiWeeklyAmount::passFilter(dist, trans);
		ok &= !(trans.userFlag & Transaction::UserInputFlag::noIncome);
		return ok;
	}
	void onJustApplied(TransactionBundle &allTrans, Puppy::Context& ioContext) override {
		FeatureBiWeeklyAmount::onJustApplied(allTrans, ioContext);
		localStaticArgs()->m_fitness *= 1 + m_localStaticArgs.m_bundle.flagsCount(Transaction::UserInputFlag::yesIncome);
	}
	qint64 distance(const Transaction *targ, const Transaction *trans) override {
		if (targ->amount() > 0 && trans->amount() > 0) {
			// if trans bellow target, probably not this
			if (targ->amount() > trans->amount() * 1.2)
				return targ->distanceWeighted<16, 512/2, 2*4>(*trans);
//			else if (trans->userFlag & Transaction::UserInputFlag::yesIncome)
//				return targ->distanceWeighted<16*2*2, 512, 4*4*16>(*trans);
			else { // if above
				return targ->distanceWeighted<16*2, 512/2, 2*4>(*trans);
			}
		}
		return 1<<20;
	}
};

class FeatureBiWeeklySalaryReally : public FeatureBiWeeklySalary
{
public:
	FeatureBiWeeklySalaryReally()
		: FeatureBiWeeklySalary("BiWeeklySalaryReally")
	{ }
	virtual void cleanArgs() override {
		FeatureBiWeeklySalary::cleanArgs();
		m_localStaticArgs.m_dayOfMonth = qAbs(m_localStaticArgs.m_dayOfMonth);
		m_localStaticArgs.m_dayOfMonth %= 13;
//		m_localStaticArgs.m_dayOfMonth = qBound(0, m_localStaticArgs.m_dayOfMonth2, 13);
		m_localStaticArgs.m_dayOfMonth2 = m_localStaticArgs.m_dayOfMonth + 14;
	}
	QJsonObject toJson(Puppy::Context& ioContext) override {
		QJsonObject retObj = FeatureBiWeeklySalary::toJson(ioContext);
		retObj.insert("jdOffset", m_localStaticArgs.m_dayOfMonth);
		return retObj;
	}
	Oracle* makeNewOracle() override {
		OracleEveryOtherWeek* pNewOr = new OracleEveryOtherWeek(this);
		pNewOr->m_args = m_localStaticArgs;
		pNewOr->checkMightBeRent();
		return pNewOr;
	}
	QVector<Transaction> targetTransactions(QDate iniDate, QDate lastDate) override;

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
		if (localStaticArgs()->m_fitness > 0) {
			if (ioContext.flags & Puppy::Context::Housing1) {
			}
			else {
				ioContext.flags |= Puppy::Context::Housing1;
//				localStaticArgs()->m_fitness *= -(m_localStaticArgs.m_kla);
			}
		}
		else
			return;
		localStaticArgs()->m_fitness *= 8 * !!(localStaticArgs()->m_bundle.flagsOR() & (Transaction::UserInputFlag::yesRecur | Transaction::UserInputFlag::yesHousing));
		localStaticArgs()->m_fitness *= 1 - qAbs(kindaLog(User::declaredRent*30) + m_localStaticArgs.m_bundle.klaAverage());
	}
protected:
	bool passFilter(qint64 dist, const Transaction& trans) const override {
		Q_UNUSED(dist);
		bool ok = FeatureMonthlyAmount::passFilter(dist, trans);
		ok &= !(trans.userFlag & Transaction::UserInputFlag::noHousing);
		return ok;
	}
	qint64 distance(const Transaction *targ, const Transaction *trans) override {
		// if both are neg
		if (targ->amount() <= -300 && trans->amount() <= -300) {
//			if (qAbs(targ->date.daysTo(Transaction::currentDay()))
			return targ->distanceWeighted<16*2, 512/2, 2*4>(*trans);
		}
		return 1<<20;
	}
	int onMissedTarget(Transaction *targ, const Transaction *last) override {
		if (targ->flags & Transaction::MovedLate)
			return 0;
		if (last && !(last->userFlag & Transaction::UserInputFlag::yesNext))
			return 0;
		if (last && (last->userFlag & Transaction::UserInputFlag::noNext))
			return 0;
		QDate newDate = targ->date.addDays(approxSpacingPayment() * 2 / 3);
		if (newDate > Transaction::currentDay())
			return 0;
		targ->date = newDate;
		targ->flags |= Transaction::MovedLate;
		return 1;
	}
};

#endif // FEATURESALARY_H
