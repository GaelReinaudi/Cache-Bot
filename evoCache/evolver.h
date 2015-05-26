#ifndef EVOLVER_H
#define EVOLVER_H

#include <QThread>
#include "EvolutionSpinner.h"

class EvolutionSpinner;

class Evolver : public QObject
{
	Q_OBJECT

public:
	explicit Evolver(QString jsonFile, int afterJday = 0, int beforeJday = 0);
	~Evolver();

public slots:

signals:
	void initialized(bool);

private:
	QThread* m_evoThread;
	EvolutionSpinner* m_evoSpinner;
	Account* account;
};

#endif // EVOLVER_H
