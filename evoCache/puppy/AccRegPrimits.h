#ifndef ACCREGPRIMITS_H
#define ACCREGPRIMITS_H

#include <QtCore>
#include "puppy/Puppy.hpp"
#include "botContext.h"
#include "oracle.h"

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
class Avg : public Puppy::Primitive
{
public:
	Avg() : Primitive(2, "AVG") { }
	virtual void execute(void* outDatum, Puppy::Context& ioContext)  {
		double& lResult = *(double*)outDatum;
		double lArg2;
		getArgument(0, &lResult, ioContext);
		getArgument(1, &lArg2, ioContext);
		lResult += lArg2;
		lResult *= 0.5;
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
	Cosinus() : Primitive(1, "COS") { }
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
	AccountFeature(unsigned int inNumberArguments, std::string inName)
		: Primitive(inNumberArguments, inName)
	{}
	virtual ~AccountFeature() {}
	virtual void execute(void* outDatum, Puppy::Context& ioContext) {
		Q_UNUSED(outDatum);
		Q_UNUSED(ioContext);
	}
	virtual QJsonObject toJson(Puppy::Context& ioContext) {
		QJsonObject retObj;
		retObj.insert("name", QString::fromStdString(getName()));
		retObj.insert("numArgs", int(getNumberArguments()));
		retObj.insert("fitness", m_fitness);
		retObj.insert("billProba", m_billProba);
		QJsonArray argList;
		for (unsigned int i = 0; i < getNumberArguments(); ++i) {
			double lArgi = 0.0;
			getArgument(i, &lArgi, ioContext);
			argList.append(lArgi);
		}
		retObj.insert("args", argList);
		return retObj;
	}
	bool isFeature() const override { return true; }
protected:
	virtual void getArgs(Puppy::Context &ioContext) {
		// if we are forcing a given hashed bundle
		int filterHashIndex = ioContext.filterHashIndex;
		if(filterHashIndex >= 0) {
			m_filterHash = ioContext.m_pUser->hashBundles().keys()[filterHashIndex];
		}
		else {
			m_filterHash = -1;
		}
	}
	virtual void cleanArgs() {}

protected:
	// if any, the hash to filter the transaction on
	int m_filterHash = -1;

//	TransactionBundle m_bundle;
	double m_fitness = 0.0;
	double m_billProba = 0.0;
};

class CacheBotRootPrimitive : public AccountFeature
{
public:
	CacheBotRootPrimitive()
		: AccountFeature(BotContext::MAX_NUM_FEATURES, rootName().toStdString())
	{ }
	virtual ~CacheBotRootPrimitive() { }
	static QString rootName() {
		return "ROOT";
	}
	bool isRoot() const override { return true; }
	void execute(void* outDatum, Puppy::Context& ioContext) override {
		double& lResult = *(double*)outDatum;
		lResult = 0.0;
		double lArgi;
		for(unsigned int i = 0; i < BotContext::LIMIT_NUM_FEATURES; ++i) {
			getArgument(i, &lArgi, ioContext);
			lResult += lArgi;
		}
	}
};

class DummyFeature : public AccountFeature
{
public:
	DummyFeature(QString featureName = "Dummy")
			: AccountFeature(6, featureName.toStdString())
	{}
};


#endif // ACCREGPRIMITS_H
