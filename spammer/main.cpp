#include "spammerwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	SpammerWindow w;
	w.show();

	return a.exec();
}
