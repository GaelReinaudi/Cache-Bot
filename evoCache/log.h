#ifndef LOG_H
#define LOG_H

#include <qapplication.h>

class logger
{
public:
	static QTextStream& stream() {
		static QFile data("optim.log");
		static bool ret = data.open(QFile::WriteOnly | QFile::Truncate);
		static QTextStream optout(&data);
		return optout << QDateTime::currentDateTime().toString("yyyMMdddTHH:mm:ss.zzz ");
	}
};
#define LOG() logger::stream()

#endif // LOG_H
