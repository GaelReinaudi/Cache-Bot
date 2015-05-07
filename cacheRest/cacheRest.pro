#-------------------------------------------------
#
# Project created by QtCreator 2015-05-06T20:31:41
#
#-------------------------------------------------

QT       += network testlib

QT       -= gui

TARGET = cacheRest
TEMPLATE = lib

DEFINES += CACHEREST_LIBRARY

SOURCES += cacherest.cpp

HEADERS += cacherest.h\
        cacherest_global.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
