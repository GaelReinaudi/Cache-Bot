include(../defaults.pri)

QT += core gui widgets printsupport network

TARGET = cacheLight
TEMPLATE = app

INCLUDEPATH  += ./ \
	$$PWD/../ \
	$$PWD/../AccountChart/ \
	$$PWD/../AccountChart/qcustomplot


SOURCES += \
	main.cpp \
	mainwindow.cpp \
	$$PWD/../AccountChart/qcustomplot/qcustomplot.cpp

HEADERS  += \
	mainwindow.h \
	$$PWD/log.h \
	$$PWD/../AccountChart/qcustomplot/qcustomplot.h

FORMS    += mainwindow.ui

unix {
LIBS += -L../bin/$${DEBUGRELEASE} -lcacheRest
LIBS += -L../bin/$${DEBUGRELEASE} -lcore
}
win32 {
LIBS += -L../bin/$${DEBUGRELEASE} -lcacheRest
LIBS += -L../bin/$${DEBUGRELEASE} -lcore
}
