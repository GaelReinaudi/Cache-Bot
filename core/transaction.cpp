#include "transaction.h"
#include "account.h"
#include "botContext.h"

int hoursOffsetToHack_issue_9 = -5;
QDateTime Transaction::s_actualCurrentDayTime = QDateTime::currentDateTime().addSecs(hoursOffsetToHack_issue_9 * 3600);
QDate Transaction::s_currentDay = Transaction::s_actualCurrentDayTime.date().addDays(-1);//.addMonths(-2);
Transaction Transaction::s_hypotheTrans;
int Transaction::s_magicFilter = 0;

QVector<int> Transaction::onlyLoadHashes = QVector<int>();
QVector<int> Transaction::onlyPlaidCat = QVector<int>();
QVector<QRegExp> Transaction::rootCatRegExp;
QMap< QString, QVector<QRegExp> > Transaction::subCatRegExp;
int Transaction::s_maxDaysOld = 5 * 31;
int Transaction::s_maxDaysOldAllTransatcion = 30;
QDate Transaction::onlyAfterDate = Transaction::currentDay().addMonths(-6);
int Transaction::onlyAccountType = Account::Type::Saving | Account::Type::Checking | Account::Type::Credit;

bool Transaction::noUse() const
{
	return (magic != Transaction::s_magicFilter) || isFuture() || isToOld() || isInternal();
}

int Transaction::isVoid() const
{
	if (dimOfVoid) {
		WARNINGS(111, 3, "this is in the void ", dimOfVoid);
		return dimOfVoid;
	}
	return 0;
}

int Transaction::type() const {
	return account->type() + 16 * (flags & Internal);
}

void Transaction::loadUserFlags(const QJsonObject &json) {
	static QStringList ignore; static bool notAlready = true;
	if (notAlready) {
		notAlready = false;
		for (const auto & v : BotContext::JSON_ARGS["userInputIgnore"].toArray()) {
			ignore.append(v.toString());
			WARN() << "userInputIgnore flag: " << v.toString();
		}
	}
	for (const QJsonValue& jv : json["userInput"].toArray()) {
		QString flag = jv.toString();
		INFO() << "userInput flag:" << flag;
		if (flag.startsWith("noRecur")) {
			if (ignore.contains("noRecurPos") && amount() > 0) {
				WARN() << "ignoring flag:" << flag;
			}
			else {
				userFlag |= NoRecur;
			}
		}
	}
}

QString Transaction::cleanName(const QString &inName)
{
	QString outName = inName.toUpper();
	outName.remove(" FROM").remove(" TO");
	outName.remove("XXXXX").remove("CKF ").remove(" LN").replace("HOMEFINANCE", " MTGE");
	outName.remove("CHK").remove(" SAV");
	outName.remove(" ACCT");
	outName.remove(" ONLINE").remove(" BANKING").remove(" CONFIRMATION");
	outName.remove(" IMAGE");
	return outName.trimmed();
}

void Transaction::read(const QJsonObject &json) {
	bool ok = false;
	QString accountStr = json["plaid_account"].toString();
	id = json["_id"].toString();
	name = Transaction::cleanName(json["name"].toString());
	setAmount(-json["amount"].toDouble(ok));
	nameHash.setFromString(name, m_kla);
	if (json.contains("cache_category_id")) {
		categoryHash.setFromHash(json["cache_category_id"].toString().toLongLong());// * (m_kla<0?-1:1));
		WARN() << "using cache_category_id: " << categoryHash.hash();
	}
	else {
		categoryHash.setFromHash(json["category_id"].toString().toLongLong() * (m_kla<0?-1:1));
	}
	QString dateToUse = "date";
	if (json.contains("pending_date"))
		dateToUse = "pending_date";
	date = QDate::fromString(json[dateToUse].toString().left(10), "yyyy-MM-dd");
	QJsonArray npcArrayOld = json["category"].toArray();
	for (int npcIndex = 0; npcIndex < npcArrayOld.size(); ++npcIndex) {
		categories.append(npcArrayOld[npcIndex].toString());
	}
	s_maxDaysOldAllTransatcion = qMax(s_maxDaysOldAllTransatcion, int(date.daysTo(Transaction::currentDay())));
	loadUserFlags(json);

	// logs all in the LOG.
	auto out = INFO();
//	out.setFieldWidth(8);
//	out.setPadChar(' ');
//	out.setFieldAlignment(QTextStream::AlignRight);
	out << "Transaction::read("<<id<<") " << amountDbl() << "   " << date.toString("MM/dd")
		  << "   " << name;
	out << "   [";
	out << categoryHash.hash();
	for (QString& s : categories) {
		out << "," << s;
	}
	out << "]"
		<< " account .." << accountStr.right(4)
		   ;
}

qint64 Transaction::dist(const Transaction &other, bool log) const {
	// if both are positive, let's make them a lot closer
	if ((klaEff() > 0 && other.klaEff() > 0)
	 || (klaEff() < -2 && other.klaEff() < -2)) {
		return distanceWeighted<16*2, 512*2, 2*2>(other, log);
	}
//	if ((klaEff() < -3 && other.klaEff() < -3)) {
//		return distanceWeighted<16*2, 512*2, 2*2*1024*128>(other, log);
//	}
	return distanceWeighted<16, 512, 2>(other, log);
}

void Transaction::makeCatRegExps(QString strVal, QString keyCat)
{
	// load file once
	static QJsonObject jsonCacheCat;
	if (jsonCacheCat.isEmpty()) {
		// Cache categories
		QFile fileCacheCat("../../cache_categories.json");
		fileCacheCat.open(QFile::ReadOnly);
		QString strCat = fileCacheCat.readAll();
		QRegExp rx("/[*].*[*]/");
		rx.setMinimal(true);
		strCat.remove(rx);
		DBG() << strCat;
		jsonCacheCat = QJsonDocument::fromJson(strCat.toUtf8()).object();
		DBG() << "jsonCacheCat\n" << QString(QJsonDocument(jsonCacheCat).toJson());
	}

	bool isRoot = keyCat == "";
	if (isRoot) {
		rootCatRegExp.clear();
		subCatRegExp.clear();
		QStringList subCats;
		for (QJsonValue it : jsonCacheCat[keyCat].toArray()) {
			subCats += it.toString();
		}
		INFO() << "subCats: " << subCats.join(", ");
	}
	if (!jsonCacheCat.contains(strVal)) {
		QRegExp r(strVal);
		rootCatRegExp.append(r);
		if (!jsonCacheCat.contains(keyCat))
			return;
		subCatRegExp[keyCat].append(r);
	}
	for (QJsonValue v : jsonCacheCat[strVal].toArray()) {
		QString s = v.toString();
		QString subCat = isRoot ? s : keyCat;
		Transaction::makeCatRegExps(s, subCat);
	}
	if (isRoot) {
		NOTICE() << "root regExp:";
		for (const QRegExp& r : rootCatRegExp) {
			INFO() << r.pattern();
		}
		for (const auto& s : subCatRegExp.keys()) {
			NOTICE() << "subCat " << s << " regExp:";
			for (const QRegExp& r : subCatRegExp[s]) {
				INFO() << r.pattern();
			}
		}
	}
}

Transaction* StaticTransactionArray::appendNew(QJsonObject jsonTrans, Account *pInAcc) {
	QString name = Transaction::cleanName(jsonTrans["name"].toString());
	QString dateToUse = "date";
	if (jsonTrans.contains("pending_date"))
		dateToUse = "pending_date";
	QDate date = QDate::fromString(jsonTrans[dateToUse].toString().left(10), "yyyy-MM-dd");
	double kla = -jsonTrans["amount"].toDouble();
	qint64 hash = NameHashVector::fromString(name, kla);
	qint64 hashCat = jsonTrans["category_id"].toString().toLongLong();
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
	if (!Transaction::onlyPlaidCat.isEmpty() && !Transaction::onlyPlaidCat.contains(hashCat)) {
		DBG() << "not Adding transaction because Transaction::onlyCategory doesn't contain "
			  << hashCat;
		return 0;
	}
	if (!Transaction::rootCatRegExp.isEmpty()) {
		QString strHashCat = QString("%1").arg(hashCat, 8, 10, QChar('0'));
		bool match = false;
		for (QRegExp r : Transaction::rootCatRegExp) {
			if (r.pattern().startsWith("!")) {
				r.setPattern(r.pattern().mid(1));
				match &= !r.exactMatch(strHashCat);
			}
			else
				match |= r.exactMatch(strHashCat);
		}
		if (!match) {
			DBG() << "not Adding transaction because matching nothing in Transaction::rootCatRegExp "
				  << strHashCat;
			return 0;
		}
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
	pNewTrans->account = pInAcc;
	pNewTrans->read(jsonTrans);
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
		if (amnt128 > 0 && totPos) {
			trans(i).effect128 = amnt128 / totPos;
			++effectCount[trans(i).effect128];
		}
		if (amnt128 < 0 && totNeg) {
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
		if (tr->noUse())
			continue;
		double w = weight(*tr);
		totW += w;
		avgW += w * tr->amountDbl();
	}
	return totW ? avgW / totW : 0.0;
}

auto lam = [](const Transaction& tr){
	double daysOld = tr.date.daysTo(Transaction::currentDay());
	double totSpan = Transaction::maxDaysOld();
	double thresh = totSpan * 2 / 3;
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

double TransactionBundle::daysToNextSmart() const
{
	double daysToNext = double(2 * Transaction::maxDaysOld()) / qMax(1, count());
	static const double EMA_FACTOR = 0.25;

	double daysTo_i = 0;//Transaction::maxDaysOld();
	for (int i = 1; i < count(); ++i) {
		if (trans(i).isReturn) {
			daysTo_i += trans(i - 1).date.daysTo(trans(i).date);
			continue;
		}
		daysTo_i += trans(i - 1).date.daysTo(trans(i).date);
		double oldD2N = daysToNext;
		if (daysTo_i < 2) {
			daysTo_i += 1;
		}
		if (daysToNext <= 0.0) {
			WARN() << "daysToNext == 0.0";
			daysToNext = daysTo_i;
		}
		daysToNext *= (1.0 - EMA_FACTOR);
		daysToNext += daysTo_i * EMA_FACTOR;

		DBG() << "daysTo_ " << i << ": " << daysTo_i << " daysToNext: " << oldD2N << " -> " << daysToNext;
		daysTo_i = 0;
	}
	// if time since last is getting larger than when we should have seen one, we take it as a new point
	double daysToPres = last().date.daysTo(Transaction::currentDay());
	daysToPres *= 1.25;
	if (daysToPres > daysToNext) {
		daysToNext *= (1.0 - EMA_FACTOR);
		daysToNext += daysToPres * EMA_FACTOR;
	}
	DBG() << "daysToPres " << daysToPres << " final daysTo " << daysToNext;

	if (avgSmart() < 0.0) {
		daysToNext *= 1.05;
	}
	return daysToNext;
}
