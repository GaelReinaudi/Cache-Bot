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
		QENDSWITH(arguments.at(0).toString()
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
		QENDSWITH(arguments.at(0).toString()
				 , StringLoggedInReplySuccess);
	}

	void getIds() {
		CacheRest::Instance()->getUserIds();
		QSignalSpy spyIds(CacheRest::Instance()->worker, SIGNAL(repliedIds(QString)));
		QVERIFY(spyIds.wait());
		QCOMPARE(spyIds.count(), 1);
		QList<QVariant> arguments = spyIds.takeFirst();
		QString jsonString = arguments.at(0).toString();
		QSTARTSWITH(jsonString
					, "{\"user_ids\":{");
		QJsonObject jsobj = QJsonDocument::fromJson(jsonString.toUtf8()).object()["user_ids"].toObject();
		m_userIds = jsobj.keys();//jsonString.mid(jsonString.indexOf(":")).split(QRegExp("\\W+"), QString::SkipEmptyParts);
		qDebug() << "m_userIds" << m_userIds;
		QFile file("jsonAllUserIds.json");

		QJsonDocument jsonDoc(jsobj);
		QFile sampleReturn("jsonAllUserIds.json");
		sampleReturn.open(QFile::WriteOnly | QFile::Truncate);
		QTextStream fileout(&sampleReturn);
		fileout << jsonDoc.toJson(QJsonDocument::Indented);
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
					, "{\"error\":{\"stack\"");
	}

	void userNoBank() {
		CacheRest::Instance()->getUserData(TEST_USER_ID_1);
		QSignalSpy spyUserData(CacheRest::Instance()->worker, SIGNAL(repliedUserData(QString)));
		QVERIFY(spyUserData.wait(10000));
		QCOMPARE(spyUserData.count(), 1);
		QList<QVariant> arguments = spyUserData.takeFirst();
		//qDebug() << arguments.at(0).toString();
		QSTARTSWITH(arguments.at(0).toString()
					, "{\"error\":\"kNoAccountIds");
	}

	void getUseraData() {
		CacheRest::Instance()->getUserData(TEST_USER_ID_3);
		QSignalSpy spyUserData(CacheRest::Instance()->worker, SIGNAL(repliedUserData(QString)));
		QVERIFY(spyUserData.wait(10000));
		QCOMPARE(spyUserData.count(), 1);
		QList<QVariant> arguments = spyUserData.takeFirst();
		m_userData = arguments.at(0).toString();
		QSTARTSWITH(m_userData
					, "{\"transact");
	}

	void injectData() {
		m_testUser = CacheRest::Instance()->newUser(TEST_USER_ID_2);
		QSignalSpy spyInjectData(CacheRest::Instance()->worker, SIGNAL(repliedUserData(QString)));
		QVERIFY(spyInjectData.wait(10000));
	}

//	void sendExtraCash() {
//		double extra = 20.0;
//		CacheRest::Instance()->sendExtraCash(m_testUser->id(), extra);
//		QSignalSpy spyExtraCash(CacheRest::Instance()->worker, SIGNAL(repliedSendExtraCache(QString)));
//		QVERIFY(spyExtraCash.wait(10000));
//	}

	void getBestBot() {
		CacheRest::Instance()->getBestBot(TEST_USER_ID_2);
		QSignalSpy spyBestBot(CacheRest::Instance()->worker, SIGNAL(repliedBestBot(QString)));
		QVERIFY(spyBestBot.wait(10000));
		QCOMPARE(spyBestBot.count(), 1);
		QList<QVariant> arguments = spyBestBot.takeFirst();
		QString bestBotJson = arguments.at(0).toString();
		QVERIFY(bestBotJson.contains("features\":"));
	}

//	void allUsersEvoCacheComputations() {
//		for (const QString& userId : m_userIds) {
//			CacheRest::Instance()->evoCacheEC2Computation(userId);
//		}
//	}
//	void localRunAllEvoCacheView() {
//		for (const QString& userId : m_userIds) {
//			QString program = "./evoCacheView.exe";
//			QStringList arguments;
//			arguments << userId;
//			QProcess *myProcess = new QProcess(0);
//			myProcess->start(program, arguments);
//		}
//	}

	void randUsersExtraCashComputations() {
		// test a random user
		qsrand(QTime::currentTime().msecsSinceStartOfDay());
		QString randUser = "556502390fbee50300e6d07c";//m_userIds[qrand() % m_userIds.count()];
		qDebug() << "randUser " << randUser;
		CacheRest::Instance()->extraCashEC2Computation(randUser);
		QSignalSpy spyExtraCashComputation(CacheRest::Instance()->worker, SIGNAL(repliedExtraCashEC2Computation(QString)));
		QVERIFY(spyExtraCashComputation.wait(30000));
		QList<QVariant> arguments = spyExtraCashComputation.takeFirst();
		QString extraCashReply = arguments.at(0).toString();
		QVERIFY(!extraCashReply.isEmpty());
		qDebug() << "extraCashReply:" << endl << extraCashReply.left(256) << endl;
	}

private:
	QStringList m_userIds;
	QString m_userData;
	User* m_testUser = 0;
};

