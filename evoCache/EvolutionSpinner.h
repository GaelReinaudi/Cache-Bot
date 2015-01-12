#ifndef EVOLUTIONSPINNER_H
#define EVOLUTIONSPINNER_H
#include <QObject>
#include <QRectF>

#include "puppy/Puppy.hpp"
typedef QVector<QRectF> VectorRectF;

class EvolutionSpinner : public QObject
{
	Q_OBJECT

public:
	EvolutionSpinner(Account* pAc, QObject* parent = 0);

public slots:
	void startEvolution(bool doStart);

protected:
	unsigned int evaluateSymbReg(std::vector<Puppy::Tree> &ioPopulation, Puppy::Context &ioContext);

signals:
	void resultReady(const QString &result);
	void sendMask(VectorRectF vecRect);

private:
	Puppy::Context* m_context = 0;
	int m_gen = 0;
	volatile bool m_doSpin = false;
};

//Q_DECLARE_METATYPE(VectorRectF)

#endif // EVOLUTIONSPINNER_H
