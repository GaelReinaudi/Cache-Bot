#include "extraCache.h"
#include "account.h"
#include "cacherest.h"

extraCache::extraCache(QString userID)
	: CacheAccountConnector(userID)
{
}

void extraCache::onUserInjected()
{
	CacheAccountConnector::onUserInjected();
	CacheRest::Instance()->getBestBot(userID());
	connect(CacheRest::Instance()->worker, SIGNAL(onRepliedBestBot(QString)), this, SLOT(onRepliedBestBot(QString)));
}

void extraCache::onRepliedBestBot(QString strData)
{
	qApp->exit();
	qDebug() << strData;
}
