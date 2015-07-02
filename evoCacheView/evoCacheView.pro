include($$PWD/../defaults.pri)
include($$PWD/../evoCache/puppy/puppy.pri)

QT += core
QT += network
QT += widgets printsupport

TARGET = evoCacheView
TEMPLATE = app

INCLUDEPATH  += ./ \
	$$PWD/../AccountChart \
	$$PWD/../evoCache

SOURCES += \
	$$PWD/main.cpp \
	$$PWD/mainwindow.cpp \
	$$PWD/../evoCache/evolver.cpp \
	$$PWD/../evoCache/EvolutionSpinner.cpp \
	$$PWD/../AccountChart/qcustomplot/qcustomplot.cpp \
	$$PWD/../AccountChart/ACChart/acustomplot.cpp \
	$$PWD/../AccountChart/AmortiChart/AMPlot.cpp

HEADERS  += \
	$$PWD/mainwindow.h \
	$$PWD/../evoCache/evolver.h \
	$$PWD/../evoCache/EvolutionSpinner.h \
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
