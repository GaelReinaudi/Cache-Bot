#include "metricViewer.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QString jsonFileOrUser = "../../data/adelineGaelTransactions.json";
	if(argc > 1) {
		jsonFileOrUser = argv[1];
	}
	MetricViewer w(jsonFileOrUser);
	w.show();

	return a.exec();
}
