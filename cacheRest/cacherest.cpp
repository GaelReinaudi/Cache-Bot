#include "cacherest.h"


CacheRest::CacheRest(QObject *parent)
	: QObject(parent)
{
	worker = new HttpRequestWorker(this);
}

void CacheRest::login(QString username, QString password) {
	HttpRequestInput httpRequest(LoginRoute, "POST");
	httpRequest.add_var("email", username);
	httpRequest.add_var("password", password);
	worker->execute(&httpRequest);
}

void CacheRest::getUserIds()
{
	HttpRequestInput httpRequest(IdsRoute, "POST");
	worker->execute(&httpRequest);
}

void CacheRest::getUserData(QString userId, User *pUserToInject /*= 0*/)
{
	HttpRequestInput httpRequest(UserDataRoute.arg(userId), "POST");
	worker->execute(&httpRequest);
	if (pUserToInject) {
		QObject::connect(worker, SIGNAL(repliedUserData(QString)), pUserToInject, SLOT(injectJsonString(QString)));
	}
}

User *CacheRest::newUser(QString userId)
{
	User* pUser = new User(userId);
	getUserData(userId, pUser);
	return pUser;
}


