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
				.arg(qRound(m_args.m_dayProba * 30));
	}
	friend class FeaturePriceWindow;
};

class FeaturePriceWindow : public FeatureStatDistrib
{
public:
	FeaturePriceWindow()
		: FeatureStatDistrib("PriceWindow")
	{
		if (!s_priceWindowDivisionKLA.count())
			initDivisions();
	}
	FeaturePriceWindow(const QString& featureName)
		: FeatureStatDistrib(featureName)
	{
		if (!s_priceWindowDivisionKLA.count())
			initDivisions();
	}

protected:
	void getArgs(Puppy::Context &ioContext) override {
		FeatureStatDistrib::getArgs(ioContext);
		m_localStaticArgs.m_klaFrom = -999;
		m_localStaticArgs.m_klaTo = 999;
		int num = s_priceWindowDivisionKLA.count();
		for (int i = 0; i < num; ++i) {
			if (m_localStaticArgs.m_kla >= s_priceWindowDivisionKLA[i]) {
				m_localStaticArgs.m_klaFrom = s_priceWindowDivisionKLA[i];
			}
			if (m_localStaticArgs.m_kla < s_priceWindowDivisionKLA[num - i - 1]) {
				m_localStaticArgs.m_klaTo = s_priceWindowDivisionKLA[num - i - 1];
			}
		}
		if (m_localStaticArgs.m_klaFrom < -10 && m_localStaticArgs.m_klaTo > s_priceWindowDivisionKLA[0]){
			ERR() << "kla " << m_localStaticArgs.m_kla << " klaFrom " << m_localStaticArgs.m_klaFrom << " klaTo " << m_localStaticArgs.m_klaTo;
		}
		if (m_localStaticArgs.m_klaTo > 10 && m_localStaticArgs.m_klaFrom < s_priceWindowDivisionKLA[num - 1]){
			ERR() << "kla " << m_localStaticArgs.m_kla << " klaFrom " << m_localStaticArgs.m_klaFrom << " klaTo " << m_localStaticArgs.m_klaTo;
		}
		if (qAbs(m_localStaticArgs.m_kla) < 6 && qAbs(m_localStaticArgs.m_kla - m_localStaticArgs.m_klaFrom) > 2){
			ERR() << m_localStaticArgs.m_kla <<" ! "<< m_localStaticArgs.m_klaFrom;
		}
		if (qAbs(m_localStaticArgs.m_kla) < 6 && qAbs(m_localStaticArgs.m_kla - m_localStaticArgs.m_klaTo) > 2){
			ERR() << m_localStaticArgs.m_kla <<" ! "<< m_localStaticArgs.m_klaTo;
		}
	}
	bool passFilter(qint64 dist, const Transaction& trans) const override {
		Q_UNUSED(dist);
		return trans.klaEff() <= m_localStaticArgs.m_klaTo
				&& trans.klaEff() >= m_localStaticArgs.m_klaFrom;
	}
	int minTransactionForBundle() const override { return 9999+4; }

	Oracle* makeNewOracle() override {
		OraclePriceWindow* pNewOr = new OraclePriceWindow(this);
		pNewOr->m_args = m_localStaticArgs;
		return pNewOr;
	}
	void onJustApplied(TransactionBundle& allTrans, Puppy::Context& ioContext) override {
//		if (BotContext::JSON_ARGS["PriceWindow"].toString().trimmed() != "enabled") {
//			localStaticArgs()->m_fitness -= 123456789.0;
//		}
	}
	static void initDivisions() {
		s_priceWindowDivisionKLA.clear();
		s_priceWindowDivisionKLA = { -6.2,-5.8,-5.4,-5.0,-4.6
									,-4.2,-3.8,-3.4,-3.0,-2.6
									,-2.2,-1.8,-1.4,-1.0
									,0.8,1.6,2.4,3.2,4.0
									,4.8,5.6,6.4,7.2,8.0,9.0
								   };
//		double klaArg = -6;
//		double inc = 0.2 + 0.2;
//		for (uint i = 0; i < BotContext::MAX_NUM_FEATURES; ++i) {
//			double amntBill2 = toBillDigits_2(unKindaLog(klaArg));
//			klaArg += inc;
//			if (klaArg <= inc/2 && inc < 0)
//				klaArg = inc = 0;
//			if (klaArg >= -inc/2 && inc > 0) {
//				klaArg = 8;
//				inc = -(0.4 + 0.4);
//			}
//		}
	}
public:
	static QVector<double> s_priceWindowDivisionKLA;
};

class FeatureSalaryWindow : public FeaturePriceWindow
{
public:
	FeatureSalaryWindow()
		: FeaturePriceWindow("SalaryWindow")
	{
	}
protected:
	bool passFilter(qint64 dist, const Transaction& trans) const override {
		Q_UNUSED(dist);
		bool ok = FeaturePriceWindow::passFilter(dist, trans);
		ok &= !(trans.userFlag & Transaction::UserInputFlag::noIncome);
		return ok;
	}
	int minTransactionForBundle() const override { return 2; }
	void onJustApplied(TransactionBundle& allTrans, Puppy::Context& ioContext) override {
		if (m_localStaticArgs.m_bundle.flagsCount(Transaction::UserInputFlag::yesIncome) < 4) {
			localStaticArgs()->m_fitness -= 123456789.0;
		}
		else
			localStaticArgs()->m_fitness *= 4;
	}
};

#endif // FEATUREPRICEWINDOW_H
