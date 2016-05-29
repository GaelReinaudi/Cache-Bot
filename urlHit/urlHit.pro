include($$PWD/../defaults.pri)
include($$PWD/../evoCache/puppy/puppy.pri)

QT       += core
QT       += network
QT       -= gui

TARGET = urlHit
TEMPLATE = app
#CONFIG += console
#CONFIG -= app_bundle


SOURCES += main.cpp \
    cacheConnect.cpp

LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcacheRest
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcore

HEADERS += \
    cacheConnect.h
