#ifndef USER_H
#define USER_H

#include "common.h"

class Account : public DBobj
{
public:
	Account() {}

private:

};

class Bank : public DBobj
{
public:
	Bank() {}

private:
	QList<Account> m_accounts;
};

class User : public DBobj
{
public:
	User() {}

private:
	QList<Bank> m_banks;
};

#endif // USER_H
