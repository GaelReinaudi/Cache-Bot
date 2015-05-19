include(../defaults.pri)
QT       -= gui

TARGET = core
TEMPLATE = lib

CONFIG += staticlib

SOURCES += \
	common.cpp \
	account.cpp \
    user.cpp \
	transaction.cpp


HEADERS += \
	common.h \
	log.h \
	account.h \
	user.h \
    transaction.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
