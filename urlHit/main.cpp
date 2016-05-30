#include <QCoreApplication>

#include "cacherest.h"
#include "cacheConnect.h"

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QString argStr = QString(argv[1]).remove("'");
	CacheConnect con(argStr);

	return a.exec();
}

