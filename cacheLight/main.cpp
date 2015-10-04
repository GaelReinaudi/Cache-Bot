#include "extraCashView.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QString jsonFileOrUser = "../../data/adelineGaelTransactions.json";
	if(argc > 1) {
		jsonFileOrUser = argv[1];
	}
	QString jsonStr;
	if(argc > 2) {
		jsonStr = QString(argv[2]);
	}
//	qDebug() << "jsonStr: " << jsonStr;
	QJsonDocument jsonDoc(QJsonDocument::fromJson(jsonStr.toUtf8()));
	const QJsonObject& jsonObj = jsonDoc.object();
//	qDebug() << "jsonArgs: " << QString(QJsonDocument(jsonObj).toJson(QJsonDocument::Compact));
	ExtraCashView w(jsonFileOrUser, jsonObj);
	w.show();

	return a.exec();
}
