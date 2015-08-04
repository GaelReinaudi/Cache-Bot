#ifndef FEATURESTATDISTRIB_H
#define FEATURESTATDISTRIB_H
#include "AccRegPrimits.h"
#include "oracle.h"

class OracleStatDistrib : public Oracle
{
public:

protected:
//	QJsonObject toJson(Puppy::Context& ioContext) override {
//		QJsonObject retObj = AccountFeature::toJson(ioContext);
//		retObj.insert("hash", m_hash);
//		retObj.insert("days", m_daysBundle);
//		retObj.insert("proba", m_dayProba);
//		retObj.insert("avgAmn", m_bundle.count() ? m_bundle.sumDollar() / m_bundle.count() : 0.0);
//		return retObj;
//	}
	QVector<Transaction> revelation(QDate upToDate) override;

private:
	struct Args
	{
		TransactionBundle m_bundle;
		int m_hash = 0;
		double m_dayProba = 0.0;
		double m_daysBundle = 0.0;
	} m_args;
	friend class FeatureStatDistrib;
};

class FeatureStatDistrib : public AccountFeature
{
public:
	FeatureStatDistrib(QString featureName = "StatDistrib")
		: AccountFeature(6, featureName.toStdString())
	{
	}
	~FeatureStatDistrib() {}

protected:
	void getArgs(Puppy::Context &ioContext) override;
	QJsonObject toJson(Puppy::Context& ioContext) override {
		QJsonObject retObj = AccountFeature::toJson(ioContext);
		retObj.insert("hash", m_localStaticArgs.m_hash);
		retObj.insert("days", m_localStaticArgs.m_daysBundle);
		retObj.insert("proba", m_localStaticArgs.m_dayProba);
		retObj.insert("avgAmn", m_localStaticArgs.m_bundle.count() ? m_localStaticArgs.m_bundle.sumDollar() / m_localStaticArgs.m_bundle.count() : 0.0);
		retObj.insert("labels", QJsonArray::fromStringList(m_localStaticArgs.m_bundle.uniqueNames()));
		retObj.insert("tot$", m_localStaticArgs.m_bundle.sumDollar());
		retObj.insert("numBund", m_localStaticArgs.m_bundle.count());
		return retObj;
	}
	void execute(void* outDatum, Puppy::Context& ioContext) override;

private:
	OracleStatDistrib::Args m_localStaticArgs;
};

#endif // FEATURESTATDISTRIB_H
