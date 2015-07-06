#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "core_global.h"
#include "common.h"
class Account;

typedef NameHashVector2 NameHashVector;

class CORESHARED_EXPORT Transaction// : public DBobj
{
private:
	//	double amount = 0.0;
		qint64 kamount = 0; // integer = round(amount * Mult)
		double kla = 0; // Mult * kindaLog(amount)
public:
	Account* account = 0;
	QString name; // "YARROW HOTEL GRILL" or "STRIKE TECHNOLOG"
	QDate date; // "2015-01-28"
	QStringList categories; // ["Food and Drink", "Restaurants"] or ["Transfer", "Payroll"]
	NameHashVector nameHash;
//	int indexHash = -1;
	// used to make the distance arbitrary far from anything
	int dimensionOfVoid = 0;

	enum Flag { None = 0x0, Predicted = 0x1, CameTrue = 0x2 , Internal = 0x4 };
	int flags = Flag::None;
	bool isInternal() const { return flags & Transaction::Flag::Internal; }
	int type() const;

	//! json in
	void read(const QJsonObject &json);
	//! json out
	void write(QJsonObject &json) const;

	double time_t() const {
		static const qint64 day0 = QDateTime::fromTime_t(0).date().toJulianDay();
		return (3600.0 * 24.0) * (double(date.toJulianDay() - day0) - 0.3125);
	}
	void setAmount(double amntDbl) {
		 kamount = double(KA_MULTIPLICATOR) * amntDbl;
		 kla = double(KLA_MULTIPLICATOR) * kindaLog(amntDbl);
	}
	void setKLA(double newKLA) {
		kla = newKLA;
		kamount = KA_MULTIPLICATOR * unKindaLog(double(newKLA) / double(KLA_MULTIPLICATOR));
	}
	int amountInt() const {
		return kla;
	}
	double amountDbl() const {
		return double(kamount) / double(KA_MULTIPLICATOR);
	}
	double compressedAmount() const{
		return kindaLog(amountDbl());
	}
	static bool earlierThan(const Transaction first, const Transaction second) {
		return first.date < second.date;
	}
	static bool smallerAmountThan(const Transaction first, const Transaction second) {
		return first.amountInt() < second.amountInt();
	}
	//! julian day
	qint64 jDay() const {
		return date.toJulianDay();
	}

	template <qint64 mD, qint64 mA, qint64 mH>
	qint64 distanceWeighted(const Transaction& other, bool log = false) const {
		qint64 d = 0;
		d += LIMIT_DIST_TRANS * (absInt(jDay() - other.jDay())) / mD;
		d += LIMIT_DIST_TRANS * (absInt(amountInt() - other.amountInt())) / mA;
		d += LIMIT_DIST_TRANS * nameHash.dist(other.nameHash) / mH;
//		d += LIMIT_DIST_TRANS * qint64(absInt(indexHash - other.indexHash)) / mIH;
		d |= (1<<20) * qint64(absInt(dimensionOfVoid - other.dimensionOfVoid));
		d |= (1<<20) * qint64(isInternal() || other.isInternal());
		if(log) {
			LOG() << "dist " << d
				<< QString(" = %1 x day(%2)").arg(double(LIMIT_DIST_TRANS)/mD).arg(absInt(jDay() - other.jDay()))
				<< QString(" = %1 x kamount(%2)").arg(double(LIMIT_DIST_TRANS)/mA).arg(absInt(amountInt() - other.amountInt()))
				<< QString(" = %1 x hash(%2)").arg(double(LIMIT_DIST_TRANS)/mH).arg(nameHash.dist(other.nameHash))
				<< endl;
		}
		return d;
	}

	//! distance between this transaction and anther.
	qint64 dist(const Transaction& other, bool log = false) const;

	static const qint64 LIMIT_DIST_TRANS = 512;

public:
	static QVector<int> onlyLoadHashes;
	static QDate onlyAfterDate;
	static int onlyAccountType;
};

struct StaticTransactionArray
{
	//! json in
	void read(const QJsonArray& npcArray, int afterJday = 0, int beforeJday = 0, const QVector<QString>& onlyAcIds = QVector<QString>());
	//! json out
	void write(QJsonArray &npcArray) const;
	Transaction* transArray() { return &m_transArray[0]; }
	Transaction& trans(int i) { return i >= 0 ? m_transArray[i] : m_transArray[i + count()]; }
	void clear() { m_numTrans = 0; }
	int count() const { return m_numTrans; }
	Transaction* appendNew(QJsonObject jsonTrans, Account* pInAcc);
	Transaction* last() { return &m_transArray[m_numTrans - 1]; }
	void removeLast() { m_numTrans--; }
	void sort() {
		qSort(m_transArray.begin(), m_transArray.begin() + m_numTrans, Transaction::earlierThan);
	}

	QDate lastTransactionDate() {
		return transArray()[count() - 1].date;
	}
	QDate firstTransactionDate() {
		return transArray()[0].date;
	}

private:
	// static array to allow pointing at the transactions
	std::array<Transaction, MAX_TRANSACTION_PER_ACCOUNT> m_transArray;
	int m_numTrans = 0;
};



class CORESHARED_EXPORT TransactionBundle : public QObject
{
public:
	TransactionBundle() {
		m_vector.reserve(64);
	}
	void clear() {
		m_vector.clear();
	}
	void append(Transaction* pTrans) {
		m_vector.append(pTrans);
	}
	Transaction& trans(int index) {
		if(index >= 0)
			return *m_vector[index];
		return trans(index + m_vector.count());
	}
	int count() const {
		return m_vector.count();
	}
	QString averageName() const {
		QVector<uint> sum(64, 0);
		int tot = m_vector.count();
		for (int i = 0; i < tot; ++i) {
			Transaction* t = m_vector[i];
			for (int c = 0; c < qMin(63, t->name.length()); ++c) {
				sum[c] += t->name[c].toLatin1();
			}
		}
		char charL[64];
		for (int c = 0; c < 63; ++c) {
			charL[c] = sum[c] / tot;
		}
		return QString::fromLatin1(charL);
	}
	QStringList uniqueNames() const {
		QStringList ret;
		for (int i = 0; i < m_vector.count(); ++i) {
			Transaction* t = m_vector[i];
			if (!ret.contains(t->name))
				ret.append(t->name);
		}
		return ret;
	}
	double sumDollar() const {
		double ret = 0.0;
		for (int i = 0; i < m_vector.count(); ++i) {
			Transaction* t = m_vector[i];
			ret += t->amountDbl();
		}
		return ret;
	}
	int averageKLA() const {
		double ret = sumDollar() / m_vector.count();
		return kindaLog(ret) * KLA_MULTIPLICATOR;
	}

private:
	QVector<Transaction*> m_vector;
};

#endif // TRANSACTION_H
