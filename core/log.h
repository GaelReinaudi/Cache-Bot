#ifndef LOG_H
#define LOG_H

class logger
{
public:
	static QTextStream& stream() {
		static QFile data("optim.log");
		static bool ret = data.open(QFile::WriteOnly | QFile::Truncate);
		ret |= true;
		static QTextStream optout(&data);
		return optout << QDateTime::currentDateTime().toString("HH:mm:ss.zzz ");
	}
};
#define LOG() logger::stream()

#endif // LOG_H
