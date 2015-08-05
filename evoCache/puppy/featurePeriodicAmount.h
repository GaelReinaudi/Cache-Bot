#ifndef FEATUREPERIODICAMOUNT_H
#define FEATUREPERIODICAMOUNT_H
#include "AccRegPrimits.h"
#include "oracle.h"

class OracleOneDayOfMonth : public Oracle
{
public:

protected:
	QVector<Transaction> revelation(QDate upToDate) override;

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

//			if(m_bundle.count()) {
//				QString str = QString::fromStdString(getName()) + " ("
//					  + QString::number(m_bundle.count()) +  ") "
//					  + " kl$ " + QString::number(double(m_kla) / KLA_MULTIPLICATOR)
//					  + " / " + QString::number(kindaLog(m_bundle.sumDollar() / m_bundle.count()))
//					  + " = " + QString::number(unKindaLog(double(m_kla) / KLA_MULTIPLICATOR))
//					  + " / " + QString::number(m_bundle.sumDollar() / m_bundle.count());
//				o_retObj.insert("info", str);
//				str = QString("On the ") + QString::number(m_dayOfMonth) + "th, ";
//				str += QString("hash: ") + QString::number(m_bundle.trans(0).nameHash.hash());
//				o_retObj.insert("hash", m_bundle.trans(0).nameHash.hash());
//			}
		}
		TransactionBundle m_bundle;
		int m_dayOfMonth = 0;
		int m_dayOfMonth2 = 0;
		int m_kla = 0;
		int m_b[4];
		// characteristics
		int m_consecMonthBeforeMissed = 0;
		int m_consecMonth = 0;
		int m_consecMissed = 0;
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
	static QVector<Transaction> BlankTransactionsForDayOfMonth(QDate iniDate, QDate lastDate, int dayOfMonth);

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
			bool ok = tryReplaceArgumentNode(2, nodeName.c_str(), ioContext);
			if(!ok) {
				LOG() << "Could not replace the node with " << nodeName.c_str() << endl;
			}
		}
		else {
			m_filterHash = -1;
		}

		double a = 0;
		int ind = -1;
		getArgument(++ind, &a, ioContext);
		m_localStaticArgs.m_dayOfMonth = a;
		getArgument(++ind, &a, ioContext);
		m_localStaticArgs.m_kla = a;

		int bInd = -1;
		getArgument(++ind, &a, ioContext);
		m_localStaticArgs.m_b[++bInd] = a;
		getArgument(++ind, &a, ioContext);
		m_localStaticArgs.m_b[++bInd] = a;
		getArgument(++ind, &a, ioContext);
		m_localStaticArgs.m_b[++bInd] = a;
		getArgument(++ind, &a, ioContext);
		m_localStaticArgs.m_b[++bInd] = a;
	}
	void cleanArgs() override {
		FeaturePeriodicAmount::cleanArgs();
		m_localStaticArgs.m_dayOfMonth %= 31;
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
		m_localStaticArgs.m_dayOfMonth2 = m_localStaticArgs.m_dayOfMonth+14;//(m_dayOfMonth / 32) % 31;
		++m_localStaticArgs.m_dayOfMonth2;
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
