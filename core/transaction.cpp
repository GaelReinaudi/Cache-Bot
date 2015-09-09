#include "transaction.h"
#include "account.h"

QDate Transaction::s_currentDay = QDate::currentDate();//.addDays(-26);//.addMonths(-2);

QVector<int> Transaction::onlyLoadHashes = QVector<int>();
int Transaction::s_maxDaysOld = 6 * 31;
QDate Transaction::onlyAfterDate = Transaction::currentDay().addMonths(-6);
int Transaction::onlyAccountType = Account::Type::Saving | Account::Type::Checking | Account::Type::Credit;

int Transaction::type() const {
	return account->type() + 16 * (flags & Internal);
}

void Transaction::read(const QJsonObject &json) {
	bool ok = false;
	QString accountStr = json["plaid_account"].toString();
	name = json["name"].toString();
	name.remove("FROM").remove("TO");//.remove("ACCT");
	nameHash.setFromString(name);
	setAmount(-json["amount"].toDouble(ok));
	date = QDate::fromString(json["date"].toString().left(10), "yyyy-MM-dd");
	QJsonArray npcArrayOld = json["category"].toArray();
	for (int npcIndex = 0; npcIndex < npcArrayOld.size(); ++npcIndex) {
		categories.append(npcArrayOld[npcIndex].toString());
	}

	// logs all in the LOG.
	auto out = INFO();
//	out.setFieldWidth(8);
//	out.setPadChar(' ');
//	out.setFieldAlignment(QTextStream::AlignRight);
	out << "Transaction::read() " << amountDbl() << "   " << date.toString("MM/dd")
		  << "   " << name;
	out << "   [";
	for (QString& s : categories) {
		out << " " << s;
	}
	out << " ]"
		<< " account .." << accountStr.right(4)
		   ;
}

void Transaction::write(QJsonObject &json) const {
	json["name"] = name;
	json["hash"] = nameHash.hash();
	json["amount"] = -amountDbl();
	json["date"] = date.toString("yyyy-MM-dd");
}

qint64 Transaction::dist(const Transaction &other, bool log) const {
	// if both are positive, let's make them a lot closer
	if ((amount() > 0 && other.amount() > 0)
	 || (amount() < -100 && other.amount() < -100)) {
		return distanceWeighted<16*2, 512*2, 2*2>(other, log);
	}
	return distanceWeighted<16, 512, 2>(other, log);
}

Transaction* StaticTransactionArray::appendNew(QJsonObject jsonTrans, Account *pInAcc) {
	QString name = jsonTrans["name"].toString();
	name.remove("FROM").remove("TO");//.remove("ACCT");
	QDate date = QDate::fromString(jsonTrans["date"].toString().left(10), "yyyy-MM-dd");
	qint64 hash = NameHashVector::fromString(name);
	for (const QString& nono : pInAcc->excludeNameTransContain()) {
		if (name.contains(nono, Qt::CaseInsensitive)) {
			WARN() << "not Adding transaction because it looks like an internal transfer based on name containing"
				  << nono;
			return 0;
		}
	}
	if (!Transaction::onlyLoadHashes.isEmpty() && !Transaction::onlyLoadHashes.contains(hash)) {
		DBG() << "not Adding transaction because Transaction::onlyLoadHashes doesn't contain "
			  << hash;
		return 0;
	}
	if (date < Transaction::onlyAfterDate) {
		DBG() << "not Adding transaction because Transaction::onlyAfterDate ";
		return 0;
	}
	if (!(pInAcc->type() & Transaction::onlyAccountType)) {
		DBG() << "not Adding transaction because Transaction::onlyAccountType "
			  << QString("b%1").arg(QString::number(Transaction::onlyAccountType, 2));
		return 0;
	}
	Transaction* pNewTrans = &m_transArray[m_numTrans++];
	pNewTrans->read(jsonTrans);
	pNewTrans->account = pInAcc;
	return pNewTrans;
}

void StaticTransactionArray::stampAllTransactionEffect()
{
	QMap<int, int> effectCount;
	int totNeg = 0;
	int totPos = 0;
	for (int  i = 0; i < m_numTrans; ++i) {
		if (trans(i).isInternal())
			continue;
		int amnt = trans(i).amountDbl();
		if (amnt > 0)
			totPos += amnt;
		if (amnt < 0)
			totNeg += amnt;
	}
	for (int  i = 0; i < m_numTrans; ++i) {
		if (trans(i).isInternal())
			continue;
		int amnt128 = 128 * trans(i).amountDbl();
		if (amnt128 > 0) {
			trans(i).effect128 = amnt128 / totPos;
			++effectCount[trans(i).effect128];
		}
		if (amnt128 < 0) {
			trans(i).effect128 = amnt128 / totNeg;
			++effectCount[-trans(i).effect128];
		}
	}
	qDebug() << "effectCount" << effectCount;
}





double TransactionBundle::averageAmount(std::function<double (const Transaction &)> weight) const
{
	double totW = 0.0;
	double avgW = 0.0;
	for (const Transaction* tr : m_vector) {
		double w = weight(*tr);
		totW += w;
		avgW += w * tr->amountDbl();
	}
	return totW ? avgW / totW : 0.0;
}

double TransactionBundle::emaAmount(const double facNew) const
{
	if (m_vector.count() == 0)
		return 0.0;
	double ret = m_vector.first()->amountDbl();
	for (int i = 1; i < m_vector.count(); ++i) {
		const Transaction* t = m_vector.at(i);
		ret *= (1.0 - facNew);
		ret += facNew * t->amountDbl();
	}
	return ret;
}

auto lam = [](const Transaction& tr){
	double daysOld = tr.date.daysTo(Transaction::currentDay());
	double totSpan = Transaction::maxDaysOld();
	double thresh = totSpan / 2;
	if (daysOld <= thresh)
		return 1.0;
	else if (daysOld <= totSpan)
		return 1.0 - (daysOld - thresh) / (totSpan - thresh);
	else
		return 0.0;
};

double TransactionBundle::avgSmart() const
{
	return averageAmount(lam);
}

Transaction TransactionBundle::randSmart() const
{
	return randomTransaction(lam);
}

Transaction TransactionBundle::randomTransaction(std::function<double (const Transaction &)> weight) const
{
	double totW = 0.0;
	static QVector<double> summingWeights;
	summingWeights.clear();
	for (const Transaction* tr : m_vector) {
		double w = weight(*tr);
		totW += w;
		summingWeights.append(totW);
	}
	if (totW == 0.0)
		return Transaction();

	double randInTotW = totW * double(qrand()) * (1.0 / double(RAND_MAX));

	for (int i = 0; i < summingWeights.count(); ++i) {
		if (randInTotW <= summingWeights[i]) {
			return trans(i);
		}
	}
	Q_ASSERT(false); // should never get there if this code works...
	return Transaction();
}

Transaction TransactionBundle::randomTransaction() const
{
	if (count() == 0)
		return Transaction();
	return trans(qrand() % count());
}

double TransactionBundle::klaAverage() const {
	if (m_vector.count() == 0)
		return 0.0;
	return kindaLog(averageAmount(lam));
}
