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
		return ret;
	}

public:
	U coord[Dim];
};

class NameHashVector1 : public FiniteVector<1, int>
{
public:
	void setFromString(const QString& str) {

	}

	qint64 dist(const NameHashVector1& other) const {
		return manDist(other);
	}

	qint64 hash() const {
		return manLength();
	}

};

#endif // COMMON_H
