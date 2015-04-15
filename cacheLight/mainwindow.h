#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDate>
#include "core/acdata.h"

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

protected slots:
	void updateChart();
	void makePredictiPlot();
	void makePastiPlot();
	void onWheelEvent(QWheelEvent*wEv);

private:
	Ui::MainWindow *ui;
	QDate m_date;
	int m_d0 = 0;
	double m_lastBal = 0.0;
	int m_ipb = 0;
	// account to read predicted transactions from
	Account m_account;
};

#endif // MAINWINDOW_H
