#ifndef EVOLUTIONSPINNER_H
#define EVOLUTIONSPINNER_H
#include <QObject>

#include "puppy/Puppy.hpp"

class EvolutionSpinner : public QObject
{
	Q_OBJECT

public:
	EvolutionSpinner(Account* pAc, QObject* parent = 0);

	void doSpin() {
		m_doSpin = true;
	}

protected:
	unsigned int evaluateSymbReg(std::vector<Puppy::Tree> &ioPopulation, Puppy::Context &ioContext, const std::vector<double> &inX, const std::vector<double> &inF);

signals:
	void resultReady(const QString &result);

private:
	Puppy::Context* m_context = 0;
	int m_gen = 0;
	volatile bool m_doSpin = false;
};

#endif // EVOLUTIONSPINNER_H
