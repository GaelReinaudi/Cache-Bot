include(../defaults.pri)
QT       -= gui
QT       += network

TARGET = cacheRest
TEMPLATE = lib

DEFINES += CACHEREST_LIBRARY

SOURCES += cacherest.cpp \
	httprequestworker.cpp

HEADERS += \
	cacherest_global.h \
	httprequestworker.h \
	cacherest.h

unix {
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcore
}
win32 {
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcore
}

unix {
	target.path = /usr/lib
	INSTALLS += target
}
