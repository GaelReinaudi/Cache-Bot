QT       += core gui widgets printsupport
CONFIG += c++11

TARGET = AMortiChart
TEMPLATE = app

INCLUDEPATH += ../ACChart

SOURCES += \
	main.cpp\
    mainwindow.cpp \
	../qcustomplot/qcustomplot.cpp \
	../ACChart/acdata.cpp \
	AMPlot.cpp

HEADERS  += \
	mainwindow.h \
	../qcustomplot/qcustomplot.h \
	../ACChart/acdata.h \
	AMPlot.h

FORMS    += mainwindow.ui
