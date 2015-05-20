#include <QtTest/QTest>
#include <QSignalSpy>
#include "cacherest.h"

class ServerTest: public QObject
{
	Q_OBJECT
public:
	ServerTest() {
		m_cacheRest = new CacheRest(this);
	}

private slots:
	void wrongCredentials() {
		m_cacheRest->login("InvalidUser", "InvalidPassword");
		QSignalSpy spyLogin(m_cacheRest->worker, SIGNAL(repliedLogin(QString)));
		QVERIFY(spyLogin.wait());
		QCOMPARE(spyLogin.count(), 1); // make sure the signal was emitted exactly one time
		QList<QVariant> arguments = spyLogin.takeFirst();
		QVERIFY(arguments.at(0).toString() == "Moved Temporarily. Redirecting to /login");

		m_cacheRest->getUserIds();
		QSignalSpy spyIds(m_cacheRest->worker, SIGNAL(repliedIds(QString)));
		QVERIFY(spyIds.wait());
		QCOMPARE(spyIds.count(), 1); // make sure the signal was emitted exactly one time
		arguments = spyIds.takeFirst();
		QVERIFY(arguments.at(0).toString() == "{\"error\":\"Not authenticated for this route.\"}");
	}

	void getIds() {
		m_cacheRest->login("cache-bot", ")E[ls$=1IC1A$}Boji'W@zOX_<H<*n");
		QSignalSpy spyLogin(m_cacheRest->worker, SIGNAL(repliedLogin(QString)));
		QVERIFY(spyLogin.wait());
		QCOMPARE(spyLogin.count(), 1); // make sure the signal was emitted exactly one time
		QList<QVariant> arguments = spyLogin.takeFirst();
		QVERIFY(arguments.at(0).toString() == "Moved Temporarily. Redirecting to /login");

		m_cacheRest->getUserIds();
		QSignalSpy spyIds(m_cacheRest->worker, SIGNAL(repliedIds(QString)));
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
		m_cacheRest->getUserData("552d7ba7be082c0300169ed5");
		QSignalSpy spyUserData(m_cacheRest->worker, SIGNAL(repliedUserData(QString)));
		QVERIFY(spyUserData.wait(10000));
		QCOMPARE(spyUserData.count(), 1);
		QList<QVariant> arguments = spyUserData.takeFirst();
		qDebug() << arguments.at(0).toString();
		QVERIFY(arguments.at(0).toString().startsWith("{\"error\":\"No banks for this user.\"}"));
	}

	void getUseraData() {
		m_cacheRest->getUserData("552d7ba7be082c0300169ed5");
		QSignalSpy spyUserData(m_cacheRest->worker, SIGNAL(repliedUserData(QString)));
		QVERIFY(spyUserData.wait(10000));
		QCOMPARE(spyUserData.count(), 1);
		QList<QVariant> arguments = spyUserData.takeFirst();
		m_userData = arguments.at(0).toString();
		QVERIFY(m_userData.startsWith("{\"accounts\":"));
	}

	void injectData() {
		m_testUser = m_cacheRest->newUser("552d7ba7be082c0300169ed5");
		QSignalSpy spyUserData(m_cacheRest->worker, SIGNAL(repliedUserData(QString)));
		QVERIFY(spyUserData.wait(10000));
		QSignalSpy spyInjectUser(m_testUser, SIGNAL(injected()));
		QVERIFY(spyInjectUser.wait(10000));
		//QVERIFY(m_userData.("{\"accounts\":"));
	}

	void sendExtraCash() {
		//m_testUser->sendExtraCash
	}

private:
	CacheRest* m_cacheRest = 0;
	QString m_userIds;
	QString m_userData;
	User* m_testUser = 0;
};

