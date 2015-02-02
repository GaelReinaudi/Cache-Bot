#include "acdata.h"

double kindaLog(double amount) {
	if (amount < 0)
		return -log10(-amount+1.0);
	return log10(amount+1.0);
}

QRectF kindaLog(QRectF rectLinear) {
	QRectF compRect(rectLinear);
	compRect.setBottom(kindaLog(compRect.bottom()));
	compRect.setTop(kindaLog(compRect.top()));
	return compRect;
}

bool Account::load(QString jsonFile) {
	QFile loadFile(jsonFile);
	if (!loadFile.open(QIODevice::ReadOnly)) {
		qWarning(QString("Couldn't open file %1").arg(QFileInfo(loadFile).absoluteFilePath()).toUtf8());
		return false;
	}
	QByteArray saveData = loadFile.readAll();
	QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
	const QJsonObject& json = loadDoc.object();
	m_accountIds.clear();
	QJsonArray npcArrayAccount = json["accounts"].toArray();
	qDebug() << npcArrayAccount.size();
	for (int npcIndex = 0; npcIndex < npcArrayAccount.size(); ++npcIndex) {
		QJsonObject npcObject = npcArrayAccount[npcIndex].toObject();
		QString accountID = npcObject["_id"].toString();
		Q_ASSERT(!accountID.isEmpty());
		if (accountID != "")
			m_accountIds.push_back(accountID);
	}
	m_transactions.read(json, m_accountIds);

	//m_transactions.cleanSymetricTransaction();

	return true;
}

void Transactions::read(const QJsonObject &json, const QVector<QString> &acIds) {
	m_transList.clear();
	QJsonArray npcArray = json["transactions"].toArray();
	for (int npcIndex = 0; npcIndex < npcArray.size(); ++npcIndex) {
		QJsonObject npcObject = npcArray[npcIndex].toObject();
		QString accountTrans = npcObject["_account"].toString();
		if (acIds.contains(accountTrans)) {
			Transaction tra;
			tra.read(npcObject);
			if (tra.type() > Transaction::InternalTransfer)
				m_transList.append(tra);
		}
		else {
			qDebug() << "transaction not matching an account:"<< accountTrans
					 << " object:" << npcObject;
		}
	}
	QJsonArray npcArrayOld = json["purchases"].toArray();
	for (int npcIndex = 0; npcIndex < npcArrayOld.size(); ++npcIndex) {
		QJsonObject npcObject = npcArrayOld[npcIndex].toObject();
		Transaction tra;
		tra.read(npcObject);
		m_transList.append(tra);
	}
	qSort(m_transList.begin(), m_transList.end(), Transaction::earlierThan);
	qDebug() << "m_transList count" << m_transList.size();
}

void Transaction::read(const QJsonObject &json) {
	bool ok;
	m_amount = json["amount"].toString().toDouble(&ok);
	uint sec = json["startDate"].toString().toDouble(&ok);
	if(ok)
		m_startDate = QDateTime::fromTime_t(sec);
	else {
		m_amount = -json["amount"].toDouble();
		m_startDate = QDateTime::fromString(json["date"].toString(), "yyyy-MM-dd");
		if (json["name"].toString() == "Online Transfer") {
			m_type = InternalTransfer;
		}
	}
	m_numDays = json["numDays"].toString().toDouble(&ok);
	if(!ok)
		m_numDays = 1;
	m_description = json["descr"].toString();
	qDebug() << m_amount << sec << m_startDate << m_numDays << m_description;
}
