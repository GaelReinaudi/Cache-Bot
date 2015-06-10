#ifndef USER_H
#define USER_H

#include "core_global.h"
#include "common.h"
#include "transaction.h"
#include "account.h"
//class Account;

class CORESHARED_EXPORT Bank : public DBobj
{
public:
	Bank(QJsonObject jsonBank, QObject* parent = 0)
		:DBobj(jsonBank["_id"].toString(), parent)
	{
		Q_ASSERT(!jsonBank["_id"].toString().isEmpty());
		m_accessToken = jsonBank["_id"].toString();
	}
	QString accessToken() const { return m_accessToken; }

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

	double costLiving(double withinPercentileCost);

public slots:
	void injectJsonData(QString jsonStr);
	void injectJsonBot(QString jsonStr);

protected:
	Bank* getBankByToken(QString bankTok) const {
		for (Bank* pB : m_banks) {
			if(pB->accessToken() == bankTok)
				return pB;
		}
		return 0;
	}
	Account* getAccountByPlaidId(QString plaidId) const {
		for (Account* pA : m_accounts) {
			if(pA->plaidId() == plaidId)
				return pA;
		}
		return 0;
	}

signals:
	void injected();

private:
	QVector<Bank*> m_banks;
	QVector<Account*> m_accounts;
	StaticTransactionArray m_allTransactions;
};

#endif // USER_H
