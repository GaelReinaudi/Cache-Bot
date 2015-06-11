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
	$$PWD/../AccountChart/qcustomplot/qcustomplot.cpp \
#	$$PWD/../extraCache/extraCache.cpp

HEADERS  += \
	mainwindow.h \
	$$PWD/log.h \
	$$PWD/../AccountChart/qcustomplot/qcustomplot.h \
#	$$PWD/../extraCache/extraCache.h

FORMS    += mainwindow.ui

unix {
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcacheRest
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcore
}
win32 {
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcacheRest
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcore
}
