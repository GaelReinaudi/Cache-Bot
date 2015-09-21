#ifndef CACHEREST_H
#define CACHEREST_H

#include "cacherest_global.h"
#include "httprequestworker.h"
#include "user.h"

// http://www.creativepulse.gr/en/blog/2014/restful-api-requests-using-qt-cpp-for-linux-mac-osx-ms-windows

class CACHERESTSHARED_EXPORT CacheRest : public QObject
{
private:
	CacheRest(QObject *parent = 0);

public:
	static CacheRest* Instance(QObject *parent = 0) {
		if(!pThisStatic) {
			pThisStatic = new CacheRest(parent);
		}
		return pThisStatic;
	}

public:
	void login(QString username = "cache-bot", QString password = ")E[ls$=1IC1A$}Boji'W@zOX_<H<*n");
	void getUserIds();
	void getUserData(QString userId, User *pUserToInject = 0);
	User* newUser(QString userId);
	void sendExtraCash(QString userId, double valExtra, QJsonObject newStats);
	void sendNewBot(QString userId, QJsonObject newBot);
	void getBestBot(QString userId, User *pUserToInject = 0);

	void extraCashEC2Computation(QString userId);
	void evoCacheEC2Computation(QString userId);

public:
	HttpRequestWorker *worker = 0;
private:
	static CacheRest* pThisStatic;
};

#endif // CACHEREST_H
