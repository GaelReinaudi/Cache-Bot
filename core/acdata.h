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

double kindaLog(double amount);
QRectF kindaLog(QRectF rectLinear);
unsigned int proximityHashString(const QString& str);

#define MAX_HASH_LENGTH 64
static const int MAX_TRANSACTION_PER_ACCOUNT = 1024 * 16;
class Account;

struct Transaction
{
	Account* account = nullptr;
	QString id; // "pKowox9EaKF14mBJ71m3hnmoPgA3Q0T4rjDox"
	QString name; // "YARROW HOTEL GRILL" or "STRIKE TECHNOLOG"
//	double amount = 0.0;
	int kamount = 0; // integer = round(amount * 1024)
	QDate date; // "2015-01-28"
	QStringList categories; // ["Food and Drink", "Restaurants"] or ["Transfer", "Payroll"]
	union {
		unsigned int hash = 0;
		uchar b[4];
	} nameHash;

	//! json in
	void read(const QJsonObject &json);
	//! json out
	void write(QJsonObject &json) const;

	double time_t() const {
		static const qint64 day0 = QDateTime::fromTime_t(0).date().toJulianDay();
		return (3600.0 * 24.0) * (double(date.toJulianDay() - day0) + 0.5);
	}
	double amountDbl() const {
		return double(kamount) / 1024.0;
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
};

class TransactionBundle : public QObject
{
public:
	TransactionBundle() {
		m_vector.reserve(1024);
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
	bool loadPlaidJson(QString jsonFile);

	QMap<uint, TransactionBundle*>& hashBundles() {
		return m_hashBundles;
	}

	TransactionBundle& allTrans() {
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
		void read(const QJsonObject &json, const QVector<QString>& acIds);
		//! json out
		void write(QJsonObject &json) const;
		Transaction* transArray() { return &m_transArray[0]; }
		void clear() { m_numTrans = 0; }
		int count() const { return m_numTrans; }
		Transaction* appendNew() { return &m_transArray[m_numTrans++]; }
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
		qDebug() << m_hashBundles.count() << m_hashBundles.keys().first() << m_hashBundles.keys().last();
	}

private:
	Transactions m_allTransactions;
	TransactionBundle m_allTrans;
	HashedBundles m_hashBundles;
};

class Household : public QObject
{
	QVector<Account*> m_accounts;
};

#endif // ACDATA_H
