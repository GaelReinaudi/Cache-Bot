#-------------------------------------------------
#
# Project created by QtCreator 2015-05-21T00:09:39
#
#-------------------------------------------------

QT       -= gui

TARGET = cacheCore
TEMPLATE = lib

DEFINES += CACHECORE_LIBRARY

SOURCES += cachecore.cpp

HEADERS += cachecore.h\
        cachecore_global.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
