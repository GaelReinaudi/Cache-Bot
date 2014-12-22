#ifndef EVOLUTIONSPINNER_H
#define EVOLUTIONSPINNER_H
#include <QObject>

#include "puppy/Puppy.hpp"

class EvolutionSpinner : public QObject
{
	Q_OBJECT

public:
	EvolutionSpinner(QObject* parent = 0);
protected:
	unsigned int evaluateSymbReg(std::vector<Puppy::Tree> &ioPopulation, Puppy::Context &ioContext, const std::vector<double> &inX, const std::vector<double> &inF);

signals:
	void resultReady(const QString &result);

private:
	int m_gen = 0;
};

#endif // EVOLUTIONSPINNER_H
