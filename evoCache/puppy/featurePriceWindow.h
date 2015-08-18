#ifndef FEATUREPRICEWINDOW_H
#define FEATUREPRICEWINDOW_H
#include "featureStatDistrib.h"

class FeaturePriceWindow : public FeatureStatDistrib
{
public:
	FeaturePriceWindow()
		: FeatureStatDistrib("PriceWindow")
	{
	}
protected:
	bool passFilter(quint64 dist, const Transaction& trans) const override {
		Q_UNUSED(dist);
		return dist < Transaction::LIMIT_DIST_TRANS * 1024 * 1024
				&& kindaLog(trans.amountDbl()) >= 0.1 * 1.10 * double(m_localStaticArgs.m_effect)
				&& kindaLog(trans.amountDbl()) <= 0.1 * 0.90 * double(m_localStaticArgs.m_effect);
	}
};

#endif // FEATUREPRICEWINDOW_H
