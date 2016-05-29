#ifndef CACHECONNECT_H
#define CACHECONNECT_H

#include "cacheAccountConnector.h"
class HttpRequestWorker;

class CacheConnect : public CacheAccountConnector
{
	Q_OBJECT
public:
	CacheConnect();
protected slots:
	void onLoggedIn(bool didLogin) override;
	void onFinished(HttpRequestWorker *);
};

#endif // CACHECONNECT_H
