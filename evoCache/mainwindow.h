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
	explicit MainWindow(QString jsonFile, QWidget *parent = 0);
	~MainWindow();

public slots:
	void handleResults(const QString &) {

	}
	void plotMask(ZoneVector vecZone);
	void clearMasks();
	void replotCharts();
	void clearList();
	void newList(QStringList strList);

signals:
	void operate(const QString &);

private:
	Ui::MainWindow *ui;
	QThread* m_evoThread;
	EvolutionSpinner* m_evoSpinner;
	Account* account;
};

#endif // MAINWINDOW_H
