#include "evolver.h"
#include "core/account.h"
#include "cacherest.h"

Evolver::Evolver(QString jsonFile, int afterJday, int beforeJday)
	: QObject()
{
	CacheRest::Instance()->login();
	connect(CacheRest::Instance()->worker, SIGNAL(loggedIn(bool)), this, SLOT(onLoggedIn(bool)));

	init();

	m_account->loadPlaidJson(jsonFile, afterJday, beforeJday);
	m_evoSpinner->init(m_account);

	emit initialized(true);
}

Evolver::Evolver(QString userID)
	: QObject()
	, m_userId(userID)
{
	CacheRest::Instance()->login();
	connect(CacheRest::Instance()->worker, SIGNAL(loggedIn(bool)), this, SLOT(onLoggedIn(bool)));

	init();
}

void Evolver::init()
{
	// an account object that is going to be populated by the json file
	m_account = new Account();
	// needed to spin a new thread and run the evolution in it
	m_evoThread = new QThread();
	m_evoSpinner = new EvolutionSpinner();
	m_evoSpinner->moveToThread(m_evoThread);
	connect(m_evoThread, &QThread::finished, m_evoSpinner, &QObject::deleteLater);
	connect(this, &Evolver::initialized, m_evoSpinner, &EvolutionSpinner::startStopEvolution);
	m_evoThread->start();
}

void Evolver::onLoggedIn(bool didLogin)
{
	if(didLogin) {
		CacheRest::Instance()->getUserData(m_userId);
		connect(CacheRest::Instance()->worker, SIGNAL(repliedUserData(QString)), this, SLOT(onRepliedUserData(QString)));
	}
	else {
		qWarning() << "could not log into the server";
	}
}

void Evolver::onRepliedUserData(QString strData)
{
	m_account->loadPlaidJson(strData);
	m_evoSpinner->init(m_account);

	emit initialized(true);
}

