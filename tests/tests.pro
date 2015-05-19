include(../defaults.pri)
TEMPLATE = app
QT       += testlib
QT       += network
QT       -= gui

CONFIG   += c++11
#CONFIG   += testCase
#CONFIG   += console
#CONFIG   -= app_bundle

SOURCES += main.cpp
HEADERS += main.h

unix {
LIBS += -L../cacheRest/ -lcacheRest
}
win32 {
LIBS += -L../cacheRest/$${DEBUGRELEASE} -lcacheRest
}
