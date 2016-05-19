#ifndef FEATURECategory_H
#define FEATURECategory_H
#include "core_global.h"
#include "featureStatDistrib.h"

class OracleCategory : public OracleStatDistrib
{
public:
	OracleCategory(AccountFeature* pCreatingFeature)
		: OracleStatDistrib(pCreatingFeature)
	{}
	QString description() const override {
		if (m_args.m_bundle.count() == 0)
			return "";
		QString desc;
		if (m_args.m_kla > 0)
			desc += "income from ";
		else
			desc += "purchases at ";
		desc += m_args.m_bundle.last().name;
		desc += ". Those happen ~ %1 times a month.";
		return desc.arg(qRound(m_args.m_dayProba * 30));
	}
	friend class FeatureCategory;
};

class CORESHARED_EXPORT FeatureCategory : public FeatureStatDistrib
{
public:
	FeatureCategory()
		: FeatureStatDistrib("CatDistrib")
	{
	}
protected:
	qint64 distCalc(const Transaction& tr, const Transaction& modelTrans) const override {
		qint64 dist = tr.categoryHash.hash();
		if (!dist)
			return 999999999;
		dist -= modelTrans.categoryHash.hash();
		dist = qAbs(dist);
		dist *= 512;
		dist /= 131072;
		return dist;
	}

	bool passFilter(qint64 dist, const Transaction& trans) const override {
		return dist < Transaction::LIMIT_DIST_TRANS;
	}
	int minTransactionForBundle() const override { return 24; }
	Oracle* makeNewOracle() override {
		OracleCategory* pNewOr = new OracleCategory(this);
		pNewOr->m_args = m_localStaticArgs;
		return pNewOr;
	}
	void onJustApplied(TransactionBundle &allTrans, Puppy::Context& ioContext) override {
		Q_UNUSED(allTrans);
		Q_UNUSED(ioContext);
		localStaticArgs()->m_fitness *= 2 * localStaticArgs()->m_bundle.count();
		localStaticArgs()->m_fitness /= minTransactionForBundle();
	}
	void emitGraphics(Puppy::Context& ioContext) const override {
		if (ioContext.isPostTreatment)
			return;
		for (int i = 0; i < m_localStaticArgs.m_bundle.count(); ++i) {
			const Transaction& tr = m_localStaticArgs.m_bundle.trans(i);
			emit ioContext.m_pUser->botContext()->matchedTransaction(tr.time_t(), tr.amountDbl(), 16);
		}
	}

};

#endif // FEATURECategory_H
