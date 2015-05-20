#ifndef USER_H
#define USER_H

#include "core_global.h"
#include "common.h"
class Account;

class CORESHARED_EXPORT Bank : public DBobj
{
public:
	Bank() {}

private:
	QList<Account*> m_accounts;
};

class CORESHARED_EXPORT User : public DBobj
{
	Q_OBJECT

public:
	User(QString userId);

	void readJson();

public slots:
	void injectJsonString(QString jsonStr);

signals:
	void injected();

private:
	QList<Bank*> m_banks;
};

#endif // USER_H
