#include "evolver.h"
#include "account.h"
#include "cacherest.h"


Evolver::Evolver(QString userID, QJsonObject jsonArgs)
	: CacheAccountConnector(userID, jsonArgs)
{
	init();
	connect(this, SIGNAL(startStopEvolution(bool)), m_evoSpinner, SLOT(startStopEvolution(bool)), Qt::DirectConnection);
	QTimer::singleShot(1000 * (4 * 60 + 45), this, SLOT(timeOut()));
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

void Evolver::onUserInjected(User* pUser)
{
	CacheAccountConnector::onUserInjected(pUser);

	Q_ASSERT(pUser == user());
	m_evoSpinner->init(user());
}

void Evolver::onFinishedEvolution(QJsonObject finalBotObject)
{
	if (flags & SendBot) {
		static QJsonObject staticToSendObject = finalBotObject;
		staticToSendObject.insert("_inArgs", jsonArgs());
		CacheRest::Instance()->sendNewBot(userID(), staticToSendObject);
		connect(CacheRest::Instance()->worker, SIGNAL(repliedSendNewBot(QString)), this, SLOT(onRepliedSendNewBot(QString)));
	}
}

void Evolver::onRepliedSendNewBot(QString strData)
{
	CacheAccountConnector::onRepliedSendNewBot(strData);

	qDebug() << strData;
	if (strData.contains("\"error\""))
	{
		ERR() << "Can't send Bot: " << strData;
		ERR() << "Trying again by loging in again until it works";
		disconnect(CacheRest::Instance()->worker, SIGNAL(loggedIn(bool)));
		connect(CacheRest::Instance()->worker, SIGNAL(loggedIn(bool)), this, SLOT(onFinishedEvolution()));
		CacheRest::Instance()->login();
		return;
	}
	std::cout << "Exiting program";
	qApp->exit();
}

