#ifndef ACCREGPRIMITS_H
#define ACCREGPRIMITS_H

#include <QtCore>
#include "puppy/Puppy.hpp"

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
	AccountFeature(unsigned int inNumberArguments, std::string inName)
		: Primitive(inNumberArguments, inName)
	{

	}
	virtual ~AccountFeature() { }
	virtual void execute(void* outDatum, Puppy::Context& ioContext) {
		Q_UNUSED(outDatum);
		Q_UNUSED(ioContext);
		getArgument(0, &m_endAgo, ioContext);
		getArgument(1, &m_dur, ioContext);
	}

	bool isFeature() const override {
		return true;
	}

protected:
	int m_endAgo = 0;
	int m_dur = 0;
};

class FeatureSalary : public AccountFeature
{
public:
	FeatureSalary() : AccountFeature(6, "SALARY") { }
	virtual ~FeatureSalary() { }
	virtual void execute(void* outDatum, Puppy::Context& ioContext) {
		AccountFeature::execute(outDatum, ioContext);
		getArgument(2, &m_amount, ioContext);
		getArgument(3, &m_every, ioContext);
		getArgument(4, &m_amountDelta, ioContext);
		getArgument(5, &m_dayDelta, ioContext);
		double& lResult = *(double*)outDatum;
		lResult = -1.0;
		if (m_every < 1 || m_endAgo < 0 || m_dur < 0 || m_amountDelta < 0 || m_amountDelta > m_amount || m_dayDelta < 0)
			return;
		if (m_dur > m_every<<10 || m_every > 365)
			return;
		if (m_dayDelta > 5)
			return;
		// arbitrary grade based on (valid) input values
		lResult = m_amount - m_amountDelta - m_dayDelta;
		for (int i = m_endAgo; i < m_endAgo + m_dur; i += m_every) {
			for (int j = 0; j <= m_dayDelta; ++j) {
				if(i + j >= ioContext.m_dailyAmounts.size())
					continue;
				for (int pr : ioContext.m_dailyAmounts[i + j]) {
					if (pr <= m_amount + m_amountDelta && pr >= m_amount - m_amountDelta) {
						goto foundIt;
					}
				}
				// if j not null, we try negative deltas also
				if (j) {
					for (int pr : ioContext.m_dailyAmounts[i - j]) {
						if (pr <= m_amount + m_amountDelta && pr >= m_amount - m_amountDelta) {
							goto foundIt;
						}
					}
				}
			}
		}
		lResult -= 2 * m_amount;
foundIt:
		lResult += m_amount;
	}
protected:
	int m_amount = 0;
	int m_every = 0;
	int m_amountDelta = 0;
	int m_dayDelta = 0;
};

#endif // ACCREGPRIMITS_H
