#ifndef EXTRACACHE_H
#define EXTRACACHE_H

#include "cacheAccountConnector.h"
#include <QThread>

class extraCache : public CacheAccountConnector
{
	Q_OBJECT

public:
	extraCache(QString userID);
};

#endif // EXTRACACHE_H
