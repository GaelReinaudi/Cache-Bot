#ifndef FEATURECrossCurrent_H
#define FEATURECrossCurrent_H
#include "featureOutlier.h"

class FeatureCrossCurrent : public FeatureOutlier
{
public:
	FeatureCrossCurrent()
		: FeatureOutlier("FeatureCrossCurrent")
	{ }

protected:
//	QJsonObject toJson(Puppy::Context& ioContext) override {
//		QJsonObject retObj = AccountFeature::toJson(ioContext);
//		m_localStaticArgs.intoJson(retObj);
//		return retObj;
//	}

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

#endif // FEATURECrossCurrent_H
