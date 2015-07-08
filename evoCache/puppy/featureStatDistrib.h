#ifndef FEATURESTATDISTRIB_H
#define FEATURESTATDISTRIB_H
#include "AccRegPrimits.h"
#include "oracle.h"

class FeatureStatDistrib : public AccountFeature, public Oracle
{
public:
	FeatureStatDistrib(QString featureName = "StatDistrib")
		: AccountFeature(6, featureName.toStdString())
	{
	}
	~FeatureStatDistrib() {}

protected:
	void getArgs(Puppy::Context &ioContext) override {
		AccountFeature::getArgs(ioContext);
	}
	void cleanArgs() override {
		AccountFeature::cleanArgs();
	}
	void execute(void* outDatum, Puppy::Context& ioContext) override;
	QVector<Transaction> revelation(QDate upToDate) override;
	Transaction randomTransaction() {
		if (m_bundle.count() == 0)
			return Transaction();
		Transaction& randTransBundle = m_bundle.trans(qrand() % m_bundle.count());
		m_modelTrans.setAmount(randTransBundle.amountDbl());
		return m_modelTrans;
	}

private:
	int m_hash = 0;
	Transaction m_modelTrans;
	double m_dayProba = 0.0;
	double m_daysBundle = 0.0;
};

#endif // FEATURESTATDISTRIB_H
