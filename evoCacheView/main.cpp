#include "evoCacheView.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	if(argc < 2) {
		qDebug() << "needs an argument";
		return 0;
	}
	QString jsonStr = QString(argv[1]);

	QVector<int> onlyLoadHashes;
	for (int i = 2; i < argc; ++i) {
		onlyLoadHashes.append(QString(argv[i]).toInt());
	}

	QJsonDocument jsonDoc(QJsonDocument::fromJson(jsonStr.toUtf8()));
	const QJsonObject& jsonObj = jsonDoc.object();
	qDebug() << jsonStr << jsonObj;
	EvoCacheView w(jsonObj["user_id"].toString().trimmed(), onlyLoadHashes);
	w.show();

	return a.exec();
}
