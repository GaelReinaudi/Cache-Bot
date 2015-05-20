include(../defaults.pri)
QT       -= gui

TARGET = core
TEMPLATE = lib

#CONFIG += staticlib
DEFINES += CORE_LIBRARY

SOURCES += \
	common.cpp \
	account.cpp \
    user.cpp \
	transaction.cpp


HEADERS += \
	core_global.h \
	common.h \
	log.h \
	account.h \
	user.h \
    transaction.h

