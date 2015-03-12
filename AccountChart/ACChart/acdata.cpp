#include "acdata.h"
#include "../../evoCache/log.h"

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

unsigned int proximityHashString(const QString &str) {
	unsigned int ret = 0;
	for (const QChar& c : str) {
		ret += c.toUpper().toLatin1();
	}
	return ret;
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
	qDebug() << m_accountIds;
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
			m_transList.append(tra);
		}
		else {
			qDebug() << "transaction not matching an account:"<< accountTrans
					 << " object:" << npcObject;
		}
	}
	qSort(m_transList.begin(), m_transList.end(), Transaction::earlierThan);
	qDebug() << "m_transList count" << m_transList.size();
}

void Transactions::write(QJsonObject &json) const {
	QJsonArray npcArray;
	foreach (const auto tra, m_transList) {
		QJsonObject npcObject;
		tra.write(npcObject);
		npcArray.append(npcObject);
	}
	json["purchases"] = npcArray;
}

void Transaction::read(const QJsonObject &json) {
	bool ok = false;
	QString accountStr = json["_account"].toString();
	QString id = json["_id"].toString();
	QString name = json["name"].toString();
	nameHash.hash = proximityHashString(name);
	amount = -json["amount"].toDouble(ok);
	date = QDate::fromString(json["date"].toString(), "yyyy-MM-dd");
	QJsonArray npcArrayOld = json["category"].toArray();
	for (int npcIndex = 0; npcIndex < npcArrayOld.size(); ++npcIndex) {
		categories.append(npcArrayOld[npcIndex].toString());
	}

	// logs all in the LOG.
	QTextStream& out = LOG();
	out.setFieldWidth(8);
	out.setPadChar(' ');
	out.setFieldAlignment(QTextStream::AlignRight);
	out << "Transaction::read()" << amount << " date " << date.toJulianDay()
		  << " name " << name;
	out << " cat:[";
	for (QString& s : categories) {
		out << " " << s;
	}
	out << "]"
		<< " account " << accountStr << " id " << id
		<< endl;
}

void Transaction::write(QJsonObject &json) const {
	json["amount"] = amountDbl();
	//		json["startDate"] = int(m_startDate.toTime_t());
	//		json["numDays"] = m_numDays;
	//		json["descr"] = m_description;
}

