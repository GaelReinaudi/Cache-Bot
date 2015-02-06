#-------------------------------------------------
#
# Project created by QtCreator 2014-12-15T23:41:16
#
#-------------------------------------------------

QT       += core gui widgets printsupport
CONFIG+=c++11

INCLUDEPATH  += ./ \
	$$PWD/puppy/puppy/include \
	$$PWD/../AccountChart \

TARGET = evoCache
TEMPLATE = app

SOURCES += main.cpp\
		mainwindow.cpp\
	$$PWD/puppy/puppy/src/Primitive.cpp \
	$$PWD/puppy/puppy/src/Puppy.cpp \
	$$PWD/puppy/puppy/src/Tree.cpp \
	$$PWD/../AccountChart/qcustomplot/qcustomplot.cpp \
	$$PWD/../AccountChart/ACChart/acustomplot.cpp \
	$$PWD/../AccountChart/ACChart/acdata.cpp \
	$$PWD/../AccountChart/AmortiChart/AMPlot.cpp \
	EvolutionSpinner.cpp \
    AccRegPrimits.cpp

HEADERS  += mainwindow.h \
	$$PWD/../AccountChart/qcustomplot/qcustomplot.h \
	$$PWD/../AccountChart/ACChart/acustomplot.h \
	$$PWD/../AccountChart/ACChart/acdata.h \
	$$PWD/../AccountChart/AmortiChart/AMPlot.h \
	puppy/Primitive.hpp \
	puppy/PrimitiveHandle.hpp \
	puppy/PrimitiveInline.hpp \
	puppy/Puppy.hpp \
	puppy/Context.hpp \
	puppy/Randomizer.hpp \
	puppy/Token.hpp \
	puppy/Tree.hpp \
	$$PWD/log.h \
    EvolutionSpinner.h \
    AccRegPrimits.h


FORMS    += mainwindow.ui


