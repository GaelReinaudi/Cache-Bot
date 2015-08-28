include($$PWD/../defaults.pri)
include($$PWD/../evoCache/puppy/puppy.pri)

QT += core gui widgets printsupport network

TARGET = cacheLight
TEMPLATE = app

INCLUDEPATH  += ./ \
	$$PWD/../ \
	$$PWD/../AccountChart/ \
	$$PWD/../AccountChart/qcustomplot


SOURCES += \
	main.cpp \
	$$PWD/../extraCash/extraCache.cpp \
	$$PWD/../AccountChart/qcustomplot/qcustomplot.cpp \
    extraCashView.cpp

HEADERS  += \
	$$PWD/../extraCache/extraCache.h \
	$$PWD/../AccountChart/qcustomplot/qcustomplot.h \
    extraCashView.h

FORMS    += extraCashView.ui

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
