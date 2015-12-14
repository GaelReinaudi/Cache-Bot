#include "spinnerwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	SpinnerWindow w;
	w.show();

	return a.exec();
}
