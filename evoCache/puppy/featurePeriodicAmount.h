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
	QJsonObject toJson() const override {
		QJsonObject ret = Oracle::toJson();
		ret["approxAmnt"] = toSignifDigit_2(m_args.m_bundle.averageAmount());
		ret["day1"] = (m_args.m_dayOfMonth + 31) % 31;
		ret["day2"] = (m_args.m_dayOfMonth2 + 31) % 31;
		return ret;
	}

protected:
	QVector<Transaction> revelation(QDate upToDate) override;
	double avgDaily() const override;

private:
	struct Args : public FeatureArgs
	{
		void intoJson(QJsonObject& o_retObj) const override {
			FeatureArgs::intoJson(o_retObj);
			o_retObj.insert("dayOfMonth", m_dayOfMonth);
			o_retObj.insert("consecutive", m_consecMonthBeforeMissed);
			o_retObj.insert("cons-missed", m_consecMissed);
			o_retObj.insert("tot$", m_bundle.sumDollar());
			o_retObj.insert("numBund", m_bundle.count());
			o_retObj.insert("fitRerun", m_fitRerun);
			o_retObj.insert("amnt", unKindaLog(double(m_kla)));
			o_retObj.insert("amntS", m_bundle.avgSmart());
			o_retObj.insert("kla", m_kla);
			o_retObj.insert("hash", m_hash);
		}
		int m_dayOfMonth = 0;
		int m_dayOfMonth2 = 0;
		double m_kla = 0;
		int m_hash = 0;
		// characteristics
		int m_consecMonthBeforeMissed = 0;
		int m_consecMonth = 0;
		int m_consecMissed = 999;
		int m_prevMissed = 0;
		double m_fitRerun = 0;
	} m_args;
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
		return desc.arg(qAbs(toSignifDigit_2(m_args.m_bundle.averageAmount())))
				.arg(m_args.m_dayOfMonth)
				.arg(m_args.m_dayOfMonth2);
	}
	friend class FeaturePeriodicAmount;
	friend class FeatureMonthlyAmount;
	friend class FeatureBiWeeklyAmount;
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
	static double computeProba(OracleOneDayOfMonth::Args args) {
		if (args.m_consecMissed > 0)
			return 0.0;
		// if one that seems new but none before
		if (args.m_consecMonthBeforeMissed <= -args.m_consecMissed)
			return 0.0;
		double proba = 1.0;
		proba *= qMax(1, args.m_consecMonthBeforeMissed);
		proba /= 4 + 2 * args.m_consecMissed;
		return proba;
	}
};

class FeatureMonthlyAmount : public FeaturePeriodicAmount
{
public:
	FeatureMonthlyAmount()
		: FeaturePeriodicAmount("MonthlyAmount")
	{ }
	static QVector<Transaction> BlankTransactionsForDayOfMonth(QDate iniDate, QDate lastDate, int dayOfMonth, std::function<Transaction(void)> lambda = [](){return Transaction();});

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

	double apply(TransactionBundle& allTrans, bool doLog = false) override;
	void onJustApplied(TransactionBundle &allTrans, bool doLog) override;
	void emitGraphics(Puppy::Context& ioContext) const override;
	Oracle* makeNewOracle() override {
		OracleOneDayOfMonth* pNewOr = new OracleOneDayOfMonth(this);
		pNewOr->m_args = m_localStaticArgs;
		return pNewOr;
	}

	double maxDailyProbability() const override {
		return FeaturePeriodicAmount::computeProba(m_localStaticArgs);
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
