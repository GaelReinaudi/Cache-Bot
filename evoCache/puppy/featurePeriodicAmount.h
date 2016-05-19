#ifndef FEATUREPERIODICAMOUNT_H
#define FEATUREPERIODICAMOUNT_H
#include "AccRegPrimits.h"
#include "oracle.h"

class OracleOneDayOfMonth : public Oracle
{
public:
	OracleOneDayOfMonth(AccountFeature* pCreatingFeature)
		: Oracle(pCreatingFeature)
	{}
	QJsonObject toJson() const override;

protected:
	QVector<Transaction> revelation(QDate upToDate) override;
	double avgDaily(int limDayProba = 0) const override { return m_args.avgDaily(limDayProba); }

private:
	struct Args : public FeatureArgs
	{
		void intoJson(QJsonObject& o_retObj) const override {
			FeatureArgs::intoJson(o_retObj);
			o_retObj.insert("dayOfMonth", m_dayOfMonth);
			o_retObj.insert("consecutive", m_consecMonthBeforeMissed);
			o_retObj.insert("consMissed", m_consecMissed);
			o_retObj.insert("lastDateMatched", m_lastDateMatched.toString("yyyy-MM-dd"));
			o_retObj.insert("consPrevMissed", m_prevMissed);
			o_retObj.insert("_total", m_bundle.sumDollar());
//			o_retObj.insert("numBund", m_bundle.count());
			o_retObj.insert("fitRerun", m_fitRerun);
			o_retObj.insert("featArgAmnt", unKindaLog(double(m_kla)));
//			o_retObj.insert("avgSmt", m_bundle.avgSmart());
			o_retObj.insert("kla", m_kla);
			o_retObj.insert("hash", m_hash);
		}
		double computeProba() const override {
			if (m_bundle.count() < 2)
				return 0.0;
			if (m_prevMissed >= m_consecMonthBeforeMissed)
				return 0.0;
			int often = 0;
			if (m_consecMonthBeforeMissed > 2 * m_consecMissed)
				often = 1;
			if (m_consecMissed > m_maxMissesAllowed + often)
				return 0.0;
			// if one that seems new but none before
			if (m_consecMonthBeforeMissed <= -m_consecMissed)
				return 0.0;
			double proba = 1.0;
			proba *= qMax(1, m_consecMonthBeforeMissed);
			proba /= 4 + 2 * m_consecMissed;
			return proba;
		}
		double avgDaily(int limDayProba = 0) const override {
			double avgMonth = 0.0;
			if (computeProba() > 0.0)
			{
				avgMonth = m_bundle.avgSmart();
				if (m_dayOfMonth2) {
					avgMonth *= 2;
				}
			}
			return avgMonth * (12.0 / 365.25);
		}
		int m_dayOfMonth = 0;
		int m_dayOfMonth2 = 0;
		double m_kla = 0;
		int m_hash = 0;
		// characteristics
		int m_consecMonthBeforeMissed = 0;
		int m_consecMonth = 0;
		int m_consecMissed = 999;
		QDate m_lastDateMatched;
		int m_prevMissed = 0;
		double m_fitRerun = 0;
		int m_maxMissesAllowed = 0;
	} m_args;
	FeatureArgs* args() override { return &m_args; }

	QString description() const {
		QString desc;
		if (m_args.m_kla > 0)
			desc += "income ";
		else
			desc += "bill ";
		desc += "in the range ~%1";
		desc += ", on the %2";
		if (m_args.m_dayOfMonth2)
			desc += " & %3";
		desc += " of the month.";
		if (m_args.m_dayOfMonth2)
			return desc.arg(qAbs(toSignifDigit_2(m_args.m_bundle.averageAmount())))
					.arg(m_args.m_dayOfMonth)
					.arg(m_args.m_dayOfMonth2);
		return desc.arg(qAbs(toSignifDigit_2(m_args.m_bundle.averageAmount())))
				.arg(m_args.m_dayOfMonth);
	}
	bool checkMightBeRent() {
		if (User::declaredRent > 2 && -m_args.m_kla > kindaLog(User::declaredRent*30) - 0.2) {
			m_args.m_maxMissesAllowed = 1;
			return true;
		}
		if (User::declaredRent == 0 && -m_args.m_kla > 2.3) {
			m_args.m_maxMissesAllowed = 1;
			return true;
		}
		return false;
	}

	friend class FeaturePeriodicAmount;
	friend class FeatureMonthlyAmount;
	friend class FeatureBiWeeklyAmount;
	friend class FeatureMonthlySalary;
	friend class FeatureBiWeeklySalaryReally;
	friend class OracleEveryOtherWeek;
};

class OracleEveryOtherWeek : public OracleOneDayOfMonth
{
public:
	OracleEveryOtherWeek(AccountFeature* pCreatingFeature)
		: OracleOneDayOfMonth(pCreatingFeature)
	{}
protected:
	QVector<Transaction> revelation(QDate upToDate) override;
};

class FeaturePeriodicAmount : public AccountFeature
{
public:
	FeaturePeriodicAmount(QString featureName = "FixedIncome")
		: AccountFeature(3, featureName.toStdString())
	{ }
	~FeaturePeriodicAmount() { }

	virtual int approxSpacingPayment() const = 0;
	int isPeriodic() const override { return approxSpacingPayment(); }
protected:
	virtual qint64 distance(const Transaction *targ, const Transaction *trans);
	virtual int onMissedTarget(Transaction *targ, const Transaction *last);
};

class FeatureMonthlyAmount : public FeaturePeriodicAmount
{
public:
	FeatureMonthlyAmount()
		: FeaturePeriodicAmount("MonthlyAmount")
	{ }
	static QVector<Transaction> BlankTransactionsForDayOfMonth(QDate iniDate, QDate lastDate, int dayOfMonth, std::function<Transaction(void)> lambda = [](){return Transaction();});
	static QVector<Transaction> BlankTransactionsForJdOffset(QDate iniDate, QDate lastDate, int jdOffset, std::function<Transaction(void)> lambda = [](){return Transaction();});

protected:
	FeatureMonthlyAmount(QString featureName)
		: FeaturePeriodicAmount(featureName)
	{ }
protected:
	int approxSpacingPayment() const override {
		return 30;
	}
	void getArgs(Puppy::Context &ioContext) override;
	void cleanArgs() override {
		FeaturePeriodicAmount::cleanArgs();
		while (qAbs(m_localStaticArgs.m_kla) > 8.0)
			m_localStaticArgs.m_kla /= 10.0;
		m_localStaticArgs.m_dayOfMonth = qBound(-14, m_localStaticArgs.m_dayOfMonth, 32);
		if (m_localStaticArgs.m_dayOfMonth == 0)
			++m_localStaticArgs.m_dayOfMonth;
	}

	QJsonObject toJson(Puppy::Context& ioContext) override {
		QJsonObject retObj = FeaturePeriodicAmount::toJson(ioContext);
		m_localStaticArgs.intoJson(retObj);
		return retObj;
	}

	bool passFilter(qint64 dist, const Transaction& trans) const override {
		Q_UNUSED(dist);
		bool ok = true;
		ok &= !(trans.userFlag & (Transaction::noRecur | Transaction::Reimbursed));
		return ok;
	}
	double apply(TransactionBundle& allTrans, bool isPostTreat, bool doLog) override;
	void onJustApplied(TransactionBundle &allTrans, Puppy::Context& ioContext) override;
	void emitGraphics(Puppy::Context& ioContext) const override;
	Oracle* makeNewOracle() override {
		OracleOneDayOfMonth* pNewOr = new OracleOneDayOfMonth(this);
		pNewOr->m_args = m_localStaticArgs;
		pNewOr->checkMightBeRent();
		return pNewOr;
	}

	double maxDailyProbability() const override {
		return m_localStaticArgs.computeProba();
	}

	virtual QVector<Transaction> targetTransactions(QDate iniDate, QDate lastDate);

protected:
	FeatureArgs* localStaticArgs() override { return &m_localStaticArgs; }
	OracleOneDayOfMonth::Args m_localStaticArgs;
	QVector<Transaction> m_targetTrans;
};

class FeatureBiWeeklyAmount : public FeatureMonthlyAmount
{
public:
	FeatureBiWeeklyAmount()
		: FeatureMonthlyAmount("BiWeeklyAmount")
	{ }
protected:
	FeatureBiWeeklyAmount(QString featureName)
		: FeatureMonthlyAmount(featureName)
	{ }
	int approxSpacingPayment() const override { return 15; } // +2d: gives some room for late payment
	virtual void cleanArgs() override {
		FeatureMonthlyAmount::cleanArgs();
		m_localStaticArgs.m_dayOfMonth2 = m_localStaticArgs.m_dayOfMonth + 15;
		m_localStaticArgs.m_dayOfMonth2 %= 32;
		m_localStaticArgs.m_dayOfMonth2 = qBound(1, m_localStaticArgs.m_dayOfMonth2, 31);
		if (qAbs(m_localStaticArgs.m_dayOfMonth - m_localStaticArgs.m_dayOfMonth2) < 14) {
			ERR() << "m_dayOfMonth " << m_localStaticArgs.m_dayOfMonth << " " << m_localStaticArgs.m_dayOfMonth2;
		}
	}
	QJsonObject toJson(Puppy::Context& ioContext) override {
		QJsonObject retObj = FeatureMonthlyAmount::toJson(ioContext);
		retObj.insert("dayOfMonth2", m_localStaticArgs.m_dayOfMonth2);
		return retObj;
	}
	QVector<Transaction> targetTransactions(QDate iniDate, QDate lastDate) override;
};

#endif // FEATUREPERIODICAMOUNT_H
