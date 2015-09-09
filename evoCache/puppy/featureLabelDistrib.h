#ifndef FEATURELABELDISTRIB_H
#define FEATURELABELDISTRIB_H
#include "featureStatDistrib.h"

#define EFFECT_RANGE_WIDTH_RATIO 2.0

class FeatureLabelDistrib : public FeatureStatDistrib
{
public:
	FeatureLabelDistrib()
		: FeatureStatDistrib("LabelDistrib")
	{
	}
protected:
	bool passFilter(quint64 dist, const Transaction& trans) const override {
		return dist < Transaction::LIMIT_DIST_TRANS
				&& trans.effect128 <=  1 + m_localStaticArgs.m_effect * EFFECT_RANGE_WIDTH_RATIO
				&& trans.effect128 >= -1 + m_localStaticArgs.m_effect / EFFECT_RANGE_WIDTH_RATIO;
	}
	int minTransactionForBundle() const override { return 16; }
};

#endif // FEATURELABELDISTRIB_H
