#include "account.h"

void Account::loadJsonData(QByteArray jsonData, int afterJday, int beforeJday)
{
//	qDebug() << jsonData;
//	QFile writeFile("test.out");
//	if (!writeFile.open(QIODevice::WriteOnly|QIODevice::Append)) {
//		qWarning(QString("Couldn't open file %1").arg(QFileInfo(writeFile).absoluteFilePath()).toUtf8());
//		return;
//	}
//	writeFile.write(jsonData);
//	writeFile.close();
//	return;

	QJsonDocument loadDoc(QJsonDocument::fromJson(jsonData));
	const QJsonObject& json = loadDoc.object();

	QJsonArray npcArrayAccount = json["accounts"].toArray();
	qDebug() << npcArrayAccount.size();
	for (int npcIndex = 0; npcIndex < npcArrayAccount.size(); ++npcIndex) {
		QJsonObject npcObject = npcArrayAccount[npcIndex].toObject();
		QString accountID = npcObject["_id"].toString();
		Q_ASSERT(!accountID.isEmpty());
		if (accountID != "")
			m_accountIds.push_back(accountID);
	}
	qDebug() << m_accountIds;
	m_allTransactions.read(json["transactions"].toArray(), afterJday, beforeJday, m_accountIds);

	m_predicted.read(json["predicted"].toArray());

	//m_transactions.cleanSymetricTransaction();

	// make a bundle of all the transactions
	m_allTrans.clear();
	for (int i = 0; i < m_allTransactions.count(); ++i) {
		m_allTrans.append(&m_allTransactions.transArray()[i]);
	}
	makeHashBundles();
}

bool Account::loadPlaidJson(QString jsonFile, int afterJday, int beforeJday) {
	m_jsonFilePath = jsonFile;
	QFile loadFile(m_jsonFilePath);
	if (!loadFile.open(QIODevice::ReadOnly)) {
		qWarning(QString("Couldn't open file %1").arg(QFileInfo(loadFile).absoluteFilePath()).toUtf8());
		return false;
	}
	QByteArray jsonData = loadFile.readAll();
	loadJsonData(jsonData, beforeJday, afterJday);

	return true;
}

bool Account::toJson(QVector<Transaction> transactions, QString category)
{
	QFile loadFile(m_jsonFilePath);
	if (!loadFile.open(QIODevice::ReadOnly)) {
		qWarning(QString("Couldn't open file %1").arg(QFileInfo(loadFile).absoluteFilePath()).toUtf8());
		return false;
	}
	QByteArray saveData = loadFile.readAll();
	QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
	QJsonObject json = loadDoc.object();

	QJsonArray transArray;
	for (int i = 0; i < transactions.count(); ++i) {
		QJsonObject obj;
		transactions[i].write(obj);
		transArray.append(obj);
	}
	json[category] = transArray;
	qDebug() << transactions.count();
	//qDebug() << json["predicted"];

	QFile writeFile(m_jsonFilePath + ".out");
	if (!writeFile.open(QIODevice::WriteOnly)) {
		qWarning(QString("Couldn't open file %1").arg(QFileInfo(writeFile).absoluteFilePath()).toUtf8());
		return false;
	}
	QJsonDocument saveDoc(json);
	writeFile.write(saveDoc.toJson());
	return true;
}

double Account::costLiving(double withinPercentileCost)
{
	QVector<double> costs;
	for (int i = 0; i < allTrans().count(); ++i) {
		double amnt = allTrans().trans(i).amountDbl();
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
	double numDays = firstTransactionDate().daysTo(lastTransactionDate());
	avg /= numDays;
	qDebug() << "cost of living (L="<<lastCostsInd<<")" << avg;
	return avg;
}

void Account::Transactions::read(const QJsonArray& npcArray, int afterJday, int beforeJday, const QVector<QString> &onlyAcIds /*= anyID*/) {
	clear();
	for (int npcIndex = 0; npcIndex < npcArray.size(); ++npcIndex) {
		QJsonObject npcObject = npcArray[npcIndex].toObject();
		QString accountTrans = npcObject["_account"].toString();
		if (onlyAcIds.isEmpty() || onlyAcIds.contains(accountTrans)) {
			appendNew()->read(npcObject);
			if (last()->jDay() < afterJday
					|| (beforeJday && last()->jDay() > beforeJday)
					|| last()->name.contains("Online Transfer")
					|| last()->name.contains("Credit Card Payment")
					|| last()->name.contains("ment to Chase c")
					)
				removeLast();
		}
		else {
			LOG() << "transaction not matching an account:"<< accountTrans
				  << " object:" << npcArray[npcIndex].toString() << endl;
		}
	}
	qSort(m_transArray.begin(), m_transArray.begin() + m_numTrans, Transaction::earlierThan);
	qDebug() << "transaction count" << count();
}

void Account::Transactions::write(QJsonArray& npcArray) const {
	for (int i = 0; i < count(); ++i) {
		QJsonObject npcObject;
		m_transArray[i].write(npcObject);
		npcArray.append(npcObject);
	}
}

