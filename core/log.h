#ifndef LOG_H
#define LOG_H
#include <QTextStream>
#include <QFile>
#include <QMutexLocker>
#include <QDateTime>
#include <QCoreApplication>

#include <iostream>
#include "spdlog/spdlog.h"

std::ostream& operator<<(std::ostream& os, const QString& c);
std::ostream& operator<<(std::ostream& os, const QJsonObject& c);
//std::ostream& operator<<(std::ostream& os, const QByteArray& c);

class logger
{
public:
	static void setupSpdLog(QString logFileName);

	static logger* Instance() {
		if(!s_pLog) {
			s_pLog = new logger();
			QString logFileName = QString("../../%1.log").arg(qAppName());

//			s_pLog->data.setFileName(logFileName);
//			static bool ret = s_pLog->data.open(QFile::WriteOnly | QFile::Truncate);
//			if(ret)
//				ret = false;
//			s_pLog->optout.setDevice(&s_pLog->data);
//			s_pLog->optout << QDateTime::currentDateTime().toString("HH:mm:ss.zzz") << endl;


			setupSpdLog(logFileName);


		}
		return s_pLog;
	}

	QTextStream& stream() {
		QMutexLocker locker(&logMutex);
		return optout;
	}
	std::shared_ptr<spdlog::logger> m_fileLogger;
	QTextStream optout;//(&data);
	QFile data;//(QString("optim.log"));
	static logger* s_pLog;
	QMutex logMutex;
};

// 0 Emergency: system is unusable
// 1 Alert: action must be taken immediately
// 2 Critical: critical conditions
// 3 Error: error conditions
// 4 Warning: warning conditions
// 5 Notice: normal but significant condition
// 6 Informational: informational messages
// 7 Debug: debug-level messages
//#define DBG() logger::Instance()->m_fileLogger->debug()
#define DBG(lev) logger::Instance()->m_fileLogger->debug()
#define INFO() logger::Instance()->m_fileLogger->info()
#define NOTICE() logger::Instance()->m_fileLogger->notice()
#define WARN() logger::Instance()->m_fileLogger->warn()
#define ERR() logger::Instance()->m_fileLogger->error()
#define ALERT() logger::Instance()->m_fileLogger->alert()

//#define LOG() logger::Instance()->stream()

//#define noLOG(level) QDataStream(0)

#endif // LOG_H
