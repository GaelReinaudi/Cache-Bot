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
//	connect(CacheRest::Instance()->worker, SIGNAL(loggedIn(bool)), this, SLOT(onLoggedIn(bool)));

	init();




	QString jsonFileOrUser = "../../data/adelineGaelTransactions.json";
	m_account->loadPlaidJson(jsonFileOrUser, 0, 0);
	m_evoSpinner->init(m_account);
	emit initialized(true);
}

Evolver::~Evolver()
{
	m_evoThread->exit();
	m_evoThread->wait(10 * 1000);
	delete m_evoThread;
	delete m_account;
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
	connect(m_evoSpinner, &EvolutionSpinner::finishedEvolution, this, &Evolver::onFinishedEvolution);
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
	m_account->loadJsonData(strData.toUtf8());
	m_evoSpinner->init(m_account);

	emit initialized(true);
}

void Evolver::onFinishedEvolution(QJsonObject finalBotObject)
{
	CacheRest::Instance()->sendNewBot(m_userId, finalBotObject);
	connect(CacheRest::Instance()->worker, SIGNAL(repliedSendNewBot(QString)), this, SLOT(onRepliedSendNewBot(QString)));
}

void Evolver::onRepliedSendNewBot(QString strData)
{
	qDebug() << strData;
	std::cout << "Exiting program";
	qApp->exit();
}

