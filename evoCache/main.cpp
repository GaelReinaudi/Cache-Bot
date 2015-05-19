#include "mainwindow.h"
#include <QApplication>
#include <qDebug>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
    QString jsonFile;// = "/Users/alecto/Cache-Bot/data/chrisPurchases.json";
	int afterJday = 0;
	int beforeJday = 0;
	if(argc > 1) {
		jsonFile = argv[1];
	}
	if(argc > 2) {
		afterJday = QString(argv[2]).toInt();
	}
	if(argc > 3) {
		beforeJday = QString(argv[3]).toInt();
	}
	MainWindow w(jsonFile, afterJday, beforeJday);
	w.show();

	return a.exec();
}
