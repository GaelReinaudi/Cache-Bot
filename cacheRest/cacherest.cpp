#include "cacherest.h"

CacheRest* CacheRest::pThisStatic = nullptr;

CacheRest::CacheRest(QObject *parent)
	: QObject(parent)
{
	qDebug() << "creating CacheRest instance...............................";
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
	HttpRequestInput httpRequest(UserDataRoute + QString("/%1").arg(userId), "POST");
	worker->execute(&httpRequest);
	if (pUserToInject) {
		QObject::connect(worker, SIGNAL(repliedUserData(QString)), pUserToInject, SLOT(injectJsonData(QString)));
	}
}

User *CacheRest::newUser(QString userId)
{
	User* pUser = new User(userId);
	getUserData(userId, pUser);
	return pUser;
}

void CacheRest::sendExtraCash(QString userId, double valExtra)
{
	HttpRequestInput httpRequest(SendExtraCashRoute + QString("/%1").arg(userId), "POST");
	QJsonObject json;
	json.insert("amount", valExtra);
	QJsonObject jsonExtraCash;
	jsonExtraCash.insert("extraCash", json);
	httpRequest.add_json(jsonExtraCash);
	worker->execute(&httpRequest);
}

void CacheRest::sendNewBot(QString userId, QJsonObject newBot)
{
	HttpRequestInput httpRequest(SendNewBotRoute + QString("/%1").arg(userId), "POST");
	QJsonObject jsonNewBot;
	jsonNewBot.insert("newBot", newBot);
	httpRequest.add_json(jsonNewBot);
	worker->execute(&httpRequest);
}

void CacheRest::getBestBot(QString userId, User *pUserToInject /*= 0*/)
{
	HttpRequestInput httpRequest(BestBotRoute + QString("/%1").arg(userId), "POST");
	worker->execute(&httpRequest);
	if (pUserToInject) {
		QObject::connect(worker, SIGNAL(repliedBestBot(QString)), pUserToInject, SLOT(injectJsonBot(QString)));
	}
}

void CacheRest::extraCashEC2Computation(QString userId)
{
	HttpRequestInput httpRequest(ExtraCashEC2Compute + QString("/%1").arg(userId), "GET");
	worker->execute(&httpRequest);
}


