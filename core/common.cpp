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
		if (n > 0 && n < 64) {
			ret |= quint64(1) << n;
		}
	}
	return ret;
}



