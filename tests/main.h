#include <QtTest/QTest>
#include <QSignalSpy>
#include "cacherest.h"

#define TEST_USER_ID_1 "55518f01574600030092a822" // cache-bot
#define TEST_USER_ID_2 "556502390fbee50300e6d07c" // chris
#define TEST_USER_ID_3 "55653f1f2dea2e0300e39b64" // gael

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
		QCOMPARE(arguments.at(0).toString()
				 , StringLoggedInReplyFailure);

		CacheRest::Instance()->getUserIds();
		QSignalSpy spyIds(CacheRest::Instance()->worker, SIGNAL(repliedIds(QString)));
		QVERIFY(spyIds.wait());
		QCOMPARE(spyIds.count(), 1); // make sure the signal was emitted exactly one time
		arguments = spyIds.takeFirst();
		QCOMPARE(arguments.at(0).toString()
				 , QString("{\"error\":\"Not authenticated for this route.\"}"));
	}

	void loginCacheBot() {
		CacheRest::Instance()->login();
		QSignalSpy spyLogin(CacheRest::Instance()->worker, SIGNAL(repliedLogin(QString)));
		bool loggedIn = spyLogin.wait();
		QList<QVariant> arguments = spyLogin.takeFirst();
		QVERIFY(loggedIn);
		QCOMPARE(arguments.at(0).toString()
				 , StringLoggedInReplySuccess);
	}

	void getIds() {
		CacheRest::Instance()->getUserIds();
		QSignalSpy spyIds(CacheRest::Instance()->worker, SIGNAL(repliedIds(QString)));
		QVERIFY(spyIds.wait());
		QCOMPARE(spyIds.count(), 1);
		QList<QVariant> arguments = spyIds.takeFirst();
		m_userIds = arguments.at(0).toString();
		QSTARTSWITH(arguments.at(0).toString()
					, "{\"user_ids\":[");
	}

	void verifyChrisId() {
		QVERIFY(m_userIds.contains(TEST_USER_ID_2));
	}

	void verifyGaelId() {
		QVERIFY(m_userIds.contains(TEST_USER_ID_3));
	}

	void NoUserData() {
		CacheRest::Instance()->getUserData("NotAnExistingUser");
		QSignalSpy spyNoUserData(CacheRest::Instance()->worker, SIGNAL(repliedUserData(QString)));
		QVERIFY(spyNoUserData.wait(10000));
		QCOMPARE(spyNoUserData.count(), 1);
		QList<QVariant> arguments = spyNoUserData.takeFirst();
		QSTARTSWITH(arguments.at(0).toString()
					, "{\"error\":{\"message\":\"Cast to ObjectId failed");
	}

	void userNoBank() {
		CacheRest::Instance()->getUserData(TEST_USER_ID_1);
		QSignalSpy spyUserData(CacheRest::Instance()->worker, SIGNAL(repliedUserData(QString)));
		QVERIFY(spyUserData.wait(10000));
		QCOMPARE(spyUserData.count(), 1);
		QList<QVariant> arguments = spyUserData.takeFirst();
		//qDebug() << arguments.at(0).toString();
		QSTARTSWITH(arguments.at(0).toString()
					, "{\"user\":");
	}

	void getUseraData() {
		CacheRest::Instance()->getUserData(TEST_USER_ID_3);
		QSignalSpy spyUserData(CacheRest::Instance()->worker, SIGNAL(repliedUserData(QString)));
		QVERIFY(spyUserData.wait(10000));
		QCOMPARE(spyUserData.count(), 1);
		QList<QVariant> arguments = spyUserData.takeFirst();
		m_userData = arguments.at(0).toString();
		QSTARTSWITH(m_userData
					, "{\"user\":");
	}

	void injectData() {
		m_testUser = CacheRest::Instance()->newUser(TEST_USER_ID_2);
		QSignalSpy spyInjectData(CacheRest::Instance()->worker, SIGNAL(repliedUserData(QString)));
		QVERIFY(spyInjectData.wait(10000));
	}

	void sendExtraCash() {
		double extra = 20.0;
		CacheRest::Instance()->sendExtraCash(m_testUser->id(), extra);
		QSignalSpy spyExtraCash(CacheRest::Instance()->worker, SIGNAL(repliedExtraCache(QString)));
		QVERIFY(spyExtraCash.wait(10000));
	}

	void sendNewBot() {
		QJsonObject json;
		json.insert("features", QJsonArray());
		CacheRest::Instance()->sendNewBot(m_testUser->id(), json);
		QSignalSpy spyExtraCash(CacheRest::Instance()->worker, SIGNAL(repliedSendNewBot(QString)));
		QVERIFY(spyExtraCash.wait(10000));
	}

	void getBestBot() {
		CacheRest::Instance()->getBestBot(TEST_USER_ID_3);
		QSignalSpy spyBestBot(CacheRest::Instance()->worker, SIGNAL(repliedBestBot(QString)));
		QVERIFY(spyBestBot.wait(10000));
		QCOMPARE(spyBestBot.count(), 1);
		QList<QVariant> arguments = spyBestBot.takeFirst();
		QString bestBotJson = arguments.at(0).toString();
		QVERIFY(bestBotJson.contains("features\":"));
	}

private:
	QString m_userIds;
	QString m_userData;
	User* m_testUser = 0;
};

