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
	virtual void onUserInjected();
	virtual void onBotInjected();
	virtual void onRepliedSendNewBot(QString strData);
	virtual void onRepliedBestBot(QString strData);
	virtual void onRepliedExtraCache(QString strData);

private:
	QString m_userId;
	User* m_user;

};

#endif // CACHEACCOUNTCONNECTOR_H
