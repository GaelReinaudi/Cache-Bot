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

	static logger* Instance(QString appendFileStr = "") {
		if(!s_pLog) {
			s_pLog = new logger();
			QString logFileName = QString("../../logs/%1").arg(qAppName());
			if (!appendFileStr.isEmpty()) {
				logFileName.append(QString("-%1").arg(appendFileStr));
			}

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

#define CREATE_LOGGER(str) logger::Instance(str);

// 0 Emergency: system is unusable
// 1 Alert: action must be taken immediately
// 2 Critical: critical conditions
// 3 Error: error conditions
// 4 Warning: warning conditions
// 5 Notice: normal but significant condition
// 6 Informational: informational messages
// 7 Debug: debug-level messages
//#define DBG() logger::Instance()->m_fileLogger->debug()

#define OUT_PUT_LOG logger::Instance()->m_fileLogger
#define DBG(lev) OUT_PUT_LOG->debug()
#define INFO()   OUT_PUT_LOG->info()
#define NOTICE() OUT_PUT_LOG->notice()
#define WARN()   OUT_PUT_LOG->warn()
#define ERR()    OUT_PUT_LOG->error()
#define ALERT()  OUT_PUT_LOG->alert()

//#define LOG() logger::Instance()->stream()

//#define noLOG(level) QDataStream(0)

#endif // LOG_H
