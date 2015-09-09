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
    userMetrics.cpp \
	oracle.cpp \
	$$PWD/../evoCache/puppy/AccRegPrimits.cpp \
	$$PWD/../evoCache/puppy/featureAllOthers.cpp \
	$$PWD/../evoCache/puppy/featureStatDistrib.cpp \
	$$PWD/../evoCache/puppy/featurePeriodicAmount.cpp \
	$$PWD/../evoCache/puppy/featureOutlier.cpp \
	$$PWD/../evoCache/puppy/featurePriceWindow.cpp \
	$$PWD/../evoCache/puppy/featureLabelDistrib.cpp


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
    userMetrics.h \
    oracle.h

