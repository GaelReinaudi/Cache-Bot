#include "evolver.h"
#include "core/account.h"

Evolver::Evolver(QString jsonFile, int afterJday, int beforeJday)
	: QObject()
{
	// an account object that is going to be populated by the json file
	account = new Account();
	account->loadPlaidJson(jsonFile, afterJday, beforeJday);
	// needed to spin a new thread and run the evolution in it
	m_evoThread = new QThread();
	m_evoSpinner = new EvolutionSpinner(account);
	m_evoSpinner->moveToThread(m_evoThread);
	connect(m_evoThread, &QThread::finished, m_evoSpinner, &QObject::deleteLater);
	connect(this, &Evolver::initialized, m_evoSpinner, &EvolutionSpinner::startStopEvolution);

	m_evoThread->start();
	emit initialized(true);
}

Evolver::~Evolver()
{
}

