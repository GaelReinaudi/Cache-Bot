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
	void onBotInjected() override;

private:
	QDate m_date;
	int m_d0 = 0;
};

#endif // EXTRACACHE_H
