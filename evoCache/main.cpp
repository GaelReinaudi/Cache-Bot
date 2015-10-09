#include "evolver.h"
#include <QCoreApplication>

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	if(argc < 2) {
		qDebug() << "needs an argument";
		return 0;
	}
	QString jsonStr = QString(argv[1]);
	QJsonDocument jsonDoc(QJsonDocument::fromJson(jsonStr.toUtf8()));
	const QJsonObject& jsonObj = jsonDoc.object();
	qDebug() << jsonStr << jsonObj;
	Evolver w(jsonObj["user_id"].toString().trimmed());

	return a.exec();
}
