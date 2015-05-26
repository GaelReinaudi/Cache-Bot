#include "evolver.h"
#include <QCoreApplication>

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	QString jsonFile = "../../data/adelineGaelTransactions.json";
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
	Evolver w(jsonFile, afterJday, beforeJday);

	return a.exec();
}
