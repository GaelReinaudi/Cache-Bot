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
		return (trans.kla() >= 1.10 * m_localStaticArgs.m_kla
				&& trans.kla() <= 0.90 * m_localStaticArgs.m_kla)
				||
				(trans.kla() <= 1.50 * m_localStaticArgs.m_kla
				&& trans.kla() >= 0.50 * m_localStaticArgs.m_kla);
	}
	int minTransactionForBundle() const override { return 2; }

};

#endif // FEATUREPRICEWINDOW_H
