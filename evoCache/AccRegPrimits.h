#ifndef ACCREGPRIMITS_H
#define ACCREGPRIMITS_H

#include <QtCore>
#include "puppy/Puppy.hpp"
#include "EvolutionSpinner.h"

static const int NUM_FEATURES = 6;

class Add : public Puppy::Primitive
{
public:
	Add() : Primitive(2, "ADD") { }
	virtual ~Add() {}
	virtual void execute(void* outDatum, Puppy::Context& ioContext)  {
		double& lResult = *(double*)outDatum;
		double lArg2;
		getArgument(0, &lResult, ioContext);
		getArgument(1, &lArg2, ioContext);
		lResult += lArg2;
	}
};
class Add4 : public Puppy::Primitive
{
public:
	Add4() : Primitive(4, "ADD4") { }
	virtual ~Add4() {}
	virtual void execute(void* outDatum, Puppy::Context& ioContext)  {
		double& lResult = *(double*)outDatum;
		double lArg2, lArg3, lArg4;
		getArgument(0, &lResult, ioContext);
		getArgument(1, &lArg2, ioContext);
		getArgument(2, &lArg3, ioContext);
		getArgument(3, &lArg4, ioContext);
		lResult += lArg2 + lArg3 + lArg4;
	}
};
class Subtract : public Puppy::Primitive
{
public:
	Subtract() : Primitive(2, "SUB") { }
	virtual ~Subtract() { }
	virtual void execute(void* outDatum, Puppy::Context& ioContext) {
		double& lResult = *(double*)outDatum;
		double lArg2;
		getArgument(0, &lResult, ioContext);
		getArgument(1, &lArg2, ioContext);
		lResult -= lArg2;
	}
};
class Multiply : public Puppy::Primitive
{
public:
	Multiply() : Primitive(2, "MUL") { }
	virtual ~Multiply() { }
	virtual void execute(void* outDatum, Puppy::Context& ioContext) {
		double& lResult = *(double*)outDatum;
		double lArg2;
		getArgument(0, &lResult, ioContext);
		getArgument(1, &lArg2, ioContext);
		lResult *= lArg2;
	}
};
class Divide : public Puppy::Primitive
{
public:
	Divide() : Primitive(2, "DIV") { }
	virtual ~Divide() { }
	virtual void execute(void* outDatum, Puppy::Context& ioContext) {
		double& lResult = *(double*)outDatum;
		double lArg2;
		getArgument(1, &lArg2, ioContext);
		if(qAbs(lArg2) < 0.001) lResult = 1.0;
		else {
			getArgument(0, &lResult, ioContext);
			lResult /= lArg2;
		}
	}
};
class Cosinus : public Puppy::Primitive
{
public:
	Cosinus() : Primitive(2, "COS") { }
	virtual ~Cosinus() { }
	virtual void execute(void* outDatum, Puppy::Context& ioContext) {
		double& lResult = *(double*)outDatum;
		getArgument(0, &lResult, ioContext);
		lResult = qCos(lResult);
	}
};

class AccountFeature : public Puppy::Primitive
{
public:
	AccountFeature(unsigned int inNumberArguments, std::string inName, EvolutionSpinner* evoSpinner)
		: Primitive(inNumberArguments, inName)
		, m_evoSpinner(evoSpinner)
	{}
	virtual ~AccountFeature() {}
	virtual void execute(void* outDatum, Puppy::Context& ioContext) {
		Q_UNUSED(outDatum);
		Q_UNUSED(ioContext);
	}
	bool isFeature() const override { return true; }
	EvolutionSpinner *evoSpinner() const { return m_evoSpinner; }

protected:
	EvolutionSpinner* m_evoSpinner = 0;
	TransactionBundle m_bundle;
};

class CacheBotRootPrimitive : public AccountFeature
{
public:
	CacheBotRootPrimitive(EvolutionSpinner* evoSpinner)
		: AccountFeature(NUM_FEATURES, "ROOT", evoSpinner)
	{ }
	virtual ~CacheBotRootPrimitive() { }
	bool isRoot() const override {
		return true;
	}
	void execute(void* outDatum, Puppy::Context& ioContext) override {
		double& lResult = *(double*)outDatum;
		lResult = 0.0;
		double lArgi;
		for(unsigned int i = 0; i < getNumberArguments(); ++i) {
			getArgument(i, &lArgi, ioContext);
			lResult += lArgi;
		}
	}
};

class DummyFeature : public AccountFeature
{
public:
	DummyFeature(EvolutionSpinner* evoSpinner, QString featureName = "Dummy")
			: AccountFeature(6, featureName.toStdString(), evoSpinner)
	{}
};

class FeaturePeriodicAmount : public AccountFeature
{
public:
	FeaturePeriodicAmount(EvolutionSpinner* evoSpinner, QString featureName = "FixedIncome")
		: AccountFeature(6, featureName.toStdString(), evoSpinner)
	{ }
	~FeaturePeriodicAmount() { }

	virtual void execute(void* outDatum, Puppy::Context& ioContext) {}
	virtual void getArgs(Puppy::Context &ioContext) {}
	virtual void cleanArgs() {}
};

class FeatureBiWeeklyAmount : public FeaturePeriodicAmount
{
public:
	FeatureBiWeeklyAmount(EvolutionSpinner* evoSpinner)
		: FeaturePeriodicAmount(evoSpinner, "BiWeeklyIncome")
	{ }
	virtual void cleanArgs() override {
		FeaturePeriodicAmount::cleanArgs();
	}
};

class FeatureMonthlyAmount : public FeaturePeriodicAmount
{
public:
	FeatureMonthlyAmount(EvolutionSpinner* evoSpinner)
		: FeaturePeriodicAmount(evoSpinner, "MonthlyAmount")
	{ }
	void getArgs(Puppy::Context &ioContext) override {
		double a = 0;
		int ind = -1;
		getArgument(++ind, &a, ioContext);
		m_dayOfMonth = a;
		getArgument(++ind, &a, ioContext);
		m_kamount = a * 1024.0;

		int bInd = -1;
		getArgument(++ind, &a, ioContext);
		m_b[++bInd] = a;
		getArgument(++ind, &a, ioContext);
		m_b[++bInd] = a;
		getArgument(++ind, &a, ioContext);
		m_b[++bInd] = a;
		getArgument(++ind, &a, ioContext);
		m_b[++bInd] = a;
	}
	void cleanArgs() override {
		FeaturePeriodicAmount::cleanArgs();
		m_dayOfMonth %= 31;
		++m_dayOfMonth;
	}
	void execute(void* outDatum, Puppy::Context& ioContext) override;

protected:
	int m_dayOfMonth = 0;
	int m_kamount = 0;
	char m_b[4];
};

#endif // ACCREGPRIMITS_H
