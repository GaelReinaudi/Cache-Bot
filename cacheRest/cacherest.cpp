#include "cacherest.h"

CacheRest* CacheRest::pThisStatic = nullptr;

//#define SEND_TO_CACHEBOT_INSTEAD

CacheRest::CacheRest(QObject *parent)
	: QObject(parent)
{
	qDebug() << "creating CacheRest instance...............................";
	qDebug() << "GIT_VERSION" << QString(GIT_VERSION);
	worker = new HttpRequestWorker(this);
//	worker = new OfflineHttpRequestWorker(this);
}

void CacheRest::login(QString username, QString password) {
	HttpRequestInput httpRequest(LoginRoute, "POST");
	httpRequest.add_var("email", username);
	httpRequest.add_var("password", password);
	qDebug() << "loging in";
	worker->execute(&httpRequest);
}

void CacheRest::fakeSignup(QString userId)
{
	HttpRequestInput httpRequest(FakeSignupRoute, "POST");
	qDebug() << "fakeSignup";
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
	qDebug() << "getUserData";
	worker->execute(&httpRequest);
	if (pUserToInject) {
		QObject::connect(worker, SIGNAL(repliedUserData(QString)), pUserToInject, SLOT(injectJsonData(QString)));
	}
}

void CacheRest::hitUrl(QString url, QJsonObject args)
{
	HttpRequestInput httpRequest(url, "POST");
	args["_majorVersion"] = QString(GIT_VERSION).left(1);
	httpRequest.add_json(args);
	worker->execute(&httpRequest);
}

User *CacheRest::newUser(QString userId)
{
	User* pUser = new User(userId);
	getUserData(userId, pUser);
	return pUser;
}

void CacheRest::sendExtraCash(QString userId, double valExtra, QJsonObject newStats)
{
#ifdef SEND_TO_CACHEBOT_INSTEAD
	userId = "55518f01574600030092a822";
#endif
	if (newStats["_inArgs"].toObject()["send2Bot"].toString() != "")
		userId = "55518f01574600030092a822";
	QString url = SendExtraCashRoute.arg(userId);
	if (!m_overrideCallBackUrl.isEmpty()) {
		url = m_overrideCallBackUrl;
		WARN() << "using override on url: " << url;
	}
	HttpRequestInput httpRequest(url, "POST");
	QJsonObject json;
	json.insert("amount", valExtra);
	newStats.insert("extraCash", json);
	newStats["route"] = "flow";
	newStats["_majorVersion"] = QString(GIT_VERSION).left(1);
	httpRequest.add_json(newStats);
	worker->execute(&httpRequest);
}

void CacheRest::sendNewBot(QString userId, QJsonObject newBot)
{
#ifdef SEND_TO_CACHEBOT_INSTEAD
	userId = "55518f01574600030092a822";
#endif
	if (newBot["_inArgs"].toObject()["send2Bot"].toString() != "")
		userId = "55518f01574600030092a822";
	QString url = SendNewBotRoute + QString("/%1").arg(userId);
	if (!m_overrideCallBackUrl.isEmpty()) {
		url = m_overrideCallBackUrl;
		WARN() << "using override on url: " << url;
	}
	HttpRequestInput httpRequest(url, "POST");
	QJsonObject jsonNewBot;
	jsonNewBot.insert("newBot", newBot);
	jsonNewBot["route"] = "newBot";
	jsonNewBot["_majorVersion"] = QString(GIT_VERSION).left(1);
	httpRequest.add_json(jsonNewBot);
	worker->execute(&httpRequest);
}

void CacheRest::getBestBot(QString userId, User *pUserToInject /*= 0*/)
{
#ifdef SEND_TO_CACHEBOT_INSTEAD
	userId = "55518f01574600030092a822";
#endif
//	if (newStats["_inArgs"].toObject()["send2Bot"].toString() != "")
//		userId = "55518f01574600030092a822";
	HttpRequestInput httpRequest(BestBotRoute + QString("/%1").arg(userId), "POST");
	qDebug() << "getBestBot";
	QJsonObject json;
	json["user_id"] = userId;
	json["_majorVersion"] = QString(GIT_VERSION).left(1);
	httpRequest.add_json(json);
	worker->execute(&httpRequest);
	if (pUserToInject) {
		QObject::connect(worker, SIGNAL(repliedBestBot(QString)), pUserToInject, SLOT(injectJsonBot(QString)));
	}
}

void CacheRest::extraCashEC2Computation(QString userId)
{
	HttpRequestInput httpRequest(ExtraCashEC2Compute, "POST");
	QJsonObject jsonUserID;
	jsonUserID.insert("user_id", userId);
	httpRequest.add_json(jsonUserID);
	worker->execute(&httpRequest);
}

void CacheRest::evoCacheEC2Computation(QString userId)
{
	HttpRequestInput httpRequest(EvoCacheEC2Compute, "POST");
	QJsonObject jsonUserID;
	jsonUserID.insert("user_id", userId);
	httpRequest.add_json(jsonUserID);
	worker->execute(&httpRequest);
}


