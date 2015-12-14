include($$PWD/../defaults.pri)

QT += core gui widgets printsupport network

TARGET = spinner
TEMPLATE = app

SOURCES += main.cpp\
        spinnerwindow.cpp

HEADERS  += spinnerwindow.h

FORMS    += spinnerwindow.ui

unix {
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcacheRest
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcore
}
macx {
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcacheRest
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcore
}
win32 {
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcacheRest
LIBS += -L$${SRC_DIR}/bin/$${DEBUGRELEASE} -lcore
}
