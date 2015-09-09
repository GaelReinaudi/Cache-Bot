#include "account.h"

Account::Account(QJsonObject jsonAcc, QObject *parent)
	:DBobj(jsonAcc["_id"].toString(), parent)
{
	loadJsonData(jsonAcc);

//	m_excludeNameTransContain.append("Transfer");
//	m_excludeNameTransContain.append("Credit Card Payment");
//	m_excludeNameTransContain.append("ment to Chase c");
//	m_excludeNameTransContain.append("OVERDRAFT PROT");
//	m_excludeNameTransContain.append("payment to CRD");
//	m_excludeNameTransContain.append("online payment from CHK");
}

void Account::loadJsonData(QJsonObject json)
{
	QString accountID = json["_id"].toString();
	m_plaidId = json["plaid_id"].toString();
	int accountLast4Digits = json["meta"].toObject()["number"].toInt();
	QString accountName = json["meta"].toObject()["name"].toString();
	QString accountType = json["type"].toString();
	Q_ASSERT(!accountID.isEmpty());
	NOTICE() << "read account:" << accountID << ": " << accountName
			 << "(" << accountLast4Digits << "): " << accountType
			 << ". plaid " << m_plaidId;

	// quick and dirty account type
	QString metaName = json["meta"].toObject()["name"].toString();
	QString type = json["type"].toString();
	if (metaName.contains("saving", Qt::CaseInsensitive)) {
		m_type = Type::Saving;
		NOTICE() << "Saving";
	}
	if (metaName.contains("checking", Qt::CaseInsensitive)) {
		m_type = Type::Checking;
		NOTICE() << "Checking";
	}
	if (metaName.contains("Ckg", Qt::CaseInsensitive)) {
		m_type = Type::Checking;
		NOTICE() << "Checking";
	}
	if (type.contains("credit", Qt::CaseInsensitive)) {
		m_type = Type::Credit;
		NOTICE() << "Credit";
	}
	if (m_type == Type::Unknown) {
		ERR() << "Unknown account type. metaName: " << metaName << " . type: " << type;
	}
	m_balance = json["balance"].toObject()["available"].toDouble();
}

bool Account::loadPlaidJson(QString jsonFile) {
	m_jsonFilePath = jsonFile;
	QFile loadFile(m_jsonFilePath);
	if (!loadFile.open(QIODevice::ReadOnly)) {
		qWarning(QString("Couldn't open file %1").arg(QFileInfo(loadFile).absoluteFilePath()).toUtf8());
		return false;
	}
	QByteArray jsonData = loadFile.readAll();

	QJsonDocument loadDoc(QJsonDocument::fromJson(jsonData));
	const QJsonObject& json = loadDoc.object();

	loadJsonData(json);

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

	QFile writeFile(m_jsonFilePath + ".out");
	if (!writeFile.open(QIODevice::WriteOnly)) {
		qWarning(QString("Couldn't open file %1").arg(QFileInfo(writeFile).absoluteFilePath()).toUtf8());
		return false;
	}
	QJsonDocument saveDoc(json);
	writeFile.write(saveDoc.toJson());
	return true;
}




