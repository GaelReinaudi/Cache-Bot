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
		m_avgDayIn80 = a;
		getArgument(++ind, &a, ioContext);
		m_avgDayOut80 = a;
		getArgument(++ind, &a, ioContext);
		m_avgDayIn90 = a;
		getArgument(++ind, &a, ioContext);
		m_avgDayOut90 = a;
		getArgument(++ind, &a, ioContext);
		m_avgDayIn95 = a;
		getArgument(++ind, &a, ioContext);
		m_avgDayOut95 = a;
	}
	void cleanArgs() override {
		AccountFeature::cleanArgs();
	}

	virtual QJsonObject toJson(Puppy::Context& ioContext) {
		QJsonObject retObj = AccountFeature::toJson(ioContext);
		retObj.insert("fitness", m_fitness);
		retObj.insert("billProba", m_billProba);
		retObj.insert("avgDayIn80", m_avgDayIn80);
		retObj.insert("avgDayOut80", m_avgDayOut80);
		retObj.insert("avgDayIn90", m_avgDayIn90);
		retObj.insert("avgDayOut90", m_avgDayOut90);
		retObj.insert("avgDayIn95", m_avgDayIn95);
		retObj.insert("avgDayOut95", m_avgDayOut95);
		retObj.insert("tot$", m_bundle.sumDollar());
		return retObj;
	}

	void execute(void* outDatum, Puppy::Context& ioContext) override;

	virtual QVector<Transaction> targetTransactions(QDate iniDate, QDate lastDate);

private:
	double m_avgDayIn90 = 0.0;
	double m_avgDayOut90 = 0.0;
	double m_avgDayIn95 = 0.0;
	double m_avgDayOut95 = 0.0;
	double m_avgDayIn80 = 0.0;
	double m_avgDayOut80 = 0.0;
	double m_fitness = 0.0;
	double m_billProba = 0;
};

#endif // FEATUREALLOTHERS_H
