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
	QJsonObject toJson() const override {
		QJsonObject ret = Oracle::toJson();
//		QJsonObject args;
//		m_args.intoJson(args);
//		args.remove("zlabels");
//		ret["args"] = args;
		ret["approxAmnt"] = toSignifDigit_2(m_args.m_bundle.averageAmount());
		return ret;
	}

protected:
	QVector<Transaction> revelation(QDate upToDate) override;
	double avgDaily() const override;

protected:
	struct Args : public FeatureArgs
	{
		void intoJson(QJsonObject& o_retObj) const override {
			FeatureArgs::intoJson(o_retObj);
			o_retObj.insert("hash", m_hash);
			o_retObj.insert("kla", m_kla);
			o_retObj.insert("proba", m_dayProba);
			o_retObj.insert("avgAmn", m_bundle.averageAmount());
			o_retObj.insert("avgSmt", m_bundle.avgSmart());
			o_retObj.insert("tot$", m_bundle.sumDollar());
			o_retObj.insert("numBund", m_bundle.count());
		}
		int m_hash = 0;
		double m_effect = 0;
		double m_kla = 0;
		double m_dayProba = 0.0;
	} m_args;
	friend class FeatureStatDistrib;
};

class FeatureStatDistrib : public AccountFeature
{
public:
	FeatureStatDistrib(const QString& featureName)
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
	void isolateBundledTransactions(bool isPostTreatment = false) override;
	double apply(TransactionBundle& allTrans, bool doLog = false) override;
	void emitGraphics(Puppy::Context& ioContext) const override;
	Oracle* makeNewOracle() override {
		OracleStatDistrib* pNewOr = new OracleStatDistrib(this);
		pNewOr->m_args = m_localStaticArgs;
		return pNewOr;
	}

	virtual bool passFilter(quint64 dist, const Transaction& trans) const = 0;
	virtual int minTransactionForBundle() const = 0;
	void computeNextDayProba();
	double maxDailyProbability() const override {
		return m_localStaticArgs.m_dayProba;
	}

protected:
	FeatureArgs* localStaticArgs() override { return &m_localStaticArgs; }
	OracleStatDistrib::Args m_localStaticArgs;
};

#endif // FEATURESTATDISTRIB_H
