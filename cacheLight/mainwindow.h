#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDate>
#include "core/acdata.h"
#include "qcustomplot.h"
#include "cacherest.h"

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

	void init();
protected slots:
	void updateChart();
	void makePredictiPlot();
	void makePastiPlot();
	void makeMinSlope();
	void onWheelEvent(QWheelEvent*wEv);
private slots:
	void slotReadyRead() {
		QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
		if (!reply) {
			qDebug("no reply yet");
			return;
		}
		qDebug("slotReadyRead with reply");
		qDebug(reply->readAll());
		// curl -ipv4 --insecure --cookie jarfile -H "Accept: application/json" -X GET https://cache-heroku.herokuapp.com:443/bank/f202f5004003ff51b7cc7e60523b7a43d541b38246c4abc0b765306e977126540f731d94478de121c44d5c214382d36cb3c1f3c4e117a532fc78a8b078c320bb24f671bbd0199ea599c15349d2b3d820
		QNetworkRequest request;
		request.setUrl(QUrl("https://cache-heroku.herokuapp.com:443/bank/f202f5004003ff51b7cc7e60523b7a43d541b38246c4abc0b765306e977126540f731d94478de121c44d5c214382d36cb3c1f3c4e117a532fc78a8b078c320bb24f671bbd0199ea599c15349d2b3d820"));

		QNetworkReply *reply2 = manager->get(request);
		connect(reply2, SIGNAL(readyRead()), this, SLOT(slotReadyRead2()));
	}
	void slotReadyRead2() {
		QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
		if (!reply) {
			qDebug("no reply yet");
			return;
		}
		qDebug("slotReadyRead2 with reply");
		qDebug(reply->readAll());
	}
private:
	QNetworkAccessManager *manager = 0;

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
