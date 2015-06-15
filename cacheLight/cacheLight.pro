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
	$$PWD/../extraCash/extraCache.cpp \
	$$PWD/../AccountChart/qcustomplot/qcustomplot.cpp

HEADERS  += \
	mainwindow.h \
	$$PWD/log.h \
	$$PWD/../extraCache/extraCache.h \
	$$PWD/../AccountChart/qcustomplot/qcustomplot.h

FORMS    += mainwindow.ui

unix {
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcacheRest
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcore
}
win32 {
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcacheRest
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcore
}
