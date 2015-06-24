#ifndef COMMON_H
#define COMMON_H

#include <QtCore>
#include "core_global.h"
#include "log.h"

static const int MAX_TRANSACTION_PER_ACCOUNT = 1024 * 8;
static const int KLA_MULTIPLICATOR = 1024;
static const int KA_MULTIPLICATOR = 1024;

#define QSTARTSWITH(actual, expected) QCOMPARE(actual.left(QString(expected).length()), QString(expected))

double CORESHARED_EXPORT kindaLog(double amount);
double CORESHARED_EXPORT unKindaLog(double kindaLogAmount);
qint64 CORESHARED_EXPORT proximityHashString(const QString& str);
qint64 CORESHARED_EXPORT proximityHashString2(const QString& str);

static inline qint64 absInt(const qint64& x) {
	return qAbs(x);
}
static inline qint64 absInt(const qint32& x) {
	return (x ^ (x >> 31)) - (x >> 31);
}
static inline float fasterpow2 (float p) {
	float clipp = (p < -126) ? -126.0f : p;
	union { uint32_t i; float f; } v = { static_cast<uint32_t>( (1 << 23) * (clipp + 126.94269504f) ) };
	return v.f;
}
static inline float fasterexp (float p) {
	return fasterpow2 (1.442695040f * p);
}
template <int Lambda>
static inline double expoInt(qint64 x) {
	return qExp(double(x) * (1.0 / double(Lambda)));
}



class CORESHARED_EXPORT DBobj : public QObject
{
	Q_OBJECT

public:
	DBobj(QObject* parent = 0)
		:QObject(parent)
	{}
	DBobj(QString id, QObject* parent = 0)
		:QObject(parent)
	{
		m_id = id;
	}
	~DBobj() {}

	QString id() const {
		return m_id;
	}

protected:
	QString m_id;
};

template<int Dim, typename U>
class FiniteVector
{
public:
	FiniteVector() {
		for (int i = 0; i < Dim; ++i) {
			coord[i] = 0;
		}
	}

	qint64 manDist(const FiniteVector<Dim, U>& other) const {
		qint64 ret = 0;
		for (int i = 0; i < Dim; ++i) {
			ret += qAbs(coord[i] - other.coord[i]);
		}
		return ret;
	}

	qint64 manLength() const {
		qint64 ret = 0;
		for (int i = 0; i < Dim; ++i) {
			ret += qAbs(coord[i]);
		}
		MaxManLengthEver = qMax(MaxManLengthEver, ret);
		return ret;
	}

	static qint64 MaxManLengthEver;

public:
	U coord[Dim];
};
template<int Dim, typename U> qint64 FiniteVector<Dim, U>::MaxManLengthEver = 0;

class NameHashVector1 : public FiniteVector<1, int>
{
public:
	static quint64 fromString(const QString& str) {
		qint64 h = 0;
		for (const QChar& c : str) {
			int n = c.toUpper().toLatin1() * 1;
			// for numbers
			if (c.isDigit())
				n = 0;//QChar('#').toLatin1();
			h += n;
		}
		return h;
	}

	void setFromString(const QString& str) {
		coord[0] = fromString(str);
	}

	void setFromHash(int h) {
		coord[0] = h;
	}

	qint64 dist(const NameHashVector1& other) const {
		return manDist(other);
	}

	qint64 hash() const {
		return manLength();
	}

};

class NameHashVector2 : public FiniteVector<1, quint64>
{
public:
	static quint64 fromString(const QString& str) {
		quint64 h = 0;
		for (const QChar& c : str) {
			int n = c.toUpper().toLatin1();
			if (c.isDigit())
				continue;
			n -= QChar(' ').toLatin1();
			if(n >= 0 && n < 64) {
				// flips the ith bit
				h |=  (1 << (n));
			}
		}
		return h;
	}

	void setFromString(const QString& str) {
		coord[0] = fromString(str);
	}

	void setFromHash(quint64 h) {
		coord[0] = h;
	}

	qint64 dist(const NameHashVector2& other) const {
		quint64 h1 = coord[0];
		quint64 h2 = other.coord[0];
		return pop(h1 ^ h2);
	}

	qint64 hash() const {
		return coord[0];
	}

	qint64 manLength() const {
		return pop(hash());
	}

private:
	int pop(quint64 x) const {
		quint64 ox = x;
		const uint64_t m1  = 0x5555555555555555; //binary: 0101...
		const uint64_t m2  = 0x3333333333333333; //binary: 00110011..
		const uint64_t m4  = 0x0f0f0f0f0f0f0f0f; //binary:  4 zeros,  4 ones ...
		x -= (x >> 1) & m1;             //put count of each 2 bits into those 2 bits
		x = (x & m2) + ((x >> 2) & m2); //put count of each 4 bits into those 4 bits
		x = (x + (x >> 4)) & m4;        //put count of each 8 bits into those 8 bits
		x += x >>  8;  //put count of each 16 bits into their lowest 8 bits
		x += x >> 16;  //put count of each 32 bits into their lowest 8 bits
		x += x >> 32;  //put count of each 64 bits into their lowest 8 bits
		x &= 0x7f;
		Q_ASSERT(x == QString::number(ox, 2).count("1"));
		return x;
	}
};

#endif // COMMON_H
