#ifndef ACCREGPRIMITS_H
#define ACCREGPRIMITS_H

#include <QtCore>
#include "puppy/Puppy.hpp"
#include "EvolutionSpinner.h"

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

class FeatureSalary : public AccountFeature
{
public:
	FeatureSalary(EvolutionSpinner* evoSpinner) : AccountFeature(6, "SALARY", evoSpinner) { }
	virtual ~FeatureSalary() { }
	virtual void execute(void* outDatum, Puppy::Context& ioContext) {
		AccountFeature::execute(outDatum, ioContext);
		double& lResult = *(double*)outDatum;
		if(ioContext.m_isInFeature) {
			lResult = -40.0;
			ioContext.m_hasRecursiveFeature = true;
			return;
		}
		ioContext.m_isInFeature = true;
		getArgument(2, &m_amount, ioContext);
		getArgument(3, &m_every, ioContext);
		getArgument(4, &m_amountDelta, ioContext);
		getArgument(5, &m_dayDelta, ioContext);
		ioContext.m_isInFeature = false;
		if(ioContext.m_hasRecursiveFeature) {
			lResult = -20.0;
			ioContext.m_hasRecursiveFeature = false;
			return;
		}
		if(lResult < 0.0)
			return;
		lResult = -10.0;
		VectorRectF outVecRec;
		if (m_every < 1 || m_endAgo < 0 || m_dur < 0 || m_amountDelta < 0 || m_amountDelta > m_amount || m_dayDelta < 0) {
			lResult = -8.0;
			return;
		}
		if (m_dur > m_every * 1024 || m_every > 365) {
			lResult = -6.0;
			return;
		}
		if (m_dayDelta > 5 || m_endAgo > 1e6 || m_amount > 1e6) {
			lResult = -4.0;
			return;
		}
		lResult = -2.0;
		for (int i = m_endAgo; i < m_endAgo + m_dur; i += m_every) {
			// arbitrary grade based on (valid) input values
			lResult = m_amount / (1+m_amountDelta) + m_every / (1+m_dayDelta);
			lResult /= 1024;
			QRectF zone;
			zone.setBottom(m_amount - m_amountDelta);
			zone.setTop(m_amount + m_amountDelta);
			zone.setLeft((i + m_dayDelta));
			zone.setRight((i - m_dayDelta));
			outVecRec.append(zone);
			QVector<QVector<int> > copyDailyAmounts = ioContext.m_dailyAmounts;

			for (int j = 0; j <= m_dayDelta; ++j) {
				if(i + j >= copyDailyAmounts.size())
					continue;
				else {
					for (int& pr : copyDailyAmounts[i + j]) {
						if (pr && pr <= m_amount + m_amountDelta && pr >= m_amount - m_amountDelta) {
							pr = 0;
							goto foundIt;
						}
					}
				}
				// if j not null, we try negative deltas also
				if (j && j < i) {
					for (int& pr : copyDailyAmounts[i - j]) {
						if (pr && pr <= m_amount + m_amountDelta && pr >= m_amount - m_amountDelta) {
							pr = 0;
							goto foundIt;
						}
					}
				}
			}
			lResult -= 2 * m_amount / (1 + m_amountDelta);
foundIt:
			lResult += m_amount / (1 + m_amountDelta);
		}
		if(lResult > ioContext.bestResult) {
			emit evoSpinner()->sendMask(outVecRec);
			ioContext.bestResult = lResult;
			QThread::msleep(100);
		}
	}
protected:
	double m_amount = 0;
	double m_every = 0;
	double m_amountDelta = 0;
	double m_dayDelta = 0;
};

#endif // ACCREGPRIMITS_H
