#ifndef FEATURELABELDISTRIB_H
#define FEATURELABELDISTRIB_H
#include "core_global.h"
#include "featureStatDistrib.h"

#define EFFECT_RANGE_WIDTH_RATIO 2.0

class OracleLabelDistrib : public OracleStatDistrib
{
public:
	OracleLabelDistrib(AccountFeature* pCreatingFeature)
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
	friend class FeatureLabelDistrib;
};

class CORESHARED_EXPORT FeatureLabelDistrib : public FeatureStatDistrib
{
public:
	FeatureLabelDistrib()
		: FeatureStatDistrib("LabelDistrib")
	{
		//WARN() << "creating instance FeatureLabelDistrib";
	}
protected:
	bool passFilter(qint64 dist, const Transaction& trans) const override {
		bool ok = FeatureStatDistrib::passFilter(dist, trans);
		ok &= dist < Transaction::LIMIT_DIST_TRANS / 4
				&& trans.klaEff() < 0
				&& double(trans.klaEff()) <=  1.0 + m_localStaticArgs.m_kla / EFFECT_RANGE_WIDTH_RATIO
				&& double(trans.klaEff()) >= -1.0 + m_localStaticArgs.m_kla * EFFECT_RANGE_WIDTH_RATIO;
		return ok;
	}
	int minTransactionForBundle() const override { return 24; }
	Oracle* makeNewOracle() override {
		OracleLabelDistrib* pNewOr = new OracleLabelDistrib(this);
		pNewOr->m_args = m_localStaticArgs;
		return pNewOr;
	}
	void onGeneration(int nGen, double progressGeneration, Puppy::Context &ioContext) override {
		FeatureStatDistrib::onGeneration(nGen, progressGeneration, ioContext);
		if (nGen <= 4) {
			// if we are pre-training
			int filterHashIndex = ioContext.filterHashIndex;
			if(filterHashIndex >= 0) {
				double effectForIndiv = 2.0+ 0* progressGeneration * 10.0;
				QString nodeName = QString("%1").arg(effectForIndiv);
				ioContext.getPrimitiveByName(nodeName);
				bool ok = tryReplaceArgumentNode(3, nodeName.toStdString().c_str(), ioContext);
				if(!ok) {
					ERR() << "Could not replace the node with " << nodeName;
				}
			}
		}
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
			emit ioContext.m_pUser->botContext()->matchedTransaction(tr.time_t(), tr.amountDbl(), 8);
		}
	}

};

#endif // FEATURELABELDISTRIB_H
