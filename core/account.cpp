#include "account.h"

Account::Account(QJsonObject jsonAcc, QObject *parent)
	:DBobj(jsonAcc["_id"].toString(), parent)
{
	loadJsonData(jsonAcc);
}

void Account::loadJsonData(QJsonObject json, int afterJday, int beforeJday)
{
	QString accountID = json["_id"].toString();
	m_plaidId = json["plaid_id"].toString();
	int accountLast4Digits = json["meta"].toObject()["number"].toInt();
	QString accountName = json["meta"].toObject()["name"].toString();
	QString accountType = json["type"].toString();
	Q_ASSERT(!accountID.isEmpty());
	LOG() << "read account:" << accountID << ": " << accountName << "(" << accountLast4Digits << "): " << accountType << endl;

	// quick and dirty account type
	QString metaName = json["meta"].toObject()["name"].toString();
	if (metaName.contains("saving", Qt::CaseInsensitive)) {
		m_type = Type::Saving;
	}
	if (metaName.contains("checking", Qt::CaseInsensitive)) {
		m_type = Type::Checking;
	}
	if (metaName.contains("credit", Qt::CaseInsensitive)) {
		m_type = Type::Credit;
	}

//	predictedTransactions().read(json["predicted"].toArray());
//
//	// make a bundle of all the transactions
//	m_allTrans.clear();
//	for (int i = 0; i < m_allTransactions.count(); ++i) {
//		m_allTrans.append(&m_allTransactions.transArray()[i]);
//	}
//	makeHashBundles();
}

bool Account::loadPlaidJson(QString jsonFile, int afterJday, int beforeJday) {
	m_jsonFilePath = jsonFile;
	QFile loadFile(m_jsonFilePath);
	if (!loadFile.open(QIODevice::ReadOnly)) {
		qWarning(QString("Couldn't open file %1").arg(QFileInfo(loadFile).absoluteFilePath()).toUtf8());
		return false;
	}
	QByteArray jsonData = loadFile.readAll();

	QJsonDocument loadDoc(QJsonDocument::fromJson(jsonData));
	const QJsonObject& json = loadDoc.object();

	loadJsonData(json, beforeJday, afterJday);

	return true;
}

bool Account::toJson(QVector<Transaction> transactions, QString category)
{
	QFile loadFile(m_jsonFilePath);
	QByteArray saveData;
	if (!loadFile.open(QIODevice::ReadOnly)) {
		qWarning(QString("Couldn't open file %1").arg(QFileInfo(loadFile).absoluteFilePath()).toUtf8());
		//return false;
	}
	else {
		saveData = loadFile.readAll();
	}
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




