#include "core_global.h"
#include "log.h"
#include <QJsonDocument>

CORESHARED_EXPORT logger* logger::s_pLog = 0;


std::ostream CORESHARED_EXPORT &operator<<(std::ostream &os, const QString &c)
{
	return os << c.toStdString();
}
std::ostream CORESHARED_EXPORT &operator<<(std::ostream &os, const QJsonObject &c)
{
	return os << QString(QJsonDocument(c).toJson()).toStdString();
}
//std::ostream CORESHARED_EXPORT &operator<<(std::ostream &os, const QByteArray &c)
//{
//	return os << QString(c);
//}


void CORESHARED_EXPORT logger::setupSpdLog(QString logFileName)
{
	try
	{
		//Create console, multithreaded logger
//		spdlog::set_async_mode(1024);
#ifdef Q_OS_LINUX
		s_pLog->m_fileLogger = spdlog::stdout_logger_mt("console");
		s_pLog->m_fileLogger->set_level(spdlog::level::warn);
#else
//		s_pLog->m_fileLogger = spdlog::daily_logger_mt("file_logger", "myfilename");
		s_pLog->m_fileLogger = spdlog::create<spdlog::sinks::rotating_file_sink_mt>("file", logFileName.toStdString(), "log", 1024 * 1024 * 20, 1, true);
//		s_pLog->m_fileLogger = spdlog::create<spdlog::sinks::simple_file_sink_mt>("simplefile", logFileName.toStdString() + ".log", true);
		s_pLog->m_fileLogger->set_level(spdlog::level::info);
#endif
		s_pLog->m_fileLogger->set_pattern("[%l] %v");
		s_pLog->m_fileLogger->warn() << "********** NEW SESSION ********** "
									 << QDateTime::currentDateTime().toString("HH:mm:ss.zzz")
									 << "    GIT_VERSION: " << QString(GIT_VERSION) << "    ";
	}
	catch (const spdlog::spdlog_ex& ex)
	{
		std::cout << "Log failed: " << ex.what() << std::endl;
	}
}
