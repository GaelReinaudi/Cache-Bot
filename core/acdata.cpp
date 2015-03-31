#include "acdata.h"
#include "../evoCache/log.h"

double kindaLog(double amount) {
	if (amount < 0)
		return -qLn(-amount+1.0) * 0.43429448190325182765112891891661; // 1 / 2.3025850929940456840179914546844;
	return qLn(amount+1.0) * 0.43429448190325182765112891891661; // 1 / 2.3025850929940456840179914546844;
}
double unKindaLog(double kindaLogAmount)
{
	if (kindaLogAmount < 0)
		return 1.0 - qExp(-kindaLogAmount * 2.3025850929940456840179914546844); // 1 / 0.43429448190325182765112891891661
	return -1.0 + qExp(kindaLogAmount * 2.3025850929940456840179914546844); // 1 / 0.43429448190325182765112891891661
}

unsigned int proximityHashString(const QString &str) {
	unsigned int ret = 0;
	for (const QChar& c : str) {
		int n = c.toUpper().toLatin1() * 1;
		// for numbers
		if (c.isDigit())
			n = 0;//QChar('#').toLatin1();
		ret += n;
	}
	return ret;
}

bool Account::loadPlaidJson(QString jsonFile) {
	QFile loadFile(jsonFile);
	if (!loadFile.open(QIODevice::ReadOnly)) {
		qWarning(QString("Couldn't open file %1").arg(QFileInfo(loadFile).absoluteFilePath()).toUtf8());
		return false;
	}
	QByteArray saveData = loadFile.readAll();
	QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
	const QJsonObject& json = loadDoc.object();
//	m_accountIds.clear();
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
	m_allTransactions.read(json["transactions"].toArray(), m_accountIds);

	m_predicted.read(json["predicted"].toArray());

	//m_transactions.cleanSymetricTransaction();

	// make a bundle of all the transactions
	m_allTrans.clear();
	for (int i = 0; i < m_allTransactions.count(); ++i) {
		m_allTrans.append(&m_allTransactions.transArray()[i]);
	}
	makeHashBundles();

	return true;
}

void Account::Transactions::read(const QJsonArray& npcArray, const QVector<QString> &onlyAcIds /*= anyID*/) {
	clear();
	for (int npcIndex = 0; npcIndex < npcArray.size(); ++npcIndex) {
		QJsonObject npcObject = npcArray[npcIndex].toObject();
		QString accountTrans = npcObject["_account"].toString();
		if (onlyAcIds.isEmpty() || onlyAcIds.contains(accountTrans)) {
			appendNew()->read(npcObject);
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

void Transaction::read(const QJsonObject &json) {
	bool ok = false;
	QString accountStr = json["_account"].toString();
	id = json["_id"].toString();
	name = json["name"].toString();
	nameHash.hash = proximityHashString(name);
	setAmount(-json["amount"].toDouble(ok));
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
	out << "Transaction::read()" << amountDbl() << " date " << date.toJulianDay()
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



