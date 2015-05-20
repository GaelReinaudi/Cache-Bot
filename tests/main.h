#include <QtTest/QTest>
#include <QSignalSpy>
#include "cacherest.h"

class ServerTest: public QObject
{
	Q_OBJECT
public:
	ServerTest() {
		CacheRest::Instance(this);
	}

private slots:
	void wrongCredentials() {
		CacheRest::Instance()->login("InvalidUser", "InvalidPassword");
		QSignalSpy spyLogin(CacheRest::Instance()->worker, SIGNAL(repliedLogin(QString)));
		QVERIFY(spyLogin.wait());
		QCOMPARE(spyLogin.count(), 1); // make sure the signal was emitted exactly one time
		QList<QVariant> arguments = spyLogin.takeFirst();
		QVERIFY(arguments.at(0).toString() == "Moved Temporarily. Redirecting to /login");

		CacheRest::Instance()->getUserIds();
		QSignalSpy spyIds(CacheRest::Instance()->worker, SIGNAL(repliedIds(QString)));
		QVERIFY(spyIds.wait());
		QCOMPARE(spyIds.count(), 1); // make sure the signal was emitted exactly one time
		arguments = spyIds.takeFirst();
		QVERIFY(arguments.at(0).toString() == "{\"error\":\"Not authenticated for this route.\"}");
	}

	void getIds() {
		CacheRest::Instance()->login("cache-bot", ")E[ls$=1IC1A$}Boji'W@zOX_<H<*n");
		QSignalSpy spyLogin(CacheRest::Instance()->worker, SIGNAL(repliedLogin(QString)));
		QVERIFY(spyLogin.wait());
		QCOMPARE(spyLogin.count(), 1); // make sure the signal was emitted exactly one time
		QList<QVariant> arguments = spyLogin.takeFirst();
		QVERIFY(arguments.at(0).toString() == "Moved Temporarily. Redirecting to /login");

		CacheRest::Instance()->getUserIds();
		QSignalSpy spyIds(CacheRest::Instance()->worker, SIGNAL(repliedIds(QString)));
		QVERIFY(spyIds.wait());
		QCOMPARE(spyIds.count(), 1);
		arguments = spyIds.takeFirst();
		m_userIds = arguments.at(0).toString();
		QVERIFY(arguments.at(0).toString().startsWith("{\"user_ids\":["));
	}

	void verifyChrisId() {
		QVERIFY(m_userIds.contains("552d7ba7be082c0300169ed5"));
	}

	void verifyGaelId() {
		QVERIFY(m_userIds.contains("55496831ceb5b20300ea0cf1"));
	}


	void userNoBank() {
		CacheRest::Instance()->getUserData("552d7ba7be082c0300169ed5");
		QSignalSpy spyUserData(CacheRest::Instance()->worker, SIGNAL(repliedUserData(QString)));
		QVERIFY(spyUserData.wait(10000));
		QCOMPARE(spyUserData.count(), 1);
		QList<QVariant> arguments = spyUserData.takeFirst();
		qDebug() << arguments.at(0).toString();
		QVERIFY(arguments.at(0).toString().startsWith("{\"error\":\"No banks for this user.\"}"));
	}

	void getUseraData() {
		CacheRest::Instance()->getUserData("552d7ba7be082c0300169ed5");
		QSignalSpy spyUserData(CacheRest::Instance()->worker, SIGNAL(repliedUserData(QString)));
		QVERIFY(spyUserData.wait(10000));
//		QCOMPARE(spyUserData.count(), 1);
		QList<QVariant> arguments = spyUserData.takeFirst();
		m_userData = arguments.at(0).toString();
//		QVERIFY(m_userData.startsWith("{\"accounts\":"));
	}

	void injectData() {
		m_testUser = CacheRest::Instance()->newUser("552d7ba7be082c0300169ed5");
		QSignalSpy spyUserData(CacheRest::Instance()->worker, SIGNAL(repliedUserData(QString)));
		QVERIFY(spyUserData.wait(10000));
		//QVERIFY(m_userData.("{\"accounts\":"));
	}

	void sendExtraCash() {
		double extra = 20.0;
		CacheRest::Instance()->sendExtraCash(m_testUser->id(), extra);
		QSignalSpy spyExtraCash(CacheRest::Instance()->worker, SIGNAL(repliedUserData(QString)));
		QVERIFY(spyExtraCash.wait(10000));
	}

private:
//	CacheRest* m_cacheRest = 0;
	QString m_userIds;
	QString m_userData;
	User* m_testUser = 0;
};

