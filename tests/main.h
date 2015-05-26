#include <QtTest/QTest>
#include <QSignalSpy>
#include "cacherest.h"

#define TEST_USER_ID_1 "55518f01574600030092a822" // cache-bot
#define TEST_USER_ID_2 "55518f01574600030092a822"

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
		QVERIFY(arguments.at(0).toString() == StringLoggedInReply);

		CacheRest::Instance()->getUserIds();
		QSignalSpy spyIds(CacheRest::Instance()->worker, SIGNAL(repliedIds(QString)));
		QVERIFY(spyIds.wait());
		QCOMPARE(spyIds.count(), 1); // make sure the signal was emitted exactly one time
		arguments = spyIds.takeFirst();
		QVERIFY(arguments.at(0).toString() == "{\"error\":\"Not authenticated for this route.\"}");
	}

	void loginCacheBot() {
		CacheRest::Instance()->login();
		QSignalSpy spyLogin(CacheRest::Instance()->worker, SIGNAL(repliedLogin(QString)));
		bool loggedIn = spyLogin.wait();
		QList<QVariant> arguments = spyLogin.takeFirst();
		QVERIFY(loggedIn);
		QVERIFY(arguments.at(0).toString() == StringLoggedInReply);
	}

	void getIds() {
		CacheRest::Instance()->getUserIds();
		QSignalSpy spyIds(CacheRest::Instance()->worker, SIGNAL(repliedIds(QString)));
		QVERIFY(spyIds.wait());
		QCOMPARE(spyIds.count(), 1);
		QList<QVariant> arguments = spyIds.takeFirst();
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
		CacheRest::Instance()->getUserData(TEST_USER_ID_1);
		QSignalSpy spyUserData(CacheRest::Instance()->worker, SIGNAL(repliedUserData(QString)));
		QVERIFY(spyUserData.wait(10000));
		QCOMPARE(spyUserData.count(), 1);
		QList<QVariant> arguments = spyUserData.takeFirst();
		qDebug() << arguments.at(0).toString();
		QVERIFY(arguments.at(0).toString().startsWith("{\"error\":{}"));
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
		QSignalSpy spyExtraCash(CacheRest::Instance()->worker, SIGNAL(repliedExtraCache(QString)));
		QVERIFY(spyExtraCash.wait(10000));
	}

private:
//	CacheRest* m_cacheRest = 0;
	QString m_userIds;
	QString m_userData;
	User* m_testUser = 0;
};

