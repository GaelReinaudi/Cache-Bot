#include "cacheAccountConnector.h"
#include "account.h"
#include "cacherest.h"

CacheAccountConnector::CacheAccountConnector(QString userID)
	: QObject()
	, m_userId(userID)
{
	// login as cache-bot
	CacheRest::Instance()->login();
	connect(CacheRest::Instance()->worker, SIGNAL(loggedIn(bool)), this, SLOT(onLoggedIn(bool)));
}

CacheAccountConnector::~CacheAccountConnector()
{
	delete m_user;
}

void CacheAccountConnector::onLoggedIn(bool didLogin)
{
	if(didLogin) {
		m_user = new User(userID());
		CacheRest::Instance()->getUserData(userID(), m_user);
		connect(m_user, SIGNAL(injected(User*)), this, SLOT(onUserInjected(User*)));
		connect(m_user, SIGNAL(injected(User*)), this, SIGNAL(injected(User*)));
		connect(m_user, SIGNAL(botInjected()), this, SLOT(onBotInjected()));
		connect(m_user, SIGNAL(botInjected()), this, SIGNAL(botInjected()));
		connect(CacheRest::Instance()->worker, SIGNAL(repliedBestBot(QString)), this, SLOT(onRepliedBestBot(QString)));
		connect(CacheRest::Instance()->worker, SIGNAL(repliedExtraCache(QString)), this, SLOT(onRepliedExtraCache(QString)));
	}
	else {
		qWarning() << "could not log into the server";
	}
}

void CacheAccountConnector::onUserInjected(User* pUser)
{
	qDebug() << "CacheAccountConnector has injected user" << pUser->email();
}

void CacheAccountConnector::onBotInjected()
{
	qDebug() << "CacheAccountConnector has a bot injected object";
}

void CacheAccountConnector::onRepliedSendNewBot(QString strData)
{
	Q_UNUSED(strData);
}

void CacheAccountConnector::onRepliedBestBot(QString strData)
{
	Q_UNUSED(strData);
}

void CacheAccountConnector::onRepliedExtraCache(QString strData)
{
	Q_UNUSED(strData);
}
