#ifndef ACCREGPRIMITS_H
#define ACCREGPRIMITS_H

#include <QtCore>
#include "puppy/Puppy.hpp"
#include "EvolutionSpinner.h"

static const int MAX_NUM_FEATURES = 20;
static unsigned int LIMIT_NUM_FEATURES = 1;

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
	AccountFeature(unsigned int inNumberArguments, std::string inName, EvolutionSpinner* evoSpinner)
		: Primitive(inNumberArguments, inName)
		, m_evoSpinner(evoSpinner)
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
		QJsonArray argList;
		for (int i = 0; i < getNumberArguments(); ++i) {
			double lArgi = 0;
			getArgument(i, &lArgi, ioContext);
			argList.append(lArgi);
		}
		retObj.insert("args", argList);
		return retObj;
	}
	bool isFeature() const override { return true; }
	EvolutionSpinner *evoSpinner() const { return m_evoSpinner; }

protected:
	EvolutionSpinner* m_evoSpinner = 0;
	TransactionBundle m_bundle;
	// if any, the hash to filter the transaction on
	int m_filterHash = -1;
};

class CacheBotRootPrimitive : public AccountFeature
{
public:
	CacheBotRootPrimitive(EvolutionSpinner* evoSpinner)
		: AccountFeature(MAX_NUM_FEATURES, "ROOT", evoSpinner)
	{ }
	virtual ~CacheBotRootPrimitive() { }
	bool isRoot() const override {
		return true;
	}
	void execute(void* outDatum, Puppy::Context& ioContext) override {
		double& lResult = *(double*)outDatum;
		lResult = 0.0;
		double lArgi;
		for(unsigned int i = 0; i < LIMIT_NUM_FEATURES; ++i) {
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
//	QJsonObject toJson() override { return QJsonObject(); }
};

class FeaturePeriodicAmount : public AccountFeature
{
public:
	FeaturePeriodicAmount(EvolutionSpinner* evoSpinner, QString featureName = "FixedIncome")
		: AccountFeature(6, featureName.toStdString(), evoSpinner)
	{ }
	~FeaturePeriodicAmount() { }

	virtual int approxSpacingPayment() = 0;
//	virtual void execute(void* outDatum, Puppy::Context& ioContext) { Q_UNUSED(outDatum); Q_UNUSED(ioContext); }
	virtual void getArgs(Puppy::Context &ioContext) { Q_UNUSED(ioContext); }
	virtual void cleanArgs() {}
};

class FeatureMonthlyAmount : public FeaturePeriodicAmount
{
public:
	FeatureMonthlyAmount(EvolutionSpinner* evoSpinner)
		: FeaturePeriodicAmount(evoSpinner, "MonthlyAmount")
	{ }
	FeatureMonthlyAmount(EvolutionSpinner* evoSpinner, QString featureName)
		: FeaturePeriodicAmount(evoSpinner, featureName)
	{ }
	int approxSpacingPayment() override { return 31; }
	void getArgs(Puppy::Context &ioContext) override {
		// if we are forcing a given hashed bundle
		int filterHashIndex = ioContext.filterHashIndex;
		if(filterHashIndex >= 0) {
			m_filterHash = ioContext.m_pUser->hashBundles().keys()[filterHashIndex];
			int avgKLA = ioContext.m_pUser->hashBundles()[m_filterHash]->averageKLA();
			std::string nodeName = QString("h%1").arg(m_filterHash).toStdString();
			std::string nodeKLA = QString("kla%1").arg(avgKLA).toStdString();
			bool ok = tryReplaceArgumentNode(2, nodeName.c_str(), ioContext);
			ok &= tryReplaceArgumentNode(3, nodeKLA.c_str(), ioContext);
			if(!ok) {
				LOG() << "Could not replace the node with " << nodeName.c_str() << endl;
			}
		}
		else {
			m_filterHash = -1;
		}

		double a = 0;
		int ind = -1;
		getArgument(++ind, &a, ioContext);
		m_dayOfMonth = a;
		getArgument(++ind, &a, ioContext);
		m_kla = a;

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

	virtual QJsonObject toJson(Puppy::Context& ioContext) {
		QJsonObject retObj = FeaturePeriodicAmount::toJson(ioContext);
		retObj.insert("dayOfMonth", m_dayOfMonth);
		retObj.insert("fitness", m_fitness);
		retObj.insert("billProba", m_billProba);
		retObj.insert("labels", QJsonArray::fromStringList(m_bundle.uniqueNames()));
		retObj.insert("tot$", m_bundle.sumDollar());
		retObj.insert("consecutive", m_consecMonthBeforeMissed);
		retObj.insert("cons-missed", m_consecMissed);

		if(m_bundle.count()) {
			QString str = QString::fromStdString(getName()) + " ("
				  + QString::number(m_bundle.count()) +  ") "
				  + " kl$ " + QString::number(double(m_kla) / KLA_MULTIPLICATOR)
				  + " / " + QString::number(kindaLog(m_bundle.sumDollar() / m_bundle.count()))
				  + " = " + QString::number(unKindaLog(double(m_kla) / KLA_MULTIPLICATOR))
				  + " / " + QString::number(m_bundle.sumDollar() / m_bundle.count());
			retObj.insert("info", str);
			str = QString("On the ") + QString::number(m_dayOfMonth) + "th, ";
			str += QString("hash: ") + QString::number(m_bundle.trans(0).nameHash.hash);
			str += QString("  ind: ") + QString::number(m_bundle.trans(0).indexHash);
			retObj.insert("hash", m_bundle.trans(0).nameHash.hash);
			retObj.insert("indH", m_bundle.trans(0).indexHash);
		}

		return retObj;
	}

	void execute(void* outDatum, Puppy::Context& ioContext) override;

	double billProbability() const {
		double proba = m_fitness;
		proba *= m_consecMonthBeforeMissed;
		proba /= 4 + 2 * m_consecMissed;
		return proba;
	}

	virtual QVector<Transaction> targetTransactions(QDate iniDate, QDate lastDate);

protected:
	int m_dayOfMonth = 0;
	int m_kla = 0;
	int m_b[4];
	// characteristics
	double m_fitness = 0.0;
	int m_consecMonthBeforeMissed = 0;
	int m_consecMonth = 0;
	int m_consecMissed = 0;
	double m_billProba = 0;
};

class FeatureBiWeeklyAmount : public FeatureMonthlyAmount
{
public:
	FeatureBiWeeklyAmount(EvolutionSpinner* evoSpinner)
		: FeatureMonthlyAmount(evoSpinner, "BiWeeklyIncome")
	{ }
	int approxSpacingPayment() override { return 15; }
	virtual void cleanArgs() override {
		m_dayOfMonth2 = (m_dayOfMonth / 32) % 31;
		++m_dayOfMonth2;
		FeatureMonthlyAmount::cleanArgs();
	}
	QJsonObject toJson(Puppy::Context& ioContext) override {
		QJsonObject retObj = FeatureMonthlyAmount::toJson(ioContext);
		retObj.insert("dayOfMonth2", m_dayOfMonth2);
		return retObj;
	}
	QVector<Transaction> targetTransactions(QDate iniDate, QDate lastDate) override;

protected:
	int m_dayOfMonth2 = 0;
};

#endif // ACCREGPRIMITS_H
