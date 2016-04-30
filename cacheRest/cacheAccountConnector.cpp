#include "cacheAccountConnector.h"
#include "account.h"
#include "cacherest.h"

CacheAccountConnector::CacheAccountConnector(QString userID, QJsonObject jsonArgs)
	: QObject()
	, m_userId(userID)
	, m_jsonArgs(jsonArgs)
{
	CREATE_LOGGER(userID);

	WARN() << "Transaction::actualCurrentDayTime()" << Transaction::actualCurrentDayTime().toString();
	WARN() << "Transaction::actualCurrentDay()" << Transaction::actualCurrentDay().toString();
	WARN() << "Transaction::currentDay()" << Transaction::currentDay().toString();
	WARN() << "jsonArgs: " << QString(QJsonDocument(m_jsonArgs).toJson(/*QJsonDocument::Compact*/));

	if (jsonArgs["NoSend"].toString() != "") {
		flags &= ~CacheAccountConnector::SendExtraCash;
	}

	QJsonArray onlyH = jsonArgs["onlyHashes"].toArray();
	for (int i = 0; i < onlyH.count(); ++i) {
		Transaction::onlyLoadHashes.append(onlyH[i].toInt());
	}

	QJsonArray onlyC = jsonArgs["plaidCat"].toArray();
	for (int i = 0; i < onlyC.count(); ++i) {
		Transaction::onlyPlaidCat.append(onlyC[i].toInt());
	}

	QString argCat = jsonArgs["category"].toString();
	NOTICE() << "category: " << argCat;

	// loading only the main category transaction
	if (!argCat.isEmpty()) {
		Transaction::makeCatRegExps(argCat);
	}

	// login as cache-bot
	CacheRest::Instance()->login();
	connect(CacheRest::Instance()->worker, SIGNAL(loggedIn(bool)), this, SLOT(onLoggedIn(bool)));
}

CacheAccountConnector::~CacheAccountConnector()
{
	delete m_user;
}

void CacheAccountConnector::onLoggedIn(bool didLogin)
{
	if(didLogin) {
		m_user = new User(userID(), m_jsonArgs);
		CacheRest::Instance()->getUserData(userID(), m_user);
		connect(m_user, SIGNAL(injected(User*)), this, SLOT(onUserInjected(User*)));
		connect(m_user, SIGNAL(injected(User*)), this, SIGNAL(injected(User*)));
		connect(m_user, SIGNAL(botInjected(Bot*)), this, SLOT(onBotInjected(Bot*)));
		connect(m_user, SIGNAL(botInjected(Bot*)), this, SIGNAL(botInjected(Bot*)));
		connect(CacheRest::Instance()->worker, SIGNAL(repliedBestBot(QString)), this, SLOT(onRepliedBestBot(QString)));
		connect(CacheRest::Instance()->worker, SIGNAL(repliedSendExtraCache(QString)), this, SLOT(onRepliedSendExtraCache(QString)));
	}
	else {
		qWarning() << "could not log into the server";
	}
}

void CacheAccountConnector::onUserInjected(User* pUser)
{
	NOTICE() << "CacheAccountConnector has injected user: " << pUser->email();

	double checkingBal = user()->balance(Account::Type::Checking);
	NOTICE() << "checkingBal " << checkingBal << " (Un/Ch/Sa/Cr "
			 << user()->balance(Account::Type::Unknown) << " "
			 << user()->balance(Account::Type::Checking) << " "
			 << user()->balance(Account::Type::Saving) << " "
			 << user()->balance(Account::Type::Credit) << " "
			 << " )";
}

void CacheAccountConnector::onBotInjected(Bot* bestBot)
{
	Q_UNUSED(bestBot);
	qDebug() << "CacheAccountConnector has a bot injected object";
}

void CacheAccountConnector::onRepliedSendNewBot(QString strData)
{
	Q_UNUSED(strData);
}

void CacheAccountConnector::onRepliedBestBot(QString strData)
{
	Q_UNUSED(strData);
}

void CacheAccountConnector::onRepliedSendExtraCache(QString strData)
{
	Q_UNUSED(strData);
}
