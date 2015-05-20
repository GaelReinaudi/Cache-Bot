include(../defaults.pri)
TEMPLATE = app
QT       += testlib
QT       += network
QT       -= gui

#CONFIG   += testCase
#CONFIG   += console
#CONFIG   -= app_bundle

SOURCES += main.cpp
HEADERS += main.h


unix {
LIBS += -L../cacheRest/ -lcacheRest
LIBS += -L../core/ -lcore
}
win32 {
LIBS += -L../cacheRest/$${DEBUGRELEASE} -lcacheRest
LIBS += -L../core/$${DEBUGRELEASE} -lcore
}
