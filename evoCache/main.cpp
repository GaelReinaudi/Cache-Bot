#include "evolver.h"
#include <QCoreApplication>

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	if(argc < 2) {
		qDebug() << "needs an argument";
		return 0;
	}
	QString jsonStr = QString(argv[1]).remove("'");

	QJsonDocument jsonDoc(QJsonDocument::fromJson(jsonStr.toUtf8()));
	const QJsonObject& jsonObj = jsonDoc.object();
	qDebug() << endl << jsonStr << endl << jsonObj;
	Evolver w(jsonObj["user_id"].toString().trimmed(), jsonObj);

	return a.exec();
}
