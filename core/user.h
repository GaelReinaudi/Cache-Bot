#ifndef USER_H
#define USER_H

#include "common.h"
class Account;

class Bank : public DBobj
{
public:
	Bank()
	{}

private:
	QList<Account*> m_accounts;
};

class User : public DBobj
{
public:
	User(QString userId)
		: DBobj(userId)
	{}

	void readJson() {

	}

public slots:
	void injectJsonString(QString jsonStr);

private:
	QList<Bank*> m_banks;
};

#endif // USER_H
