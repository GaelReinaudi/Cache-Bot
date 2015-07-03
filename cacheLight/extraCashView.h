#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDate>
#include "user.h"
#include "qcustomplot.h"
#include "cacherest.h"
#include <QNetworkCookieJar>

class ExtraCache;

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

protected slots:
	void init();
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
		request.setUrl(QUrl("https://cache-heroku.herokuapp.com:443/cache-bot/data/55496831ceb5b20300ea0cf1"));
		request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

		QNetworkReply *reply2 = manager->post(request, "");
		connect(reply2, SIGNAL(finished()), this, SLOT(slotReadyRead2()));
	}
	void slotReadyRead2() {
		QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
		if (!reply) {
			qDebug("no reply yet");
			return;
		}
		qDebug("slotReadyRead2 with reply");
		QByteArray jsonData = reply->readAll();
		qDebug(jsonData);
		return;
	}
private:
	QNetworkAccessManager *manager = 0;

private:
	Ui::ExtraCashView *ui;
	ExtraCache* m_pExtraCache = 0;
	QDate m_date;
	int m_d0 = 0;
	double m_lastBal = 0.0;
	double m_extraToday = -1.0;
	int m_ipb = 0;
	// account to read predicted transactions from
//	User* m_pUser;
	QCPBars *pBars = 0;
};

#endif // MAINWINDOW_H
