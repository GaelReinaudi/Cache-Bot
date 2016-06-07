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

struct FeatureArgs
{
	virtual void intoJson(QJsonObject& o_retObj) const {
		o_retObj.insert("zlabels", QJsonArray::fromStringList(m_bundle.uniqueNames()));
		o_retObj.insert("zcategories", QJsonArray::fromStringList(m_bundle.uniqueCategories()));
		o_retObj.insert("_daily", avgDaily());
		o_retObj.insert("numBund", m_bundle.count());
		double avg = m_bundle.avgSmart();
		o_retObj["avgAmnt"] = avg;
		o_retObj["stdDevAmnt"] = m_bundle.stdDevAmountSmart(avg);
		double avgD2N = m_bundle.averageD2N();
		o_retObj["avgD2N"] = avgD2N;
		o_retObj["stdDevD2N"] = m_bundle.stdDevD2N(avgD2N);
		o_retObj.insert("fitness", m_fitness);
		o_retObj.insert("filter", int(m_filterFlags));
	}
	virtual double avgDaily(int limDayProba = 0) const = 0;
	virtual double avgDailyPos(int limDayProba) const {
		double avg = avgDaily(limDayProba);
		return avg >= 0 ? avg : 0.0;
	}
	virtual double avgDailyNeg(int limDayProba) const {
		double avg = avgDaily(limDayProba);
		return avg <= 0 ? avg : 0.0;
	}
	virtual double computeProba() const { return 0; }
	TransactionBundle m_bundle;
	double m_fitness = 0.0;
	unsigned int m_filterFlags = 0xffffffff;
};

class AccountFeature : public Puppy::Primitive
{
public:
	AccountFeature(unsigned int inNumberArguments, std::string inName)
		: Primitive(inNumberArguments+1, inName)
	{}
	virtual ~AccountFeature() {}
	virtual void execute(void* outDatum, Puppy::Context& ioContext);
	virtual QJsonObject toJson(Puppy::Context& ioContext) {
		QJsonObject retObj;
		retObj.insert("name", QString::fromStdString(getName()));
		retObj.insert("numArgs", int(getNumberArguments()));
		retObj.insert("billProba", m_billProba);
//		retObj.insert("numBund", localStaticArgs()->m_bundle.count());
		localStaticArgs()->intoJson(retObj);
		QJsonArray argList;
		if (ioContext.mTree) {
			for (unsigned int i = 0; i < getNumberArguments(); ++i) {
				double lArgi = 0.0;
				getArgument(i, &lArgi, ioContext);
				argList.append(lArgi);
			}
		}
		retObj.insert("args", argList);
		return retObj;
	}
	bool isFeature() const override { return true; }
	virtual int isPeriodic() const { return 0; }

protected:
	virtual FeatureArgs* localStaticArgs() = 0;
	virtual const FeatureArgs* constLocalStaticArgs() const { return const_cast<AccountFeature*>(this)->localStaticArgs(); };
	virtual int getArgs(Puppy::Context &ioContext, int startAfter = -1) {
		double a = 0;
		getArgument(++startAfter, &a, ioContext);
		localStaticArgs()->m_filterFlags = qRound(a);
		return startAfter;
	}
	virtual void cleanArgs() {
		unsigned int i = localStaticArgs()->m_filterFlags % 5;
		switch (i) {
		case 0:
			localStaticArgs()->m_filterFlags = Transaction::Flag::EO;
			break;
		case 1:
			localStaticArgs()->m_filterFlags = Transaction::Flag::IO;
			break;
		case 2:
			localStaticArgs()->m_filterFlags = Transaction::Flag::EI;
			break;
		case 3:
			localStaticArgs()->m_filterFlags = Transaction::Flag::II;
			break;
		case 4:
			localStaticArgs()->m_filterFlags = Transaction::Flag::EE;
			break;
		default:
			localStaticArgs()->m_filterFlags = Transaction::Flag::None;
			break;
		}
	}
	virtual bool passFilter(qint64 dist, const Transaction& trans) const {
		Q_UNUSED(dist);//Q_UNUSED(trans);
		return trans.flags & constLocalStaticArgs()->m_filterFlags;
	}
	virtual void onGeneration(int nGen, double progressGeneration, Puppy::Context &ioContext) {
		Q_UNUSED(progressGeneration);
		if (nGen == 1) {
			// if we are forcing a given hashed bundle
			int filterHashIndex = ioContext.filterHashIndex;
			if(filterHashIndex >= 0) {
				m_filterHash = ioContext.m_pUser->hashBundles().keys()[filterHashIndex];
				QString nodeName = QString("h%1").arg(m_filterHash);
				bool ok = tryReplaceArgumentNode(0, nodeName.toStdString().c_str(), ioContext);
				if(!ok) {
					ERR() << "Could not replace the node with " << nodeName;
				}
				nodeName = QString("%1").arg(ioContext.m_pUser->hashBundles()[m_filterHash]->klaAverage());
				ioContext.getPrimitiveByName(nodeName);
				ok = tryReplaceArgumentNode(1, nodeName.toStdString().c_str(), ioContext);
				if(!ok) {
					ERR() << "Could not replace the node with " << nodeName;
				}
			}
			else {
				m_filterHash = -1;
			}
		}
	}
	virtual bool cannotExecute(Puppy::Context& ioContext) const { Q_UNUSED(ioContext); return false; }
	virtual double apply(TransactionBundle& allTrans, bool isPostTreat, bool doLog) = 0;
	virtual void isolateBundledTransactions(bool isPostTreatment = false);
	virtual void onJustApplied(TransactionBundle&, Puppy::Context&) {}
	virtual void emitGraphics(Puppy::Context&) const { }
	virtual Oracle* makeNewOracle() { return 0; }

	virtual double maxDailyProbability() { return qMax(0.0, localStaticArgs()->m_fitness); }

protected:
private:
	// if any, the hash to filter the transaction on
	int m_filterHash = -1;
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
		double lArgi = 0.0;
		ioContext.mTree->fitness.clear();
		for(int i = 0; i < ioContext.lim_NUM_FEATURE; ++i) {
			getArgument(i, &lArgi, ioContext);
			lResult += lArgi;
			ioContext.mTree->fitness.push_back(lArgi);
		}
	}
	double apply(TransactionBundle&, bool, bool) override { return 0.0; }
protected:
	FeatureArgs* localStaticArgs() override { return 0; }

};

class DummyFeature : public AccountFeature
{
public:
	DummyFeature(QString featureName = "Dummy")
			: AccountFeature(3, featureName.toStdString())
	{}
};


#endif // ACCREGPRIMITS_H
