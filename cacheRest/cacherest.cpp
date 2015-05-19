#include "cacherest.h"


CacheRest::CacheRest(QObject *parent)
	: QObject(parent)
{
	worker = new HttpRequestWorker(this);
	//	QObject::connect(reply, &QNetworkReply::finished, worker, &HttpRequestWorker::on_login_finished);
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

void CacheRest::getUserData(QString userId)
{
	HttpRequestInput httpRequest(UserDataRoute.arg(userId), "POST");
	worker->execute(&httpRequest);
}
//void CacheRest::getUserData2(QString userId, User *pUserToInject)
//{
//	HttpRequestInput httpRequest(UserDataRoute.arg(userId), "POST");
//	worker->execute(&httpRequest);
//	if (pUserToInject) {
//		QObject::connect(worker, SIGNAL(repliedUserData(QString)), pUserToInject, SLOT(injectJsonString(QString)));
//	}
//}


