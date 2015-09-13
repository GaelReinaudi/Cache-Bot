#ifndef FEATUREPRICEWINDOW_H
#define FEATUREPRICEWINDOW_H
#include "featureStatDistrib.h"

class OraclePriceWindow : public OracleStatDistrib
{
public:
	OraclePriceWindow(AccountFeature* pCreatingFeature)
		: OracleStatDistrib(pCreatingFeature)
	{}
	QString description() const {
		QString desc;
		if (m_args.m_kla > 0)
			desc += "income ";
		else
			desc += "expense ";
		desc += "in the range ~%1";
		desc += ". Those happen ~ %2 times a month.";
		return desc.arg(unKindaLog(qAbs(m_args.m_kla)), 0, 'f', 2)
				.arg(qRound(m_args.m_dayProba * 32));
	}
	friend class FeaturePriceWindow;
};

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

	Oracle* makeNewOracle() override {
		OraclePriceWindow* pNewOr = new OraclePriceWindow(this);
		pNewOr->m_args = m_localStaticArgs;
		return pNewOr;
	}
};

#endif // FEATUREPRICEWINDOW_H
