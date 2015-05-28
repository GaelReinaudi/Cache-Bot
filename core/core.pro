include(../defaults.pri)
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
	transaction.cpp


HEADERS += \
	core_global.h \
	common.h \
	log.h \
	account.h \
	user.h \
    transaction.h

unix {
	target.path = /usr/lib
	INSTALLS += target
}
