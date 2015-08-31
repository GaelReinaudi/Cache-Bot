#include "core_global.h"
#include "log.h"

CORESHARED_EXPORT logger* logger::s_pLog = 0;


std::ostream &operator<<(std::ostream &os, const QString &c)
{
	return os << c.toStdString();
}
