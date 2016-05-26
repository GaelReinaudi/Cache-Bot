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
	accountID = json["_id"].toString();
	m_plaidId = json["plaid_id"].toString();
	int accountLast4Digits = json["meta"].toObject()["number"].toInt();
	QString accountMetaName = json["meta"].toObject()["name"].toString();
	QString accountSubType = json["subtype"].toString();
	QString accountType = json["type"].toString();
	Q_ASSERT(!accountID.isEmpty());
	NOTICE() << "read account:" << accountID << ": " << accountMetaName
			 << "(" << accountLast4Digits << "): " << accountType
			 << ". plaid " << m_plaidId;

	// quick and dirty account type
	if (accountMetaName.contains("saving", Qt::CaseInsensitive)) {
		m_type = Type::Saving;
		NOTICE() << "Saving";
	}
	if (accountMetaName.contains("checking", Qt::CaseInsensitive)) {
		m_type = Type::Checking;
		NOTICE() << "Checking";
	}
	if (accountMetaName.contains("Ckg", Qt::CaseInsensitive)) {
		m_type = Type::Checking;
		NOTICE() << "Checking";
	}
	if (accountMetaName.contains("credit", Qt::CaseInsensitive)) {
		m_type = Type::Credit;
		NOTICE() << "Credit";
	}
	if (accountSubType.contains("saving", Qt::CaseInsensitive)) {
		m_type = Type::Saving;
		NOTICE() << "Saving";
	}
	if (accountSubType.contains("checking", Qt::CaseInsensitive)) {
		m_type = Type::Checking;
		NOTICE() << "Checking";
	}
	if (accountSubType.contains("credit", Qt::CaseInsensitive)) {
		m_type = Type::Credit;
		NOTICE() << "Credit";
	}
	if (accountType.contains("saving", Qt::CaseInsensitive)) {
		m_type = Type::Saving;
		NOTICE() << "Saving";
	}
	if (accountType.contains("checking", Qt::CaseInsensitive)) {
		m_type = Type::Checking;
		NOTICE() << "Checking";
	}
	if (accountType.contains("credit", Qt::CaseInsensitive)) {
		m_type = Type::Credit;
		NOTICE() << "Credit";
	}
	if (accountSubType.contains("moneymarket", Qt::CaseInsensitive)) {
		m_type = Type::Saving;
		NOTICE() << "Saving";
	}
	if (accountMetaName.contains("SAFE DEPOSIT BOX", Qt::CaseInsensitive)) {
		m_type = Type::Saving;
		NOTICE() << "Saving";
	}
	if (accountSubType.contains("brokerage", Qt::CaseInsensitive)) {
		m_type = Type::Saving;
		NOTICE() << "Saving";
	}
	if (accountSubType.contains("mortgage", Qt::CaseInsensitive)) {
		m_type = Type::Saving;
		NOTICE() << "Saving";
	}
	if (accountSubType.contains("loan", Qt::CaseInsensitive)) {
		m_type = Type::Saving;
		NOTICE() << "Saving";
	}
	if (accountSubType.contains("rewards", Qt::CaseInsensitive)) {
		m_type = Type::Saving;
		NOTICE() << "Saving";
	}
	if (m_type == Type::Unknown) {
		ERR() << "Unknown account type. metaName: " << accountMetaName << " . type: " << accountType << " . subtype: " << accountSubType;
		m_type = Type::Saving;
	}
	m_balance = json["balance"].toObject()["current"].toDouble();
}

bool Account::loadPlaidJson(QString jsonFile) {
	m_jsonFilePath = jsonFile;
	QFile loadFile(m_jsonFilePath);
	if (!loadFile.open(QIODevice::ReadOnly)) {
		WARN() << "Couldn't open file " << QFileInfo(loadFile).absoluteFilePath();
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
		WARN() << "Couldn't open file " << QFileInfo(loadFile).absoluteFilePath();
		//return false;
	}
	else {
		saveData = loadFile.readAll();
	}
	QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
	QJsonObject json = loadDoc.object();

//	QJsonArray transArray;
//	for (int i = 0; i < transactions.count(); ++i) {
//		QJsonObject obj;
//		transactions[i].write(obj);
//		transArray.append(obj);
//	}
//	json[category] = transArray;
	qDebug() << transactions.count();

	QFile writeFile(m_jsonFilePath + ".out");
	if (!writeFile.open(QIODevice::WriteOnly)) {
		WARN() << "Couldn't open file " << QFileInfo(writeFile).absoluteFilePath();
		return false;
	}
	QJsonDocument saveDoc(json);
	writeFile.write(saveDoc.toJson());
	return true;
}




