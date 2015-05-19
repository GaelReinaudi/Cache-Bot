#ifndef COMMON_H
#define COMMON_H

#include <QtCore>

static const int MAX_TRANSACTION_PER_ACCOUNT = 1024 * 8;
static const int KLA_MULTIPLICATOR = 128;
static const int KA_MULTIPLICATOR = 1024;

double kindaLog(double amount);
double unKindaLog(double kindaLogAmount);
unsigned int proximityHashString(const QString& str);


#endif // COMMON_H
