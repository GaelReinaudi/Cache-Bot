include($$PWD/../defaults.pri)

QT += core gui
QT += widgets

TARGET = userLauncher
TEMPLATE = app

SOURCES += main.cpp\
	userLauncherWindow.cpp \
    flowWidget.cpp

HEADERS  += userLauncherWindow.h \
    flowWidget.h

FORMS    += userLauncherWindow.ui \
    flowWidget.ui
