#include "user.h"
#include "account.h"

void User::injectJsonData(QString jsonStr)
{
	qDebug() << "injecting" << jsonStr.left(1024);
	QJsonDocument jsonDoc(QJsonDocument::fromJson(jsonStr.toUtf8()));
	const QJsonObject& jsonObj = jsonDoc.object();

	QFile sampleReturn("jsonData.json");
	sampleReturn.open(QFile::WriteOnly | QFile::Truncate);
	QTextStream fileout(&sampleReturn);
	fileout << jsonDoc.toJson(QJsonDocument::Indented);

	//////// "user"
	QJsonObject jsonUser = jsonObj["user"].toObject();
	//qDebug() << jsonUser["_id"].toString() << id();
	Q_ASSERT(jsonUser["_id"].toString() == id());

	//////// "banks"
	QJsonArray jsonBankArray = jsonObj["banks"].toArray();
	for (int iB = 0; iB < jsonBankArray.size(); ++iB) {
		QJsonObject jsonBank = jsonBankArray[iB].toObject();
		Bank* pBank = new Bank(jsonBank, this);
		m_banks.push_back(pBank);
	}

	//////// "accounts"
	QJsonArray jsonAccountArray = jsonObj["accounts"].toArray();
	qDebug() << jsonAccountArray.size();
	for (int iA = 0; iA < jsonAccountArray.size(); ++iA) {
		QJsonObject jsonAcc = jsonAccountArray[iA].toObject();
		QString bankTok = jsonAcc["access_token"].toString();
		Account* pAcc = new Account(jsonAcc, this);
		m_accounts.push_back(pAcc);
	}
	emit injected();
}

void User::injectJsonBot(QString jsonStr)
{

}

Account *User::globalAccount() const
{
	return m_globalAccount;
}


