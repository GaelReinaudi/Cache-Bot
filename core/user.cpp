#include "user.h"


User::User(QString userId)
	: DBobj(userId)
{}

void User::readJson() {

}

void User::injectJsonString(QString jsonStr)
{
	qDebug() << "injecting" << jsonStr.left(128);
	QJsonDocument jsonDoc(QJsonDocument::fromJson(jsonStr.toUtf8()));
	const QJsonObject& jsonObj = jsonDoc.object();

	QJsonArray npcArrayAccount = jsonObj["accounts"].toArray();
	qDebug() << npcArrayAccount.size();
//	for (int npcIndex = 0; npcIndex < npcArrayAccount.size(); ++npcIndex) {
//		QJsonObject npcObject = npcArrayAccount[npcIndex].toObject();
//		QString accountID = npcObject["_id"].toString();
//	}
	emit injected();
}

