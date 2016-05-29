#include <QCoreApplication>

#include "cacherest.h"
#include "cacheConnect.h"

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	CacheConnect con;

	return a.exec();
}

