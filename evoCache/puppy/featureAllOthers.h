#ifndef FEATUREALLOTHERS_H
#define FEATUREALLOTHERS_H
#include "AccRegPrimits.h"
#include "oracle.h"

class OracleFilteredRest : public Oracle
{
public:

protected:
//	QVector<Transaction> revelation(QDate upToDate) override;

private:
	struct Args
	{
		void intoJson(QJsonObject& o_retObj) {
			o_retObj.insert("avgDayIn099", m_avgDayIn099);
			o_retObj.insert("avgDayOut099", m_avgDayOut099);
			o_retObj.insert("avgDayIn90", m_avgDayIn90);
			o_retObj.insert("avgDayOut90", m_avgDayOut90);
			o_retObj.insert("avgDayIn95", m_avgDayIn95);
			o_retObj.insert("avgDayOut95", m_avgDayOut95);
		}
		TransactionBundle m_bundle;
		double m_avgDayIn90 = 0.0;
		double m_avgDayOut90 = 0.0;
		double m_avgDayIn95 = 0.0;
		double m_avgDayOut95 = 0.0;
		double m_avgDayIn099 = 0.0;
		double m_avgDayOut099 = 0.0;
	} m_args;
	friend class FeatureAllOthers;
};

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
		m_localStaticArgs.m_avgDayIn099 = a;
		getArgument(++ind, &a, ioContext);
		m_localStaticArgs.m_avgDayOut099 = a;
		getArgument(++ind, &a, ioContext);
		m_localStaticArgs.m_avgDayIn90 = a;
		getArgument(++ind, &a, ioContext);
		m_localStaticArgs.m_avgDayOut90 = a;
		getArgument(++ind, &a, ioContext);
		m_localStaticArgs.m_avgDayIn95 = a;
		getArgument(++ind, &a, ioContext);
		m_localStaticArgs.m_avgDayOut95 = a;
	}

	QJsonObject toJson(Puppy::Context& ioContext) override {
		QJsonObject retObj = AccountFeature::toJson(ioContext);
		m_localStaticArgs.intoJson(retObj);
		return retObj;
	}

	void execute(void* outDatum, Puppy::Context& ioContext) override;

	double apply(TransactionBundle &allTrans);

private:
	OracleFilteredRest::Args m_localStaticArgs;
};

#endif // FEATUREALLOTHERS_H
