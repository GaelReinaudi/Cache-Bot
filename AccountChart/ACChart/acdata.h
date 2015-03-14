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

class Account;

struct Transaction
{
	Account* account = nullptr;
	QString id; // "pKowox9EaKF14mBJ71m3hnmoPgA3Q0T4rjDox"
	QString name; // "YARROW HOTEL GRILL" or "STRIKE TECHNOLOG"
	double amount = 0.0;
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
		return (3600.0 * 24.0) * (double(date.toJulianDay()) + 0.5);
	}
	double amountDbl() const {
		return amount;
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

class TransactionBundle
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
	Transaction& operator[] (int index) {
		if(index >= 0)
			return *m_vector[index];
		return operator[] (index + m_vector.count());
	}
	const Transaction& operator[] (int index) const { return const_cast<TransactionBundle*>(this)->operator[](index); }
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

private:
	QVector<Transaction*> m_vector;
};

typedef QMap<uint, TransactionBundle> HashedBundles;

class Account
{
public:
	Account() {}

	// loading the json file
	// see this: https://qt-project.org/doc/qt-5-snapshot/qtcore-savegame-example.html
	bool loadPlaidJson(QString jsonFile);

	TransactionBundle& transBundle() {
		return m_allTrans;
	}
	QMap<uint, TransactionBundle>& hashBundles() {
		return m_hashBundles;
	}

private:
	QVector<QString> m_accountIds;
	struct Transactions
	{
		//! json in
		void read(const QJsonObject &json, const QVector<QString>& acIds);
		//! json out
		void write(QJsonObject &json) const;
		static bool isSymetric(const Transaction& first, const Transaction& second) {
			if (first.date == second.date) {
				if (first.amountDbl() == -second.amountDbl()) {
					return true;
				}
			}
			return false;
		}
		QVector<Transaction> transVector;
	};
	//! makes a bundle for each hash value
	void makeHashBundles() {
		for (int i = 0; i < transBundle().count(); ++i) {
			uint h = transBundle()[i].nameHash.hash;
			m_hashBundles[h].append(&transBundle()[i]);
		}
		qDebug() << m_hashBundles.count() << m_hashBundles.keys().first() << m_hashBundles.keys().last();
	}

private:
	Transactions m_transactions;
	TransactionBundle m_allTrans;
	HashedBundles m_hashBundles;
};

class Household
{
	QVector<Account> m_accounts;
};

typedef QVector<QVector<Transaction*> > DailyTransactions;

#endif // ACDATA_H
