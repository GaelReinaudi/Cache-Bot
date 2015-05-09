include(../defaults.pri)
QT       += network
QT       -= gui
CONFIG += static

TARGET = cacheRest
TEMPLATE = lib

DEFINES += CACHEREST_LIBRARY

SOURCES += cacherest.cpp \
	httprequestworker.cpp

HEADERS += cacherest.h\
        cacherest_global.h \
    httprequestworker.h

