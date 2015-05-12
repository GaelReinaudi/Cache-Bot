#include <QtTest/QTest>
#include "cacherest.h"

class MyFirstTest: public QObject
{
	Q_OBJECT
private slots:
	void testGET() {
		// curl -ipv4 --insecure --cookie-jar jarfile -d "email=gael.reinaudi@gmail.com&password=wwwwwwww" -X POST https://cache-heroku.herokuapp.com/login
		// curl -ipv4 --insecure --cookie jarfile -H "Accept: application/json" -X GET https://cache-heroku.herokuapp.com:443/bank/f202f5004003ff51b7cc7e60523b7a43d541b38246c4abc0b765306e977126540f731d94478de121c44d5c214382d36cb3c1f3c4e117a532fc78a8b078c320bb24f671bbd0199ea599c15349d2b3d820
		QNetworkAccessManager *manager = new QNetworkAccessManager(this);
		QNetworkRequest request;
		request.setUrl(QUrl("https://cache-heroku.herokuapp.com/login"));

		QNetworkReply *reply = manager->get(request);
		connect(reply, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
		connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
				this, SLOT(slotError(QNetworkReply::NetworkError)));
		connect(reply, SIGNAL(sslErrors(QList<QSslError>)),
				this, SLOT(slotSslErrors(QList<QSslError>)));	}

	void myFirstTest()
	{ QVERIFY(1 == 1); }
	void mySecondTest()
	{ QVERIFY(1 != 2); }
	void cleanupTestCase()
	{ qDebug("called after myFirstTest and mySecondTest"); }

private slots:
	void slotReadyRead() {
		QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
		if (reply) {
			qDebug(reply->readAll());
		}
	}
};
