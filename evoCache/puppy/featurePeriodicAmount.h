#ifndef FEATUREPERIODICAMOUNT_H
#define FEATUREPERIODICAMOUNT_H
#include "AccRegPrimits.h"
#include "oracle.h"

class OracleOneDayOfMonth : public Oracle
{
public:

protected:
	QVector<Transaction> revelation(QDate upToDate) override;
	double avgDaily() const override;

private:
	struct Args
	{
		void intoJson(QJsonObject& o_retObj) {
			o_retObj.insert("dayOfMonth", m_dayOfMonth);
			o_retObj.insert("consecutive", m_consecMonthBeforeMissed);
			o_retObj.insert("cons-missed", m_consecMissed);
			o_retObj.insert("labels", QJsonArray::fromStringList(m_bundle.uniqueNames()));
			o_retObj.insert("tot$", m_bundle.sumDollar());
			o_retObj.insert("numBund", m_bundle.count());
			o_retObj.insert("fitRerun", m_fitRerun);
			o_retObj.insert("amnt", unKindaLog(double(m_kla)));
			o_retObj.insert("kla", m_kla);
			o_retObj.insert("hash", m_hash);
		}
		TransactionBundle m_bundle;
		int m_dayOfMonth = 0;
		int m_dayOfMonth2 = 0;
		double m_kla = 0;
		int m_hash = 0;
		// characteristics
		int m_consecMonthBeforeMissed = 0;
		int m_consecMonth = 0;
		int m_consecMissed = 0;
		double m_fitRerun = 0;
	} m_args;
	friend class FeaturePeriodicAmount;
	friend class FeatureMonthlyAmount;
	friend class FeatureBiWeeklyAmount;
};

class FeaturePeriodicAmount : public AccountFeature
{
public:
	FeaturePeriodicAmount(QString featureName = "FixedIncome")
		: AccountFeature(6, featureName.toStdString())
	{ }
	~FeaturePeriodicAmount() { }

	virtual int approxSpacingPayment() = 0;
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
	int approxSpacingPayment() override {
		return 31;
	}
	void getArgs(Puppy::Context &ioContext) override {
		// if we are forcing a given hashed bundle
		int filterHashIndex = ioContext.filterHashIndex;
		if(filterHashIndex >= 0) {
			m_filterHash = ioContext.m_pUser->hashBundles().keys()[filterHashIndex];
			std::string nodeName = QString("h%1").arg(m_filterHash).toStdString();
			bool ok = tryReplaceArgumentNode(0, nodeName.c_str(), ioContext);
			if(!ok) {
				ERR() << "Could not replace the node with " << nodeName.c_str();
			}
//			nodeName = QString("%1").arg(ioContext.m_pUser->hashBundles()[m_filterHash]->klaAverage()).toStdString();
//			ok = tryReplaceArgumentNode(2, nodeName.c_str(), ioContext);
//			if(!ok) {
//				ERR() << "Could not replace the node with " << nodeName.c_str();
//			}
		}
		else {
			m_filterHash = -1;
		}

		double a = 0;
		int ind = -1;
		getArgument(++ind, &a, ioContext);
		m_localStaticArgs.m_hash = a;
		getArgument(++ind, &a, ioContext);
		m_localStaticArgs.m_dayOfMonth = a;
		getArgument(++ind, &a, ioContext);
		m_localStaticArgs.m_kla = a;
	}
	void cleanArgs() override {
		FeaturePeriodicAmount::cleanArgs();
		m_localStaticArgs.m_dayOfMonth = qBound(-14, m_localStaticArgs.m_dayOfMonth, 31);
		if (m_localStaticArgs.m_dayOfMonth == 0)
			++m_localStaticArgs.m_dayOfMonth;
	}

	QJsonObject toJson(Puppy::Context& ioContext) override {
		QJsonObject retObj = FeaturePeriodicAmount::toJson(ioContext);
		m_localStaticArgs.intoJson(retObj);
		return retObj;
	}

	void execute(void* outDatum, Puppy::Context& ioContext) override;
	double apply(TransactionBundle &allTrans, bool doLog = false);

	double billProbability() const {
		double proba = m_fitness;
		proba *= m_localStaticArgs.m_consecMonthBeforeMissed;
		proba /= 4 + 2 * m_localStaticArgs.m_consecMissed;
		return proba;
	}

	virtual QVector<Transaction> targetTransactions(QDate iniDate, QDate lastDate);

protected:
	OracleOneDayOfMonth::Args m_localStaticArgs;
	QVector<Transaction> m_targetTrans;
};

class FeatureBiWeeklyAmount : public FeatureMonthlyAmount
{
public:
	FeatureBiWeeklyAmount()
		: FeatureMonthlyAmount("BiWeeklyIncome")
	{ }
	int approxSpacingPayment() override { return 17; } // +2d: gives some room for late payment
	virtual void cleanArgs() override {
		FeatureMonthlyAmount::cleanArgs();
		m_localStaticArgs.m_dayOfMonth2 = m_localStaticArgs.m_dayOfMonth + 15;
		m_localStaticArgs.m_dayOfMonth2 %= 32;
		m_localStaticArgs.m_dayOfMonth2 = qBound(1, m_localStaticArgs.m_dayOfMonth2, 31);
		if (qAbs(m_localStaticArgs.m_dayOfMonth - m_localStaticArgs.m_dayOfMonth2) < 14) {
			ERR() << "m_dayOfMonth " << m_localStaticArgs.m_dayOfMonth << " " << m_localStaticArgs.m_dayOfMonth2;
		}
		FeatureMonthlyAmount::cleanArgs();
	}
	QJsonObject toJson(Puppy::Context& ioContext) override {
		QJsonObject retObj = FeatureMonthlyAmount::toJson(ioContext);
		retObj.insert("dayOfMonth2", m_localStaticArgs.m_dayOfMonth2);
		return retObj;
	}
	QVector<Transaction> targetTransactions(QDate iniDate, QDate lastDate) override;
};

#endif // FEATUREPERIODICAMOUNT_H
