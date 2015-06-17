#ifndef CACHEACCOUNTCONNECTOR_H
#define CACHEACCOUNTCONNECTOR_H

#include <QObject>
#include "cacherest_global.h"

#include "user.h"

class CACHERESTSHARED_EXPORT CacheAccountConnector : public QObject
{
	Q_OBJECT

public:
	CacheAccountConnector(QString userID);
	~CacheAccountConnector();

	QString userID() const { return m_userId; }
	User* user() const { return m_user; }

protected slots:
	virtual void onLoggedIn(bool didLogin);
	virtual void onUserInjected(User* pUser);
	virtual void onBotInjected();
	virtual void onRepliedSendNewBot(QString strData);
	virtual void onRepliedBestBot(QString strData);
	virtual void onRepliedExtraCache(QString strData);

signals:
	void injected(User* pUser);
	void botInjected();

private:
	QString m_userId;
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
