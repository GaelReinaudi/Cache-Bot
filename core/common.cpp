#include "common.h"

double kindaLog(double amount) {
	if (amount < 0)
		return -qLn(-amount+1.0) * 0.43429448190325182765112891891661; // 1 / 2.3025850929940456840179914546844;
	return qLn(amount+1.0) * 0.43429448190325182765112891891661; // 1 / 2.3025850929940456840179914546844;
}
double unKindaLog(double kindaLogAmount)
{
	if (kindaLogAmount < 0)
		return 1.0 - qExp(-kindaLogAmount * 2.3025850929940456840179914546844); // 1 / 0.43429448190325182765112891891661
	return -1.0 + qExp(kindaLogAmount * 2.3025850929940456840179914546844); // 1 / 0.43429448190325182765112891891661
}

double toSignifDigit_2(double val)
{
return toBillDigits_2(val);
	if (qAbs(val) < 1.0)
		return double(int(val * 100) / 100.0);
	if (qAbs(val) < 10.0)
		return double(int(val * 10) / 10.0);
	if (qAbs(val) < 100.0)
		return int(val);
	if (qAbs(val) < 1000.0)
		return 10 * int(val / 10.0);
	if (qAbs(val) < 10000.0)
		return 100 * int(val / 100.0);
	if (qAbs(val) < 100000.0)
		return 1000 * int(val / 1000.0);
	return 10000 * int(val / 10000.0);
}

double toBillDigits_2(double val)
{
	if (val == 0.0)
		return 0;
	double fac = val > 0 ? 1.0 : -1.0;
	val = qAbs(val);
	while (val > 100.0) {
		val /= 10.0;
		fac *= 10.0;
	}
	while (val < 10.0) {
		val *= 10.0;
		fac /= 10.0;
	}

	if (val < 12.5)
		val = 10;
	else if (val < 17.5)
		val = 15;
	else if (val < 25.0)
		val = 20;
	else if (val < 40.0)
		val = 30;
	else if (val < 62.0)
		val = 50;
	else if (val < 85.0)
		val = 75;
	else
		val = 100;

	return fac * val;
}

qint64 proximityHashString(const QString &str) {
	qint64 ret = 0;
	for (const QChar& c : str) {
		int n = c.toUpper().toLatin1() * 1;
		// for numbers
		if (c.isDigit())
			n = 0;//QChar('#').toLatin1();
		ret += n;
	}
	return ret;
}

qint64 proximityHashString2(const QString &str) {
	qint64 ret = 0;
	for (const QChar& c : str) {
		int n = c.toUpper().toLatin1() * 1;
		// getting the index from symbol '!' == 21
		n -= QChar('!').toLatin1();
		if (n > 0 && n < 63) {
			ret |= qint64(1) << n;
		}
	}
	return ret;
}

qint64 NameHashVector2::fromString(const QString &str, double kla) {
	qint64 h = 0;
	for (const QChar& c : str) {
		int n = c.toUpper().toLatin1();
		if (c.isDigit() || c == QChar('#') || c == QChar(' '))
			continue;
		n -= QChar('@').toLatin1();
		if(n >= 0 && n < 25) {
			// flips the nth bit
			h |=  (Q_INT64_C(1) << n);
		}
	}
	if (kla >= 0)
		return h;
	return h ^ (Q_INT64_C(1) << 29);
}

qint64 NameHashVector2::dist(const NameHashVector2 &other) const {
	qint64 h1 = (m_h);
	qint64 h2 = (other.m_h);
	// tries to make differences of short labels look more important
	int allBits = numBits(h1 | h2);
	if (allBits <= 6)
		return numBits(h1 ^ h2) * (7 - allBits);
	// the number of bits that are different
	return numBits(h1 ^ h2);
}

int NameHashVector2::numBits(qint64 n) const {
	uint64_t x = *reinterpret_cast<uint64_t*>(&n);
	const uint64_t m1  = Q_UINT64_C(0x5555555555555555); //binary: 0101...
	const uint64_t m2  = Q_UINT64_C(0x3333333333333333); //binary: 00110011..
	const uint64_t m4  = Q_UINT64_C(0x0f0f0f0f0f0f0f0f); //binary:  4 zeros,  4 ones ...
	x -= (x >> 1) & m1;             //put count of each 2 bits into those 2 bits
	x = (x & m2) + ((x >> 2) & m2); //put count of each 4 bits into those 4 bits
	x = (x + (x >> 4)) & m4;        //put count of each 8 bits into those 8 bits
	x += x >>  8;  //put count of each 16 bits into their lowest 8 bits
	x += x >> 16;  //put count of each 32 bits into their lowest 8 bits
	x += x >> 32;  //put count of each 64 bits into their lowest 8 bits
	x &= 0x7f;
	Q_ASSERT(int(x) == QString::number(n, 2).count("1"));
	return x;
}

QVector<double> _baseSystem = {50, 100, 250, 500, 750, 1000, 1500, 2000, 3000, 5000, 7500, 10000};

double roundToBaseSystem(double v)
{
	int i = 0;
	while (i < _baseSystem.count() && v/_baseSystem[i] > 1) {
		i++;
	}
	double x = 0;
	if (i == 0) {
		x = _baseSystem[i];
	}
	else {
		double left = v - _baseSystem[i - 1];
		double right = _baseSystem[i] - v;
		if (left < right) {
			x = _baseSystem[i - 1];
		}
		else {
			x = _baseSystem[i];
		}
	}
	WARN() << v << " rounded to base system: " <<  x << " (" << i << ")";
	return x;
}

double roundByBaseSystem(double v, double d)
{
	// normalizes dev to a clean number in the _baseSystems provided above
	d = roundToBaseSystem(d);
	// round this to the nearest dev
	double x = qFloor(v / d) * d;
	return x;
}
