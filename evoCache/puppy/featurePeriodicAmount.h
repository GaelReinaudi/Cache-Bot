#ifndef FEATUREPERIODICAMOUNT_H
#define FEATUREPERIODICAMOUNT_H
#include "AccRegPrimits.h"
#include "oracle.h"

class FeaturePeriodicAmount : public AccountFeature
{
public:
	FeaturePeriodicAmount(QString featureName = "FixedIncome")
		: AccountFeature(6, featureName.toStdString())
	{ }
	~FeaturePeriodicAmount() { }

	virtual int approxSpacingPayment() = 0;
};

class FeatureMonthlyAmount : public FeaturePeriodicAmount, public Oracle
{
public:
	FeatureMonthlyAmount()
		: FeaturePeriodicAmount("MonthlyAmount")
	{ }
protected:
	FeatureMonthlyAmount(QString featureName)
		: FeaturePeriodicAmount(featureName)
	{ }
protected:
	int approxSpacingPayment() override { return 31; }
	void getArgs(Puppy::Context &ioContext) override {
		// if we are forcing a given hashed bundle
		int filterHashIndex = ioContext.filterHashIndex;
		if(filterHashIndex >= 0) {
			m_filterHash = ioContext.m_pUser->hashBundles().keys()[filterHashIndex];
//			int avgKLA = ioContext.m_pUser->hashBundles()[m_filterHash]->averageKLA();
			std::string nodeName = QString("h%1").arg(m_filterHash).toStdString();
			bool ok = tryReplaceArgumentNode(2, nodeName.c_str(), ioContext);
//			std::string nodeKLA = QString("kla%1").arg(avgKLA).toStdString();
//			ok &= tryReplaceArgumentNode(3, nodeKLA.c_str(), ioContext);
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
		m_dayOfMonth = a;
		getArgument(++ind, &a, ioContext);
		m_kla = a;

		int bInd = -1;
		getArgument(++ind, &a, ioContext);
		m_b[++bInd] = a;
		getArgument(++ind, &a, ioContext);
		m_b[++bInd] = a;
		getArgument(++ind, &a, ioContext);
		m_b[++bInd] = a;
		getArgument(++ind, &a, ioContext);
		m_b[++bInd] = a;
	}
	void cleanArgs() override {
		FeaturePeriodicAmount::cleanArgs();
		m_dayOfMonth %= 31;
		++m_dayOfMonth;
	}

	QJsonObject toJson(Puppy::Context& ioContext) override {
		QJsonObject retObj = FeaturePeriodicAmount::toJson(ioContext);
		retObj.insert("dayOfMonth", m_dayOfMonth);
		retObj.insert("consecutive", m_consecMonthBeforeMissed);
		retObj.insert("cons-missed", m_consecMissed);

		if(m_bundle.count()) {
			QString str = QString::fromStdString(getName()) + " ("
				  + QString::number(m_bundle.count()) +  ") "
				  + " kl$ " + QString::number(double(m_kla) / KLA_MULTIPLICATOR)
				  + " / " + QString::number(kindaLog(m_bundle.sumDollar() / m_bundle.count()))
				  + " = " + QString::number(unKindaLog(double(m_kla) / KLA_MULTIPLICATOR))
				  + " / " + QString::number(m_bundle.sumDollar() / m_bundle.count());
			retObj.insert("info", str);
			str = QString("On the ") + QString::number(m_dayOfMonth) + "th, ";
			str += QString("hash: ") + QString::number(m_bundle.trans(0).nameHash.hash());
			retObj.insert("hash", m_bundle.trans(0).nameHash.hash());
//			retObj.insert("indH", m_bundle.trans(0).indexHash);
		}

		return retObj;
	}

	void execute(void* outDatum, Puppy::Context& ioContext) override;

	double billProbability() const {
		double proba = m_fitness;
		proba *= m_consecMonthBeforeMissed;
		proba /= 4 + 2 * m_consecMissed;
		return proba;
	}

	virtual QVector<Transaction> targetTransactions(QDate iniDate, QDate lastDate);
	QVector<Transaction> revelation(QDate upToDate) override {
		auto ret = targetTransactions(QDate::currentDate(), upToDate);
		LOG() << "FeatureMonthlyAmount::revelation " << ret.count() << endl;
		for (Transaction& tr : ret) {
			LOG() << "monthly " << tr.amountDbl() << " " << tr.date.toString() << endl;
		}
		return ret;
	}

protected:
	int m_dayOfMonth = 0;
	int m_kla = 0;
	int m_b[4];
	// characteristics
	int m_consecMonthBeforeMissed = 0;
	int m_consecMonth = 0;
	int m_consecMissed = 0;
};

class FeatureBiWeeklyAmount : public FeatureMonthlyAmount
{
public:
	FeatureBiWeeklyAmount()
		: FeatureMonthlyAmount("BiWeeklyIncome")
	{ }
	int approxSpacingPayment() override { return 17; } // +2d: gives some room for late payment
	virtual void cleanArgs() override {
		m_dayOfMonth2 = m_dayOfMonth+14;//(m_dayOfMonth / 32) % 31;
		++m_dayOfMonth2;
		FeatureMonthlyAmount::cleanArgs();
	}
	QJsonObject toJson(Puppy::Context& ioContext) override {
		QJsonObject retObj = FeatureMonthlyAmount::toJson(ioContext);
		retObj.insert("dayOfMonth2", m_dayOfMonth2);
		return retObj;
	}
	QVector<Transaction> targetTransactions(QDate iniDate, QDate lastDate) override;

protected:
	int m_dayOfMonth2 = 0;
};

#endif // FEATUREPERIODICAMOUNT_H
