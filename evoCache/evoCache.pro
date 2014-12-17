#-------------------------------------------------
#
# Project created by QtCreator 2014-12-15T23:41:16
#
#-------------------------------------------------

QT       += core gui widgets printsupport
CONFIG+=c++11

INCLUDEPATH  += ./ \
	$$PWD/puppy/puppy/include \
	$$PWD/../ACChart

TARGET = evoCache
TEMPLATE = app

SOURCES += main.cpp\
		mainwindow.cpp\
	$$PWD/puppy/puppy/src/Primitive.cpp \
	$$PWD/puppy/puppy/src/Puppy.cpp \
	$$PWD/puppy/puppy/src/Tree.cpp \
	$$PWD/puppy/SymbRegPrimits.cpp \
	$$PWD/../AccountChart/qcustomplot/qcustomplot.cpp \

HEADERS  += mainwindow.h \
	$$PWD/../AccountChart/qcustomplot/qcustomplot.h \
	$$PWD/puppy/puppy/Primitive.hpp \
	$$PWD/puppy/puppy/PrimitiveHandle.hpp \
	$$PWD/puppy/puppy/PrimitiveInline.hpp \
	$$PWD/puppy/puppy/Puppy.hpp \
	$$PWD/puppy/puppy/Randomizer.hpp \
	$$PWD/puppy/puppy/Token.hpp \
	$$PWD/puppy/puppy/Tree.hpp \
	$$PWD/log.h


FORMS    += mainwindow.ui


