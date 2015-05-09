include(../defaults.pri)
TEMPLATE = app
QT       += testlib
QT       -= gui

CONFIG   += console
CONFIG   -= app_bundle
CONFIG   -= qt

SOURCES += main.cpp

LIBS += \
	-lunittest++ \
	-L../cacheRest -lcacheRest
