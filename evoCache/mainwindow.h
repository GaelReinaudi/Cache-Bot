#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include "ACChart/acustomplot.h"
#include "AmortiChart/AMPlot.h"
#include "EvolutionSpinner.h"

class EvolutionSpinner;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QString userID, int afterJday = 0, int beforeJday = 0);
	~MainWindow();

public slots:
	void handleResults(const QString &) {

	}
	void plotMask(double x, double y, bool isTarget);
	void clearMasks();
	void replotCharts();
	void clearList();
	void onNewSummarizedTree(QJsonObject jsonObj);
	void onUserInjected(User* pUser);

signals:
	void operate(const QString &);

private:
	Ui::MainWindow *ui;
	QThread* m_evoThread;
	EvolutionSpinner* m_evoSpinner;
	Account* account;
};

#endif // MAINWINDOW_H
