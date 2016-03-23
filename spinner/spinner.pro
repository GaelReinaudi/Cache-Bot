include($$PWD/../defaults.pri)

QT += core gui
QT += network
QT += widgets printsupport

TARGET = spinner
TEMPLATE = app

SOURCES += main.cpp\
        spinnerwindow.cpp

HEADERS  += spinnerwindow.h

FORMS    += spinnerwindow.ui
