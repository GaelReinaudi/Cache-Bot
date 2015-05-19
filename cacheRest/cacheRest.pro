include(../defaults.pri)
QT       -= gui
QT       += network

TARGET = cacheRest
TEMPLATE = lib

CONFIG += static

DEFINES += CACHEREST_LIBRARY

SOURCES += cacherest.cpp \
	httprequestworker.cpp

HEADERS += cacherest.h\
        cacherest_global.h \
    httprequestworker.h
