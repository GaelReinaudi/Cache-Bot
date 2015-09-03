#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "core_global.h"
#include "common.h"
#include <array>
class Account;

typedef NameHashVector2 NameHashVector;

class CORESHARED_EXPORT Transaction// : public DBobj
{
private:
		double m_amountDbl = 0.0;
		double m_kla = 0; // Mult * kindaLog(amount)
public:
	Account* account = 0;
	QString name; // "YARROW HOTEL GRILL" or "STRIKE TECHNOLOG"
	QDate date; // "2015-01-28"
	QStringList categories; // ["Food and Drink", "Restaurants"] or ["Transfer", "Payroll"]
	NameHashVector nameHash;
//	int indexHash = -1;
	// used to make the distance arbitrary far from anything
	mutable int dimensionOfVoid = 0;
	char effect128 = 0;

	enum Flag { None = 0x0, Predicted = 0x1, CameTrue = 0x2 , Internal = 0x4 };
	int flags = Flag::None;
	bool isInternal() const { return flags & Transaction::Flag::Internal; }
	bool isFuture() const { return date > Transaction::currentDay(); }
	bool isToOld() const { return date < Transaction::currentDay().addDays(-Transaction::maxDaysOld()); }
	bool noUse() const { return isFuture() || isToOld() || isInternal(); }
	int type() const;

	//! json in
	void read(const QJsonObject &json);
	//! json out
	void write(QJsonObject &json) const;

	double time_t() const {
		static const qint64 day0 = QDateTime::fromTime_t(0).date().toJulianDay();
		return (3600.0 * 24.0) * (double(date.toJulianDay() - day0));
	}
	void setAmount(double amntDbl) {
		m_amountDbl = amntDbl;
		m_kla = kindaLog(amntDbl);
	}
	void setKLA(double newKLA) {
		m_kla = newKLA;
		m_amountDbl = unKindaLog(newKLA);
	}
	double amountDbl() const {
		return m_amountDbl;
	}
	double amount() const {
		return m_amountDbl;
	}
	double kla() const{
		return m_kla;
	}
	inline static bool earlierThan(const Transaction& first, const Transaction& second) {
		return first.date < second.date;
	}
	inline static bool smallerAmountThan(const Transaction& first, const Transaction& second) {
		return first.amount() < second.amount();
	}
	//! julian day
	qint64 jDay() const {
		return date.toJulianDay();
	}
	void setDimensionOfVoid(int n = 1) const {
		Q_ASSERT(dimensionOfVoid == 0);
		dimensionOfVoid += n;
	}

	template <qint64 mD, qint64 mA, qint64 mH>
	qint64 distanceWeighted(const Transaction& other, bool log = false) const {
		qint64 d = 0;
		d += LIMIT_DIST_TRANS * (qAbs(jDay() - other.jDay())) / mD;
		d += LIMIT_DIST_TRANS * (qAbs(kla() - other.kla()) * 1024) / mA;
		d += LIMIT_DIST_TRANS * nameHash.dist(other.nameHash) / mH;
		d |= (1<<20) * qint64(qAbs(dimensionOfVoid - other.dimensionOfVoid));
		d |= (1<<20) * qint64(isInternal() || other.isInternal());
		d |= (1<<20) * qint64((amount() > 0 && other.amount() < 0) || (amount() < 0 && other.amount() > 0));
		if(log) {
			DBG() << "dist " << d
				<< QString(" = %1 x day(%2)").arg(double(LIMIT_DIST_TRANS)/mD).arg(absInt(jDay() - other.jDay()))
				<< QString(" = %1 x kla(%2)").arg(double(LIMIT_DIST_TRANS)/mA).arg(qAbs(kla() - other.kla()))
				<< QString(" = %1 x hash(%2)").arg(double(LIMIT_DIST_TRANS)/mH).arg(nameHash.dist(other.nameHash));
		}
		return d;
	}

	//! distance between this transaction and anther.
	qint64 dist(const Transaction& other, bool log = false) const;

	static const qint64 LIMIT_DIST_TRANS = 512;

	//! returns the date considered as being the current day for all computations to come
	static QDate currentDay() {
		return s_currentDay;
	}
	static void setCurrentDay(const QDate &value) {
		s_currentDay = value;
	}
	//! returns the maximum month to consider a transaction for
	static int maxDaysOld() {
		return s_maxDaysOld;
	}
	static void setMaxDaysOld(const int value) {
		s_maxDaysOld = value;
	}


private:
	static int s_maxDaysOld;
	static QDate s_currentDay;
public:
	static QVector<int> onlyLoadHashes;
	static QDate onlyAfterDate;
	static int onlyAccountType;
};

struct StaticTransactionArray
{
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
	void stampAllTransactionEffect();

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



class CORESHARED_EXPORT TransactionBundle// : public QObject
{
public:
	TransactionBundle() {
		m_vector.reserve(64);
	}
	void clear() {
		m_vector.clear();
	}
	void append(const Transaction* pTrans) {
		m_vector.append(pTrans);
	}
//	Transaction& transRef(int index) {
//		if(index >= 0)
//			return *m_vector[index];
//		return transRef(index + m_vector.count());
//	}
	const Transaction& trans(int index) const {
		return *m_vector.at(index);
	}
	const Transaction& last() const {
		return *m_vector.last();
	}
	int count() const {
		return m_vector.count();
	}
//	QString averageName() const {
//		QVector<uint> sum(64, 0);
//		int tot = m_vector.count();
//		for (int i = 0; i < tot; ++i) {
//			const Transaction* t = m_vector.at(i);
//			for (int c = 0; c < qMin(63, t->name.length()); ++c) {
//				sum[c] += t->name[c].toLatin1();
//			}
//		}
//		char charL[64];
//		for (int c = 0; c < 63; ++c) {
//			charL[c] = sum[c] / tot;
//		}
//		return QString::fromLatin1(charL);
//	}
	QStringList uniqueNames() const {
		QStringList ret;
		for (int i = 0; i < m_vector.count(); ++i) {
			const Transaction* t = m_vector.at(i);
			if (!ret.contains(t->name))
				ret.append(t->name);
		}
		return ret;
	}
	double sumDollar() const {
		double ret = 0.0;
		for (int i = 0; i < m_vector.count(); ++i) {
			const Transaction* t = m_vector.at(i);
			ret += t->amountDbl();
		}
		return ret;
	}
	double averageAmount(std::function<double(const Transaction&)> weight = [](const Transaction&){ return 1.0; }) const;
	double emaAmount(const double facNew) const;
	double avgSmart() const;
	Transaction randSmart() const;
	Transaction randomTransaction(std::function<double(const Transaction&)> weight) const;
	Transaction randomTransaction() const;
	int klaAverage() const;

private:
	QVector<const Transaction*> m_vector;
};

#endif // TRANSACTION_H
