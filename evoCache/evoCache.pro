include(../defaults.pri)
include(puppy/puppy.pri)

QT += core
QT += network
QT -= gui

TARGET = evoCache
TEMPLATE = app

INCLUDEPATH  += ./ \
	$$PWD/../AccountChart \
	$$PWD/../

SOURCES += \
	main.cpp\
	evolver.cpp\
	EvolutionSpinner.cpp

HEADERS  += \
	evolver.h \
	$$PWD/log.h \
	EvolutionSpinner.h


FORMS    += mainwindow.ui

unix {
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcacheRest
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcore
}
win32 {
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcacheRest
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcore
}
