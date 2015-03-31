QT += core gui printsupport
CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ACChart
TEMPLATE = app

INCLUDEPATH += \
	$$PWD/../.. \
	$$PWD/..

SOURCES += \
	main.cpp \
	mainwindow.cpp \
	acustomplot.cpp \
	../qcustomplot/qcustomplot.cpp \
	../../core/acdata.cpp

HEADERS  += \
	mainwindow.h \
	acustomplot.h \
	core/acdata.h \
	../qcustomplot/qcustomplot.h

FORMS    += mainwindow.ui
