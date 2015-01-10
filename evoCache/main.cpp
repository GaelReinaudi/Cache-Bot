#include "mainwindow.h"
#include <QApplication>
#include <qDebug>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QString jsonFile = "";
	if(argc > 1) {
		jsonFile = argv[1];
	}
	MainWindow w(jsonFile);
	w.show();

	return a.exec();
}
