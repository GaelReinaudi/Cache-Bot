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
	extraCache w(userID);

	return a.exec();
}
