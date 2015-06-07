#include "cacheAccountConnector.h"
#include "account.h"
#include "cacherest.h"

CacheAccountConnector::CacheAccountConnector(QString userID)
	: QObject()
	, m_userId(userID)
{
	m_account = new Account();

	// login as cache-bot
	CacheRest::Instance()->login();
	connect(CacheRest::Instance()->worker, SIGNAL(loggedIn(bool)), this, SLOT(onLoggedIn(bool)));
}

CacheAccountConnector::~CacheAccountConnector()
{
	delete m_account;
}

void CacheAccountConnector::onLoggedIn(bool didLogin)
{
	if(didLogin) {
		CacheRest::Instance()->getUserData(userID());
		connect(CacheRest::Instance()->worker, SIGNAL(repliedUserData(QString)), this, SLOT(onRepliedUserData(QString)));
	}
	else {
		qWarning() << "could not log into the server";
	}
}

void CacheAccountConnector::onRepliedUserData(QString strData)
{
	account()->loadJsonData(strData.toUtf8());
}

void CacheAccountConnector::onRepliedSendNewBot(QString strData)
{
	Q_UNUSED(strData);
}

