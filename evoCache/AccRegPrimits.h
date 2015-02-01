#ifndef ACCREGPRIMITS_H
#define ACCREGPRIMITS_H

#include <QtCore>
#include "puppy/Puppy.hpp"
#include "EvolutionSpinner.h"

static const int NUM_FEATURES = 5;

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
	{

	}
	virtual ~AccountFeature() {	}
	virtual void execute(void* outDatum, Puppy::Context& ioContext) {
		Q_UNUSED(outDatum);
		Q_UNUSED(ioContext);
		getArgument(0, &m_endAgo, ioContext);
		getArgument(1, &m_dur, ioContext);
		m_endAgo = qAbs(m_endAgo);
		m_dur = qAbs(m_dur);
	}

	bool isFeature() const override {
		return true;
	}

	EvolutionSpinner *evoSpinner() const {
		return m_evoSpinner;
	}

protected:
	EvolutionSpinner* m_evoSpinner = 0;
	double m_endAgo = 0;
	double m_dur = 0;
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
		for(int i = 0; i < getNumberArguments(); ++i) {
			getArgument(i, &lArgi, ioContext);
			lResult += lArgi;
		}
	}
};

class FeatureFixedIncome : public AccountFeature
{
public:
	FeatureFixedIncome(EvolutionSpinner* evoSpinner)
		: AccountFeature(6, "FixedIncome", evoSpinner)
	{ }
	virtual ~FeatureFixedIncome() { }
	virtual void execute(void* outDatum, Puppy::Context& ioContext);
protected:
	double m_amount = 0;
	double m_every = 365.0 / 24.0;
	double m_amountDelta = 100;
	double m_dayDelta = 3;
};

class MonthlyPayments : public AccountFeature
{
public:
	MonthlyPayments(EvolutionSpinner* evoSpinner)
		: AccountFeature(6, "MonthlyPayments", evoSpinner)
	{ }
	virtual ~MonthlyPayments() { }
	virtual void execute(void* outDatum, Puppy::Context& ioContext);
protected:
	double m_amount = 0;
	double m_every = 365.25 / 12.0;
	double m_amountDelta = 100;
	double m_dayDelta = 5;
};

#endif // ACCREGPRIMITS_H
