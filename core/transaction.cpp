#include "transaction.h"
#include "account.h"

QVector<int> Transaction::onlyLoadHashes = QVector<int>();

void Transaction::read(const QJsonObject &json) {
	bool ok = false;
	QString accountStr = json["_account"].toString();
	name = json["name"].toString();
	nameHash.setFromString(name);
	setAmount(-json["amount"].toDouble(ok));
	date = QDate::fromString(json["date"].toString().left(10), "yyyy-MM-dd");
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
		<< " account " << accountStr
		<< endl;
}

void Transaction::write(QJsonObject &json) const {
	json["name"] = name;
	json["hash"] = nameHash.hash();
	json["amount"] = -amountDbl();
	json["date"] = date.toString("yyyy-MM-dd");
}

qint64 Transaction::dist(const Transaction &other, bool log) const {
	// if both are positive, let's make them a lot closer
	if (amountInt() > 0 && other.amountInt() > 0
	 || amountInt() < -2*KLA_MULTIPLICATOR && other.amountInt() < -2*KLA_MULTIPLICATOR) {
		return distanceWeighted<32, 512, 8>(other, log);
	}
	return distanceWeighted<4, 128, 2>(other, log);
}



void StaticTransactionArray::read(const QJsonArray& npcArray, int afterJday, int beforeJday, const QVector<QString> &onlyAcIds /*= anyID*/) {
	for (int npcIndex = 0; npcIndex < npcArray.size(); ++npcIndex) {
		QJsonObject jsonTrans = npcArray[npcIndex].toObject();
		QString accountTrans = jsonTrans["_account"].toString();
		if (onlyAcIds.isEmpty() || onlyAcIds.contains(accountTrans)) {
			auto ok = appendNew(jsonTrans, 0);
			if ((afterJday && last()->jDay() < afterJday) || (beforeJday && last()->jDay() > beforeJday)) {
				if( ok)
					removeLast();
				continue;
			}
		}
		else {
			LOG() << "transaction not matching an account:"<< accountTrans
				  << " object:" << npcArray[npcIndex].toString() << endl;
		}
	}
	qSort(m_transArray.begin(), m_transArray.begin() + m_numTrans, Transaction::earlierThan);
	qDebug() << "transaction loaded " << count() << "/" << npcArray.size();
}

void StaticTransactionArray::write(QJsonArray& npcArray) const {
	for (int i = 0; i < count(); ++i) {
		QJsonObject npcObject;
		m_transArray[i].write(npcObject);
		npcArray.append(npcObject);
	}
}

Transaction* StaticTransactionArray::appendNew(QJsonObject jsonTrans, Account *pInAcc) {
	QString name = jsonTrans["name"].toString();
	qint64 hash = NameHashVector::fromString(name);
	for (const QString& nono : pInAcc->excludeNameTransContain()) {
		if (name.contains(nono, Qt::CaseInsensitive)) {
			LOG() << "not Adding transaction because it looks like an internal transfer based on name containing"
				  << nono << endl;
			return 0;
		}
		if (!Transaction::onlyLoadHashes.isEmpty() && !Transaction::onlyLoadHashes.contains(hash)) {
			LOG() << "not Adding transaction because Transaction::onlyLoadHashes doesn't contain "
				  << hash << endl;
			return 0;
		}
	}
	Transaction* pNewTrans = &m_transArray[m_numTrans++];
	pNewTrans->read(jsonTrans);
	pNewTrans->account = pInAcc;
	return pNewTrans;
}



