#include "extraCache.h"
#include <QCoreApplication>

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QString userId;
	if(argc > 1) {
		userId = argv[1];
	}
	extraCache w(userId);

	return a.exec();
}
