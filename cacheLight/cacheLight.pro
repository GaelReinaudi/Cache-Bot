QT += core gui widgets printsupport
CONFIG += c++11

INCLUDEPATH  += ./ \
	$$PWD/../ \
	$$PWD/../AccountChart/ \
	$$PWD/../AccountChart/qcustomplot/

TARGET = cacheLight
TEMPLATE = app

SOURCES += \
	main.cpp\
	mainwindow.cpp\
	$$PWD/../AccountChart/qcustomplot/qcustomplot.cpp \
	$$PWD/../core/acdata.cpp

HEADERS  += \
	mainwindow.h \
	$$PWD/log.h \
	$$PWD/../AccountChart/qcustomplot/qcustomplot.h \
	$$PWD/../core/acdata.h

FORMS    += mainwindow.ui


