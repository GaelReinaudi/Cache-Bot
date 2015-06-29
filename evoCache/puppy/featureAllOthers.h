#ifndef FEATUREALLOTHERS_H
#define FEATUREALLOTHERS_H

#include "AccRegPrimits.h"

class FeatureAllOthers : public AccountFeature
{
public:
	FeatureAllOthers()
		: AccountFeature(6, "FeatureAllOthers")
	{ }

public:
	void getArgs(Puppy::Context &ioContext) override {
		double a = 0;
		int ind = -1;
		getArgument(++ind, &a, ioContext);
		m_avgDayIncome = a;
		getArgument(++ind, &a, ioContext);
		m_avgDayOutcome = a;
	}
	void cleanArgs() override {
		AccountFeature::cleanArgs();
	}

	virtual QJsonObject toJson(Puppy::Context& ioContext) {
		QJsonObject retObj = AccountFeature::toJson(ioContext);
		retObj.insert("avgDayIncome", m_avgDayIncome * 100.0);
		retObj.insert("avgDayOutcome", m_avgDayOutcome * 100.0);
		retObj.insert("tot$", m_bundle.sumDollar());
		return retObj;
	}

	void execute(void* outDatum, Puppy::Context& ioContext) override;

	virtual QVector<Transaction> targetTransactions(QDate iniDate, QDate lastDate);

private:
	double m_avgDayIncome = 0.0;
	double m_avgDayOutcome = 0.0;
	double m_fitness = 0.0;
	double m_billProba = 0;
};

#endif // FEATUREALLOTHERS_H
