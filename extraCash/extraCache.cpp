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
	CacheRest::Instance()->getBestBot(userID(), user());
}

void extraCache::onBotInjected()
{
	qApp->exit();
}
