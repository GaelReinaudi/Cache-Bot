include(../defaults.pri)
include(puppy/puppy.pri)

QT += core
QT += network
QT += widgets printsupport

TARGET = evoCache
TEMPLATE = app

INCLUDEPATH  += ./ \
	$$PWD/../AccountChart \
	$$PWD/../

SOURCES += \
	main.cpp \
	evolver.cpp \
	EvolutionSpinner.cpp \
	mainwindow.cpp \
	$$PWD/../AccountChart/qcustomplot/qcustomplot.cpp \
	$$PWD/../AccountChart/ACChart/acustomplot.cpp \
	$$PWD/../AccountChart/AmortiChart/AMPlot.cpp

HEADERS  += \
	evolver.h \
	$$PWD/log.h \
	EvolutionSpinner.h \
	mainwindow.h \
	$$PWD/../AccountChart/qcustomplot/qcustomplot.h \
	$$PWD/../AccountChart/ACChart/acustomplot.h \
	$$PWD/../AccountChart/AmortiChart/AMPlot.h


FORMS    += mainwindow.ui

unix {
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcacheRest
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcore
}
win32 {
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcacheRest
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcore
}
