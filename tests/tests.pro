include(../defaults.pri)
TEMPLATE = app
QT       += testlib
QT       += network
QT       -= gui

#CONFIG   += testCase
#CONFIG   += console
#CONFIG   -= app_bundle

SOURCES += main.cpp
HEADERS += main.h

unix {
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcacheRest
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcore
}
win32 {
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcacheRest
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcore
}

# to use libraries in directory of executable
unix:!mac{
QMAKE_LFLAGS += -Wl,--rpath=\$ORIGIN
QMAKE_LFLAGS += -Wl,--rpath=\$ORIGIN/lib
QMAKE_LFLAGS += -Wl,--rpath=\$ORIGIN/libs
}
