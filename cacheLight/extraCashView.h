#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDate>
#include "user.h"
#include "qcustomplot.h"
#include "cacherest.h"
#include <QNetworkCookieJar>

class ExtraCache;
class Bot;

namespace Ui {
class ExtraCashView;
}

class ExtraCashView : public QMainWindow
{
	Q_OBJECT

public:
	explicit ExtraCashView(QString userID, QWidget *parent = 0);
	~ExtraCashView();
	void keyPressEvent(QKeyEvent *event);

	void makeBalancePlot();
protected slots:
	void onBotInjected(Bot* pBot);
	void updateChart();
	void makeRevelationPlot();
	void makePastiPlot();
	void makeMinSlope();
	void onWheelEvent(QWheelEvent*wEv);
	void onHypotheTrans(int transAmount);
private:
	QNetworkAccessManager *manager = 0;

private:
	Ui::ExtraCashView *ui;
	ExtraCache* m_pExtraCache = 0;
	QDate m_pbDate;
	double m_pbBalance = 0.0;
	double m_extraToday = -1.0;
	int m_ipb = 0;
	// account to read predicted transactions from
//	User* m_pUser;
	QCPBars *pBars = 0;
};

#endif // MAINWINDOW_H
