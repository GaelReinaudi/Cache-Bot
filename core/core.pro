include($$PWD/../defaults.pri)
include($$PWD/../evoCache/puppy/puppy.pri)
QT       -= gui

TARGET = core
TEMPLATE = lib

#CONFIG += staticlib
DEFINES += CORE_LIBRARY

SOURCES += \
	common.cpp \
	log.cpp \
	account.cpp \
    user.cpp \
	transaction.cpp \
    bot.cpp \
    fund.cpp \
    botContext.cpp \
    histoMetric.cpp \
    userMetrics.cpp


HEADERS += \
	core_global.h \
	common.h \
	log.h \
	account.h \
	user.h \
    transaction.h \
    bot.h \
    fund.h \
    botContext.h \
    histoMetric.h \
    userMetrics.h

