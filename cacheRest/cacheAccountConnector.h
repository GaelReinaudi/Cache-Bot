#ifndef CACHEACCOUNTCONNECTOR_H
#define CACHEACCOUNTCONNECTOR_H

#include <QObject>
#include "cacherest_global.h"

#include "user.h"

class CACHERESTSHARED_EXPORT CacheAccountConnector : public QObject
{
	Q_OBJECT

public:
	CacheAccountConnector(QString userID, QJsonObject jsonArgs = QJsonObject());
	~CacheAccountConnector();

	QString userID() const { return m_userId; }
	User* user() const { return m_user; }
	const QJsonObject& jsonArgs() const {  return m_jsonArgs; }

protected slots:
	virtual void onLoggedIn(bool didLogin);
	virtual void onUserInjected(User* pUser);
	virtual void onBotInjected(Bot *bestBot);
	virtual void onRepliedSendNewBot(QString strData);
	virtual void onRepliedBestBot(QString strData);
	virtual void onRepliedSendExtraCache(QString strData);

signals:
	void injected(User* pUser);
	void botInjected(Bot* bestBot);

private:
	QString m_userId;
	QJsonObject m_jsonArgs;
	User* m_user;

public:
	enum { None = 0x0
		   , AutoExit = 0x1
		   , SendExtraCash = 0x2
		   , SendBot = 0x4
	};
	int flags = AutoExit | SendExtraCash | SendBot;
};

#endif // CACHEACCOUNTCONNECTOR_H
