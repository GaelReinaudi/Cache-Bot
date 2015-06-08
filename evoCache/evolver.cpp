#include "evolver.h"
#include "account.h"
#include "cacherest.h"

Evolver::Evolver(QString jsonFile, int afterJday, int beforeJday)
	: CacheAccountConnector("")
{
	init();

	account()->loadPlaidJson(jsonFile, afterJday, beforeJday);
	m_evoSpinner->init(account());

	emit initialized(true);
}

Evolver::Evolver(QString userID)
	: CacheAccountConnector(userID)
{
	init();
}

Evolver::~Evolver()
{
	m_evoThread->exit();
	m_evoThread->wait(10 * 1000);
	delete m_evoThread;
}

void Evolver::init()
{
	// needed to spin a new thread and run the evolution in it
	m_evoThread = new QThread();
	m_evoSpinner = new EvolutionSpinner();
	m_evoSpinner->moveToThread(m_evoThread);
	connect(m_evoThread, &QThread::finished, m_evoSpinner, &QObject::deleteLater);
	connect(m_evoSpinner, &EvolutionSpinner::initialized, m_evoSpinner, &EvolutionSpinner::startStopEvolution);
	connect(m_evoSpinner, &EvolutionSpinner::finishedEvolution, this, &Evolver::onFinishedEvolution);
	m_evoThread->start();
}

void Evolver::onRepliedUserData(QString strData)
{
//	CacheAccountConnector::onRepliedUserData(strData);
	Q_UNUSED(strData);
	QString jsonFileOrUser = "../../data/adelineGaelTransactions.json";
	account()->loadPlaidJson(jsonFileOrUser, 0, 0);
	m_evoSpinner->init(account());
}

void Evolver::onFinishedEvolution(QJsonObject finalBotObject)
{
	CacheRest::Instance()->sendNewBot(userID(), finalBotObject);
	connect(CacheRest::Instance()->worker, SIGNAL(repliedSendNewBot(QString)), this, SLOT(onRepliedSendNewBot(QString)));
}

void Evolver::onRepliedSendNewBot(QString strData)
{
	CacheAccountConnector::onRepliedSendNewBot(strData);

	qDebug() << strData;
	std::cout << "Exiting program";
	qApp->exit();
}

