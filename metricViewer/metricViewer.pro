include($$PWD/../defaults.pri)
include($$PWD/../evoCache/puppy/puppy.pri)

QT += core
QT += network
QT += widgets printsupport

TARGET = metricViewer
TEMPLATE = app

INCLUDEPATH  += ./ \
	$$PWD/../AccountChart

SOURCES += \
	main.cpp \
	$$PWD/../AccountChart/ACChart/acustomplot.cpp \
	$$PWD/../AccountChart/qcustomplot/qcustomplot.cpp \
    metricViewer.cpp

HEADERS  += \
	$$PWD/log.h \
	$$PWD/../AccountChart/ACChart/acustomplot.h \
	$$PWD/../AccountChart/qcustomplot/qcustomplot.h \
    metricViewer.h

FORMS    += metricViewer.ui

unix {
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcacheRest
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcore
}
win32 {
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcacheRest
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcore
}
