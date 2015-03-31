QT += core gui widgets printsupport
CONFIG += c++11

INCLUDEPATH  += ./ \
	$$PWD/puppy/puppy/include \
	$$PWD/../AccountChart \
	$$PWD/../

TARGET = evoCache
TEMPLATE = app

SOURCES += \
	main.cpp\
	mainwindow.cpp\
	$$PWD/puppy/puppy/src/Primitive.cpp \
	$$PWD/puppy/puppy/src/Puppy.cpp \
	$$PWD/puppy/puppy/src/Tree.cpp \
	$$PWD/../AccountChart/qcustomplot/qcustomplot.cpp \
	$$PWD/../AccountChart/ACChart/acustomplot.cpp \
	$$PWD/../AccountChart/AmortiChart/AMPlot.cpp \
	$$PWD/../core/acdata.cpp \
	EvolutionSpinner.cpp \
    AccRegPrimits.cpp

HEADERS  += \
	mainwindow.h \
	$$PWD/log.h \
	$$PWD/../AccountChart/qcustomplot/qcustomplot.h \
	$$PWD/../AccountChart/ACChart/acustomplot.h \
	$$PWD/../core/acdata.h \
	$$PWD/../AccountChart/AmortiChart/AMPlot.h \
	puppy/Primitive.hpp \
	puppy/PrimitiveHandle.hpp \
	puppy/PrimitiveInline.hpp \
	puppy/Puppy.hpp \
	puppy/Context.hpp \
	puppy/Randomizer.hpp \
	puppy/Token.hpp \
	puppy/Tree.hpp \
    EvolutionSpinner.h \
    AccRegPrimits.h


FORMS    += mainwindow.ui


