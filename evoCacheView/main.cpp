#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QString jsonFileOrUser = "../../data/adelineGaelTransactions.json";
	QVector<int> onlyLoadHashes;
	int beforeJday = 0;
	if(argc > 1) {
		jsonFileOrUser = argv[1];
		for (int i = 2; i < argc; ++i) {
			onlyLoadHashes.append(QString(argv[i]).toInt());
		}
	}
	MainWindow w(jsonFileOrUser, onlyLoadHashes);
	w.show();

	return a.exec();
}
