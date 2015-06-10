#ifndef USER_H
#define USER_H

#include "core_global.h"
#include "common.h"
class Account;

class CORESHARED_EXPORT Bank : public DBobj
{
public:
	Bank(QJsonObject jsonBank, QObject* parent = 0)
		:DBobj(jsonBank["_id"].toString(), parent)
	{
		Q_ASSERT(!jsonBank["_id"].toString().isEmpty());
		m_accessToken = jsonBank["_id"].toString();
	}

private:
	QString m_accessToken;
};

class CORESHARED_EXPORT User : public DBobj
{
	Q_OBJECT

public:
	User(QString userId, QObject* parent = 0)
		:DBobj(userId, parent)
	{}

	Account *globalAccount() const;

public slots:
	void injectJsonData(QString jsonStr);
	void injectJsonBot(QString jsonStr);

signals:
	void injected();

private:
	QList<Bank*> m_banks;
	QList<Account*> m_accounts;
	Account* m_globalAccount;
};

#endif // USER_H
