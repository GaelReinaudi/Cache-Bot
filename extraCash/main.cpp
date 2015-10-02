#include "extraCache.h"
#include <QCoreApplication>

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	if(argc < 2) {
		qDebug() << "needs an argument";
		return 0;
	}
	QString userID = argv[1];
	QString jsonStr;
	if(argc > 2) {
		jsonStr = QString(argv[2]);
	}
	QJsonDocument jsonDoc(QJsonDocument::fromJson(jsonStr.toUtf8()));
	const QJsonObject& jsonObj = jsonDoc.object();
	ExtraCache w(userID, jsonObj);

	return a.exec();
}
