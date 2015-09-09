#ifndef FEATUREALLOTHERS_H
#define FEATUREALLOTHERS_H
#include "AccRegPrimits.h"
#include "oracle.h"

class OracleFilteredRest : public Oracle
{
public:
	OracleFilteredRest(AccountFeature* pCreatingFeature)
		: Oracle(pCreatingFeature)
	{}

protected:
	QVector<Transaction> revelation(QDate upToDate) override;
	double avgDaily() const override;
	double avgDailyPos() const override{
		double avgAmnt = m_args.m_sumPos;
		avgAmnt /= m_args.m_daysBundle;
		return avgAmnt;
	}
	double avgDailyNeg() const override{
		double avgAmnt = m_args.m_sumNeg;
		avgAmnt /= m_args.m_daysBundle;
		return avgAmnt;
	}

private:
	struct Args
	{
		void intoJson(QJsonObject& o_retObj) {
			o_retObj.insert("days", m_daysBundle);
			o_retObj.insert("proba", m_dayProba);
			o_retObj.insert("numPos", m_numPos);
			o_retObj.insert("numNeg", m_numNeg);
			o_retObj.insert("sumPos", m_sumPos);
			o_retObj.insert("sumNeg", m_sumNeg);
			o_retObj.insert("numBund", m_bundle.count());
		}
		TransactionBundle m_bundle;
		double m_dayProba = 0.0;
		double m_daysBundle = 0.0;
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
		: AccountFeature(1/*keep to 0 or it will try to get the argument from a tree that doesn't exist when used at Tree.cpp:97*/
						 , "FeatureAllOthers")
	{ }

public:
	void getArgs(Puppy::Context &ioContext) override {
//		double a = 0;
//		int ind = -1;
//		getArgument(++ind, &a, ioContext);
//		m_localStaticArgs.m_avgDayIn099 = a;
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
