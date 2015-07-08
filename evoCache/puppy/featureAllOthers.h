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
		m_avgDayIn099 = a;
		getArgument(++ind, &a, ioContext);
		m_avgDayOut099 = a;
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

	QJsonObject toJson(Puppy::Context& ioContext) override {
		QJsonObject retObj = AccountFeature::toJson(ioContext);
		retObj.insert("avgDayIn099", m_avgDayIn099);
		retObj.insert("avgDayOut099", m_avgDayOut099);
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
	double m_avgDayIn099 = 0.0;
	double m_avgDayOut099 = 0.0;
};

#endif // FEATUREALLOTHERS_H
