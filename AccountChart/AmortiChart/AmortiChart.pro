QT     += core gui widgets printsupport
CONFIG += c++11

TARGET = AMortiChart
TEMPLATE = app

INCLUDEPATH  += \
	../ \
	../ACChart \
	$$PWD/../../

SOURCES += \
	main.cpp\
    mainwindow.cpp \
	../qcustomplot/qcustomplot.cpp \
	../../core/acdata.cpp \
	AMPlot.cpp

HEADERS  += \
	mainwindow.h \
	AMPlot.h \
	../qcustomplot/qcustomplot.h \
	$$PWD/../../core/acdata.h

FORMS    += mainwindow.ui
