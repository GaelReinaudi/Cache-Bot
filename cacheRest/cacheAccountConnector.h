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
	Account* account() const { return m_account; }

protected slots:
	virtual void onLoggedIn(bool didLogin);
	virtual void onRepliedUserData(QString strData);
	virtual void onRepliedSendNewBot(QString strData);
	virtual void repliedBestBot(QString strData);

private:
	QString m_userId;
	Account* m_account;

};

#endif // CACHEACCOUNTCONNECTOR_H
