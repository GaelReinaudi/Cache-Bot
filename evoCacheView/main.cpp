#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QString jsonFileOrUser = "../../data/adelineGaelTransactions.json";
	int afterJday = 0;
	int beforeJday = 0;
	if(argc > 1) {
		jsonFileOrUser = argv[1];
		if(argc > 2) {
			afterJday = QString(argv[2]).toInt();
			if(argc > 3) {
				beforeJday = QString(argv[3]).toInt();
			}
		}
	}
	MainWindow w(jsonFileOrUser, afterJday, beforeJday);
	w.show();

	return a.exec();
}
