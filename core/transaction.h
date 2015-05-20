#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "core_global.h"
#include "common.h"

class CORESHARED_EXPORT Transaction// : public DBobj
{
public:
	Transaction()
	{}
private:
	//	double amount = 0.0;
		qint64 kamount = 0; // integer = round(amount * Mult)
		double kla = 0; // Mult * kindaLog(amount)
public:
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

	enum Flag { None = 0x0, Predicted = 0x1, CameTrue = 0x2 };
	int flags = 0;

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
		return distanceWeighted<8, 1, 128, 0>(other);
	}
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
