#ifndef FEATURESTATDISTRIB_H
#define FEATURESTATDISTRIB_H
#include "AccRegPrimits.h"
#include "oracle.h"

class OracleStatDistrib : public Oracle
{
public:
	OracleStatDistrib(AccountFeature* pCreatingFeature)
		: Oracle(pCreatingFeature)
	{}

protected:
	QVector<Transaction> revelation(QDate upToDate) override;
	double avgDaily() const override;

private:
	struct Args
	{
		void intoJson(QJsonObject& o_retObj) {
			o_retObj.insert("hash", m_hash);
			o_retObj.insert("effect", m_effect);
			o_retObj.insert("kla", m_kla);
			o_retObj.insert("proba", m_dayProba);
			o_retObj.insert("avgAmn", m_bundle.averageAmount());
			o_retObj.insert("avgSmt", m_bundle.avgSmart());
			o_retObj.insert("zlabels", QJsonArray::fromStringList(m_bundle.uniqueNames()));
			o_retObj.insert("tot$", m_bundle.sumDollar());
			o_retObj.insert("numBund", m_bundle.count());
		}
		TransactionBundle m_bundle;
		int m_hash = 0;
		double m_effect = 0;
		double m_kla = 0;
		double m_dayProba = 0.0;
	} m_args;
	friend class FeatureStatDistrib;
	friend class FeaturePriceWindow;
};

class FeatureStatDistrib : public AccountFeature
{
public:
	FeatureStatDistrib(QString featureName)
		: AccountFeature(3, featureName.toStdString())
	{
	}
	~FeatureStatDistrib() {}

protected:
	void getArgs(Puppy::Context &ioContext) override;
	QJsonObject toJson(Puppy::Context& ioContext) override {
		QJsonObject retObj = AccountFeature::toJson(ioContext);
		m_localStaticArgs.intoJson(retObj);
		return retObj;
	}
	void execute(void* outDatum, Puppy::Context& ioContext) override;
	void isolateBundledTransactions(bool isPostTreatment = false) const override;
	double apply(TransactionBundle& allTrans, bool doLog = false) override;

	virtual bool passFilter(quint64 dist, const Transaction& trans) const = 0;
	virtual int minTransactionForBundle() const = 0;
	void computeNextDayProba();

protected:
	OracleStatDistrib::Args m_localStaticArgs;
};

#endif // FEATURESTATDISTRIB_H
