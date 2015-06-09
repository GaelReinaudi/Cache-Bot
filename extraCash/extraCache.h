#ifndef EXTRACACHE_H
#define EXTRACACHE_H

#include "cacheAccountConnector.h"
#include <QThread>

class extraCache : public CacheAccountConnector
{
	Q_OBJECT

public:
	extraCache(QString userID);

protected:
	void onUserInjected() override;
	void repliedBestBot(QString strData) override;

};

#endif // EXTRACACHE_H
