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
	connect(CacheRest::Instance()->worker, SIGNAL(repliedBestBot(QString)), this, SLOT(repliedBestBot(QString)));
}

void extraCache::repliedBestBot(QString strData)
{
	qApp->exit();
	qDebug() << strData;
}
