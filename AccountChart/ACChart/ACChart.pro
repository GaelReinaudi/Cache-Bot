#-------------------------------------------------
#
# Project created by QtCreator 2014-12-13T16:12:19
#
#-------------------------------------------------

QT       += core gui printsupport
CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ACChart
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    ../qcustomplot/qcustomplot.cpp \
    acustomplot.cpp \
    acdata.cpp

HEADERS  += mainwindow.h \
    ../qcustomplot/qcustomplot.h \
    acustomplot.h \
    acdata.h

FORMS    += mainwindow.ui
