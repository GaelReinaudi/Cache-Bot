#ifndef FEATUREALLOTHERS_H
#define FEATUREALLOTHERS_H
#include "AccRegPrimits.h"
#include "oracle.h"

class OracleFilteredRest : public Oracle
{
public:

protected:
	QVector<Transaction> revelation(QDate upToDate) override;

private:
	struct Args
	{
		void intoJson(QJsonObject& o_retObj) {
			o_retObj.insert("numPos", m_numPos);
			o_retObj.insert("numNeg", m_numNeg);
			o_retObj.insert("sumPos", m_sumPos);
			o_retObj.insert("sumNeg", m_sumNeg);
			o_retObj.insert("numBund", m_bundle.count());
		}
		TransactionBundle m_bundle;
		int m_numPos = 0;
		int m_numNeg = 0;
		int m_sumPos = 0;
		int m_sumNeg = 0;
	} m_args;
	friend class FeatureAllOthers;
};

class FeatureAllOthers : public AccountFeature
{
public:
	FeatureAllOthers()
		: AccountFeature(2, "FeatureAllOthers")
	{ }

public:
	void getArgs(Puppy::Context &ioContext) override {
		double a = 0;
		int ind = -1;
//		getArgument(++ind, &a, ioContext);
//		m_localStaticArgs.m_avgDayIn099 = a;
//		getArgument(++ind, &a, ioContext);
//		m_localStaticArgs.m_avgDayOut099 = a;
//		getArgument(++ind, &a, ioContext);
//		m_localStaticArgs.m_avgDayIn90 = a;
//		getArgument(++ind, &a, ioContext);
//		m_localStaticArgs.m_avgDayOut90 = a;
//		getArgument(++ind, &a, ioContext);
//		m_localStaticArgs.m_avgDayIn95 = a;
//		getArgument(++ind, &a, ioContext);
//		m_localStaticArgs.m_avgDayOut95 = a;
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
