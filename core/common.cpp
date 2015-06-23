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

