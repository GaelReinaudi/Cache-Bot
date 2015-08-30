#ifndef LOG_H
#define LOG_H
#include <QTextStream>
#include <QFile>
#include <QMutexLocker>
#include <QDateTime>
#include <QCoreApplication>

#include <iostream>
#include "spdlog/spdlog.h"

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


			try
			{
				 //Create console, multithreaded logger
				auto console = spdlog::stdout_logger_mt("console");
				console->info("Welcome to spdlog!") ;
				console->info("An info message example {} ..", 1);
				console->info() << "Streams are supported too  " << 2;
				// create a file rotating logger with 5mb size max and 3 rotated files
				auto file_logger = spdlog::rotating_logger_mt("file_logger", "myfilename", 1024 * 1024 * 5, 3);
				file_logger->info("Hello spdlog {} {} {}", 1, 2, "three");
			}
			catch (const spdlog::spdlog_ex& ex)
			{
				std::cout << "Log failed: " << ex.what() << std::endl;
			}


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
