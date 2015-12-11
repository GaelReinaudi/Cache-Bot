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
		return desc.arg(qAbs(toSignifDigit_2(m_args.m_bundle.averageAmount())))
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
	bool passFilter(qint64 dist, const Transaction& trans) const override {
		Q_UNUSED(dist);
		if (m_localStaticArgs.m_kla < 0)
			return trans.kla() <= m_localStaticArgs.m_kla + 0.2
					&& trans.kla() >= m_localStaticArgs.m_kla - 0.2;
		else
			return trans.kla() <= m_localStaticArgs.m_kla + 0.4
					&& trans.kla() >= m_localStaticArgs.m_kla - 0.4;
	}
	int minTransactionForBundle() const override { return 4; }

	Oracle* makeNewOracle() override {
		OraclePriceWindow* pNewOr = new OraclePriceWindow(this);
		pNewOr->m_args = m_localStaticArgs;
		return pNewOr;
	}
	void onJustApplied(TransactionBundle &allTrans, bool doLog) override {
		if (BotContext::JSON_ARGS["PriceWindow"].toString().trimmed() != "enabled") {
			m_fitness -= 100.0;
		}
	}
};

#endif // FEATUREPRICEWINDOW_H
