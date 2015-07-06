#ifndef ORACLE_H
#define ORACLE_H
#include "core_global.h"
#include "common.h"

class CORESHARED_EXPORT Oracle : public QObject
{
public:
	Oracle(QObject* parent)
		: QObject(parent)
	{}
};

#endif // ORACLE_H
