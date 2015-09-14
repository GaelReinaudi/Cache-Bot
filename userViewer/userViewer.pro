include($$PWD/../defaults.pri)
include($$PWD/../evoCache/puppy/puppy.pri)

QT += core
QT += network
QT += widgets printsupport

TARGET = userViewer
TEMPLATE = app

INCLUDEPATH  += ./ \
	$$PWD/../AccountChart

SOURCES += \
	main.cpp \
	$$PWD/../AccountChart/ACChart/acustomplot.cpp \
	$$PWD/../AccountChart/AmortiChart/AMPlot.cpp \
	$$PWD/../AccountChart/qcustomplot/qcustomplot.cpp \
    userViewer.cpp


HEADERS  += \
	$$PWD/../AccountChart/ACChart/acustomplot.h \
	$$PWD/../AccountChart/AmortiChart/AMPlot.h \
	$$PWD/../AccountChart/qcustomplot/qcustomplot.h \
    userViewer.h

FORMS    += userViewer.ui

unix {
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcacheRest
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcore
}
macx {
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcacheRest
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcore
}
win32 {
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcacheRest
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcore
}
