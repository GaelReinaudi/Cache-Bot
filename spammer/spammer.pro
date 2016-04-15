include($$PWD/../defaults.pri)

QT += core gui
QT += widgets
QT += network

TARGET = spammer
TEMPLATE = app

SOURCES += main.cpp\
	spammerwindow.cpp

HEADERS  += spammerwindow.h

FORMS    += spammerwindow.ui

LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcacheRest
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcore
