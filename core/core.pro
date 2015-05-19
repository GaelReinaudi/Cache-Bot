include(../defaults.pri)
QT       -= gui

TARGET = core
TEMPLATE = lib

CONFIG += staticlib

SOURCES += \
	acdata.cpp \
    common.cpp \
    user.cpp \
    transaction.cpp


HEADERS += \
	acdata.h \
	common.h \
	log.h \
    user.h \
    transaction.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
