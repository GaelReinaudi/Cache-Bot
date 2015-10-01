#include "evolver.h"
#include <QCoreApplication>

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	QString jsonFileOrUser = "../../data/adelineGaelTransactions.json";
	QString onlyAccount;
	if(argc > 1) {
		jsonFileOrUser = argv[1];
	}
	if(argc > 2) {
		onlyAccount = argv[2];
	}
	Evolver w(jsonFileOrUser, onlyAccount);

	return a.exec();
}
