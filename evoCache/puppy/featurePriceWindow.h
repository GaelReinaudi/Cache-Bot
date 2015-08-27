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
		return true//dist < Transaction::LIMIT_DIST_TRANS * 1024 * 1024
				&& trans.amountInt() >= 1.10 * m_localStaticArgs.m_kla
				&& trans.amountInt() <= 0.90 * m_localStaticArgs.m_kla
				|| trans.amountInt() <= 1.10 * m_localStaticArgs.m_kla
				&& trans.amountInt() >= 0.90 * m_localStaticArgs.m_kla
				;
	}
};

#endif // FEATUREPRICEWINDOW_H
