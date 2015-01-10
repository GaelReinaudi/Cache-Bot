#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include "ACChart/acustomplot.h"

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
	void startEvolution(bool doStart);
	void handleResults(const QString &) {

	}

signals:
	void operate(const QString &);

private:
	Ui::MainWindow *ui;
	ACustomPlot* m_accountPlot;
	QThread* m_evoThread;
	EvolutionSpinner* m_evoSpinner;
};

#endif // MAINWINDOW_H
