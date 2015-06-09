#include "extraCache.h"
#include "account.h"
#include "cacherest.h"

extraCache::extraCache(QString userID)
	: CacheAccountConnector(userID)
{
}

void extraCache::onRepliedUserData(QString strData)
{
	CacheAccountConnector::onRepliedUserData(strData);
	CacheRest::Instance()->getBestBot(userID());
	connect(CacheRest::Instance()->worker, SIGNAL(repliedBestBot(QString)), this, SLOT(repliedBestBot(QString)));
}

void extraCache::repliedBestBot(QString strData)
{
	qDebug() << strData;
	account()->loadJsonData(strData.toUtf8());
}
