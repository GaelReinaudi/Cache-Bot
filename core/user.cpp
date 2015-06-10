#include "user.h"

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
		Bank* pBank = getBankByToken(bankTok);
		Account* pAcc = new Account(jsonAcc, pBank);
		m_accounts.push_back(pAcc);
	}

	//////// "transactions"
	QJsonArray jsonTransArray = jsonObj["transactions"].toArray();
	qDebug() << jsonTransArray.size();
//	m_allTransactions.read(jsonTransArray);
	for (int iT = 0; iT < jsonTransArray.size(); ++iT) {
		QJsonObject jsonTrans = jsonTransArray[iT].toObject();
		QString acPlaidId = jsonTrans["plaid_account"].toString();
		Account* pAccount = getAccountByPlaidId(acPlaidId);
		m_allTransactions.appendNew()->read(jsonTrans);
		m_allTransactions.last()->account = pAccount;
	}
	m_allTransactions.sort();

	//////// complete Accounts with transaction pointers
	for (int i = 0; i < m_allTransactions.count(); ++i) {
		Transaction* pT = &m_allTransactions.transArray()[i];
		pT->account->append(pT);
	}

	emit injected();
}

void User::injectJsonBot(QString jsonStr)
{

}

double User::costLiving(double withinPercentileCost)
{
	QVector<double> costs;
	for (int i = 0; i < m_allTransactions.count(); ++i) {
		double amnt = m_allTransactions.trans(i).amountDbl();
		if (amnt < 0.0) {
			costs.append(-amnt);
		}
	}
	qSort(costs);
	double avg = 0.0;
	int lastCostsInd = costs.count() * withinPercentileCost;
	for (int i = 0; i < lastCostsInd; ++i) {
		avg += costs[i];
	}
	double numDays = m_allTransactions.firstTransactionDate().daysTo(m_allTransactions.lastTransactionDate());
	if(numDays) {
		avg /= numDays;
		qDebug() << "cost of living (L="<<lastCostsInd<<")" << avg;
	}
	return avg;
}



