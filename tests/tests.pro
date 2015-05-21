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
LIBS += -L../bin -lcacherest
LIBS += -L../bin -lcore
}
win32 {
LIBS += -L../bin/$${DEBUGRELEASE} -lcacheRest
LIBS += -L../bin/$${DEBUGRELEASE} -lcore
}
