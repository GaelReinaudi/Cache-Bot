#include "userViewer.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	if(argc < 2) {
		qDebug() << "needs an argument";
		return 0;
	}
	QString jsonStr = QString(argv[1]).remove("'");
	QJsonDocument jsonDoc(QJsonDocument::fromJson(jsonStr.toUtf8()));
	QJsonObject jsonObj = jsonDoc.object();
	qDebug() << endl << jsonStr << endl << jsonObj;
	jsonObj.insert("PriceWindow", QString("enabled"));
	jsonObj.insert("Category", QString("enabled"));
	UserViewer w(jsonObj["user_id"].toString().trimmed(), jsonObj);
	w.show();

	return a.exec();
}
