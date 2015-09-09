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
		return (trans.kla() >= 1.04 * m_localStaticArgs.m_kla
				&& trans.kla() <= 0.95 * m_localStaticArgs.m_kla)
				||
				(trans.kla() <= 1.14 * m_localStaticArgs.m_kla
				&& trans.kla() >= 0.85 * m_localStaticArgs.m_kla);
	}
	int minTransactionForBundle() const override { return 2; }

};

#endif // FEATUREPRICEWINDOW_H
