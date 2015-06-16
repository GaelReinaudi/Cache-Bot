include(../defaults.pri)

QT       += core
QT       += network
QT       -= gui

TARGET = extraCash
TEMPLATE = app

CONFIG   += console
CONFIG   -= app_bundle


SOURCES += main.cpp \
	extraCache.cpp

HEADERS += \
	extraCache.h

unix {
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcacheRest
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcore
}
win32 {
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcacheRest
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcore
}

