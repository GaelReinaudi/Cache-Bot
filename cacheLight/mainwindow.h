#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDate>
#include "core/acdata.h"
#include "qcustomplot.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();
	bool wasPredicted(Transaction& trans);
	void keyPressEvent(QKeyEvent *event);
	int computeMinSlopeOver(int numDays);

protected slots:
	void updateChart();
	void makePredictiPlot();
	void makePastiPlot();
	void makeMinSlope();
	void onWheelEvent(QWheelEvent*wEv);

private:
	Ui::MainWindow *ui;
	QDate m_date;
	int m_d0 = 0;
	double m_lastBal = 0.0;
	double m_slushThreshold = 0.0;
	double m_minSlope = 0.0;
	double m_extraToday = -1.0;
	int m_ipb = 0;
	// account to read predicted transactions from
	Account m_account;
	QCPBars *pBars = 0;
};

#endif // MAINWINDOW_H
