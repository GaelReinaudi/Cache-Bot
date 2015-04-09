#ifndef ACDATA_H
#define ACDATA_H

#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFileInfo>
#include <QDebug>
#include <cmath>
#include <QRectF>
#include <QtMath>
#include <array>

double kindaLog(double amount);
double unKindaLog(double kindaLogAmount);
unsigned int proximityHashString(const QString& str);

#define MAX_HASH_LENGTH 64
static const int MAX_TRANSACTION_PER_ACCOUNT = 1024 * 16;
static const int KLA_MULTIPLICATOR = 128;
static const int KA_MULTIPLICATOR = 1024;

class Account;

static inline
qint64 absInt(const qint64& x) {
	return qAbs(x);
}
static inline
qint64 absInt(const qint32& x) {
	return (x ^ (x >> 31)) - (x >> 31);
}
static inline float
fasterpow2 (float p)
{
	float clipp = (p < -126) ? -126.0f : p;
	union { uint32_t i; float f; } v = { static_cast<uint32_t>( (1 << 23) * (clipp + 126.94269504f) ) };
	return v.f;
}
static inline float
fasterexp (float p)
{
	return fasterpow2 (1.442695040f * p);
}
template <int Lambda>
static inline
double expoInt(qint64 x)
{
	return qExp(double(x) * (1.0 / double(Lambda)));
}

struct Transaction
{
private:
	//	double amount = 0.0;
		qint64 kamount = 0; // integer = round(amount * Mult)
		double kla = 0; // Mult * kindaLog(amount)
public:
	Account* account = nullptr;
	QString id; // "pKowox9EaKF14mBJ71m3hnmoPgA3Q0T4rjDox"
	QString name; // "YARROW HOTEL GRILL" or "STRIKE TECHNOLOG"
	QDate date; // "2015-01-28"
	QStringList categories; // ["Food and Drink", "Restaurants"] or ["Transfer", "Payroll"]
	union {
		int hash = 0;
		uchar b[4];
	} nameHash;
	int indexHash = -1;
	// used to make the distance arbitrary far from anything
	int dimensionOfVoid = 0;

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
	double numDays() const{
		return 1;
	}
	static bool earlierThan(const Transaction first, const Transaction second) {
		return first.date < second.date;
	}
	//! julian day
	qint64 jDay() const {
		return date.toJulianDay();
	}

	template <quint64 wD, quint64 wA, quint64 wH, quint64 wIH>
	qint64 distanceWeighted(const Transaction& other) const {
		qint64 d = 0;
		d += qint64(wD) * (absInt(jDay() - other.jDay()));
		d += qint64(wA) * (absInt(amountInt() - other.amountInt()));
		d += qint64(wH) * qint64(absInt(nameHash.hash - other.nameHash.hash));
		d += qint64(wIH) * qint64(absInt(indexHash - other.indexHash));
		//LOG() << "dist " << d << " = day " << jDay() << "-" << other.jDay() << " kamount " << kamount << "-" << other.kamount << " hash " << nameHash.hash << "-" << other.nameHash.hash << endl;
		d += qint64(1<<20) * qint64(absInt(dimensionOfVoid - other.dimensionOfVoid));
		return d;
	}

	//! distance between this transaction and anther.
	inline quint64 dist(const Transaction& other) const {
		return distanceWeighted<8, 2, 128, 0>(other);
	}
};

class TransactionBundle : public QObject
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

typedef QMap<uint, TransactionBundle*> HashedBundles;

class Account// : public QObject
{
public:
	Account() {}

	// loading the json file
	// see this: https://qt-project.org/doc/qt-5-snapshot/qtcore-savegame-example.html
	bool loadPlaidJson(QString jsonFile, int afterJday);

	QMap<uint, TransactionBundle*>& hashBundles() {
		return m_hashBundles;
	}

	TransactionBundle& allTrans(int filterHash = -1) {
		if (filterHash >= 0)
			return *m_hashBundles[filterHash];
		return m_allTrans;
	}
	QDate lastTransactionDate() {
		return m_allTransactions.transArray()[m_allTransactions.count() - 1].date;
	}
	QDate firstTransactionDate() {
		return m_allTransactions.transArray()[0].date;
	}

private:
	QVector<QString> m_accountIds;
	struct Transactions
	{
		Transactions() {}
		//! json in
		void read(const QJsonArray& npcArray, int afterJday = 0, const QVector<QString>& onlyAcIds = QVector<QString>());
		//! json out
		void write(QJsonArray &npcArray) const;
		Transaction* transArray() { return &m_transArray[0]; }
		void clear() { m_numTrans = 0; }
		int count() const { return m_numTrans; }
		Transaction* appendNew() { return &m_transArray[m_numTrans++]; }
		Transaction* last() { return &m_transArray[m_numTrans - 1]; }
		void removeLast() { m_numTrans--; }
	private:
		std::array<Transaction, MAX_TRANSACTION_PER_ACCOUNT> m_transArray;
		int m_numTrans = 0;
	};
	//! makes a bundle for each hash value
	void makeHashBundles() {
		for (int i = 0; i < allTrans().count(); ++i) {
			Transaction& t = allTrans().trans(i);
			uint h = t.nameHash.hash;
			if (!m_hashBundles.contains(h))
				m_hashBundles[h] = new TransactionBundle();
			m_hashBundles[h]->append(&t);
		}
		// assigns the index of the hash to the transactions
		for (int i = 0; i < hashBundles().count(); ++i) {
			int h = hashBundles().keys()[i];
			for (int j = 0; j < hashBundles()[h]->count(); ++j) {
				hashBundles()[h]->trans(j).indexHash = i;
			}
		}

		qDebug() << m_hashBundles.count() << m_hashBundles.keys().first() << m_hashBundles.keys().last();
	}

private:
	Transactions m_allTransactions;
	TransactionBundle m_allTrans;
	HashedBundles m_hashBundles;

public:
	// for predicted transcations
	Transactions m_predicted;
};

class Household : public QObject
{
	QVector<Account*> m_accounts;
};

#endif // ACDATA_H
