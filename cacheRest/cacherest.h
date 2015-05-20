#ifndef CACHEREST_H
#define CACHEREST_H

#include "cacherest_global.h"
#include "httprequestworker.h"
#include "user.h"

// http://www.creativepulse.gr/en/blog/2014/restful-api-requests-using-qt-cpp-for-linux-mac-osx-ms-windows

class CACHERESTSHARED_EXPORT CacheRest : public QObject
{
public:
	CacheRest(QObject *parent = 0);

public:
	void login(QString username, QString password);
	void getUserIds();
	void getUserData(QString userId, User *pUserToInject = 0);
	User* newUser(QString userId);


public:
	HttpRequestWorker *worker = 0;
};

#endif // CACHEREST_H
