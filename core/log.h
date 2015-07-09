#ifndef LOG_H
#define LOG_H
#include <QTextStream>
#include <QFile>
#include <QMutexLocker>
#include <QDateTime>
#include <QCoreApplication>

class logger
{
public:
	static logger* Instance() {
		if(!s_pLog) {
			s_pLog = new logger();
			s_pLog->data.setFileName(QString("../../%1.log").arg(qAppName()));
			static bool ret = s_pLog->data.open(QFile::WriteOnly | QFile::Truncate);
			if(ret)
				ret = false;
			s_pLog->optout.setDevice(&s_pLog->data);
			s_pLog->optout << QDateTime::currentDateTime().toString("HH:mm:ss.zzz") << endl;
		}
		return s_pLog;
	}

	QTextStream& stream() {
		QMutexLocker locker(&logMutex);
		return optout;
	}
	QTextStream optout;//(&data);
	QFile data;//(QString("optim.log"));
	static logger* s_pLog;
	QMutex logMutex;
};

#define LOG() logger::Instance()->stream()

#define noLOG(level) QDataStream(0)

#endif // LOG_H
