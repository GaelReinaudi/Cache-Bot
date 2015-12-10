#ifndef FEATUREOUTLIER_H
#define FEATUREOUTLIER_H
#include "AccRegPrimits.h"
#include "oracle.h"

class OracleOutlier : public Oracle
{
public:
	OracleOutlier(AccountFeature* pCreatingFeature)
		: Oracle(pCreatingFeature)
	{}

protected:
	QVector<Transaction> revelation(QDate upToDate) override {
		Q_UNUSED(upToDate);
		return QVector<Transaction>();
	}
	double avgDaily() const override {
		return m_args.m_dayProba * m_args.m_amount;
	}

private:
	struct Args : public FeatureArgs
	{
		void intoJson(QJsonObject& o_retObj) const override {
			FeatureArgs::intoJson(o_retObj);
			o_retObj.insert("proba", m_dayProba);
			o_retObj.insert("amount", m_amount);
			o_retObj.insert("eff128", m_effect);
		}
		double m_dayProba = 0.0;
		double m_amount = 0.0;
		char m_effect = 0;
	} m_args;
	friend class FeatureOutlier;
};

class FeatureOutlier : public AccountFeature
{
public:
	FeatureOutlier()
		: AccountFeature(3, "FeatureOutlier")
	{ }

protected:
	QJsonObject toJson(Puppy::Context& ioContext) override {
		QJsonObject retObj = AccountFeature::toJson(ioContext);
		m_localStaticArgs.intoJson(retObj);
		return retObj;
	}

protected:
	double apply(TransactionBundle& allTrans, bool isPostTreat, bool doLog) override;
	void emitGraphics(Puppy::Context& ioContext) const override;
	Oracle* makeNewOracle() {
		OracleOutlier* pNewOr = new OracleOutlier(this);
		pNewOr->m_args = m_localStaticArgs;
		return pNewOr;
	}

private:
	FeatureArgs* localStaticArgs() override { return &m_localStaticArgs; }
	OracleOutlier::Args m_localStaticArgs;
};

#endif // FEATUREOUTLIER_H
