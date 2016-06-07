#ifndef FEATURESTATDISTRIB_H
#define FEATURESTATDISTRIB_H
#include "AccRegPrimits.h"
#include "oracle.h"

class OracleStatDistrib : public Oracle
{
public:
	OracleStatDistrib(AccountFeature* pCreatingFeature)
		: Oracle(pCreatingFeature)
	{}
	QJsonObject toJson() const override {
		QJsonObject ret = Oracle::toJson();
		m_args.intoJson(ret);
		ret["approxAmnt"] = toSignifDigit_2(m_args.m_bundle.averageAmount());
//		double avg = m_args.m_bundle.averageAmount();
//		ret["avgAmnt"] = avg;
//		ret["stdDevAmnt"] = m_args.m_bundle.stdDevAmountSmart(avg);
		ret["dayProba"] = m_args.m_dayProba;
		ret["daily"] = m_args.avgDaily();//m_args.m_bundle.averageAmount() * m_args.m_dayProba;
		double fracCat = 0.0;
		ret.insert("mostCatId", m_args.m_bundle.mostCatId(&fracCat));
		ret.insert("mostCatFrac", fracCat);
		double daysToNext = -1.0;
		double lastAgo = -1.0;
		double savedFor = -1.0;
		if (m_args.m_bundle.count()) {
			daysToNext = m_args.m_bundle.daysToNextSmart();
			lastAgo = m_args.m_bundle.last().date.daysTo(Transaction::currentDay());
			double fracAvg = m_args.m_bundle.last().amountDbl() / m_args.m_bundle.avgSmart();
			lastAgo -= daysToNext * (fracAvg - 1.0);
			savedFor = lastAgo / daysToNext;
		}
		ret["daysToNext"] = daysToNext;
		ret["lastAgo"] = lastAgo;
		ret["savedFor"] = savedFor;
		QJsonArray transIds;
		for (int i = 0; i < m_args.m_bundle.count(); ++i) {
			const Transaction& tr = m_args.m_bundle.trans(i);
			transIds.append(tr.id);
		}
		ret["trans"] = transIds;
		return ret;
	}

protected:
	QVector<Transaction> revelation(QDate upToDate) override;
	double avgDaily(int limDayProba = 0) const override { return m_args.avgDaily(limDayProba); }

protected:
	struct Args : public FeatureArgs
	{
		void intoJson(QJsonObject& o_retObj) const override {
			FeatureArgs::intoJson(o_retObj);
			o_retObj.insert("hash", m_hash);
			o_retObj.insert("kla", m_kla);
			o_retObj.insert("proba", m_dayProba);
//			o_retObj.insert("avgAmn", m_bundle.averageAmount());
//			o_retObj.insert("avgSmt", m_bundle.avgSmart());
			o_retObj.insert("_total", m_bundle.sumDollar());
		}
		double avgDaily(int limDayProba = 0) const override
		{
			if (limDayProba && m_dayProba / (1 - m_dayProba) < 1.0 / limDayProba)
				return 0;
			return m_bundle.avgSmart() * m_dayProba / (1 - m_dayProba);
		}
		int m_hash = 0;
		double m_effect = 0;
		double m_kla = 0;
		double m_dayProba = 0.0;
		double m_klaFrom = 0.0;
		double m_klaTo = 0.0;
	} m_args;
	FeatureArgs* args() override { return &m_args; }
	friend class FeatureStatDistrib;
};

class FeatureStatDistrib : public AccountFeature
{
public:
	FeatureStatDistrib(const QString& featureName)
		: AccountFeature(3, featureName.toStdString())
	{
	}
	~FeatureStatDistrib() {}

protected:
	int getArgs(Puppy::Context &ioContext, int startAfter = -1) override;
	QJsonObject toJson(Puppy::Context& ioContext) override {
		QJsonObject retObj = AccountFeature::toJson(ioContext);
		m_localStaticArgs.intoJson(retObj);
		return retObj;
	}
	void isolateBundledTransactions(bool isPostTreatment = false) override;
	double apply(TransactionBundle& allTrans, bool isPostTreat, bool doLog) override;
	void emitGraphics(Puppy::Context& ioContext) const override;
	Oracle* makeNewOracle() override {
		OracleStatDistrib* pNewOr = new OracleStatDistrib(this);
		pNewOr->m_args = m_localStaticArgs;
		return pNewOr;
	}

	virtual int minTransactionForBundle() const = 0;
	void computeNextDayProba();
	double maxDailyProbability() override {
		return m_localStaticArgs.m_dayProba;
	}
	virtual qint64 distCalc(const Transaction& tr, const Transaction &modelTrans) const;

protected:
	FeatureArgs* localStaticArgs() override { return &m_localStaticArgs; }
	OracleStatDistrib::Args m_localStaticArgs;
};

#endif // FEATURESTATDISTRIB_H
