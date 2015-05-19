INCLUDEPATH += $$PWD/cacheRest
INCLUDEPATH += $$PWD/core
SRC_DIR = $$PWD

CONFIG += c++11

CONFIG(debug, debug|release) {
DEBUGRELEASE = debug
}
else {
DEBUGRELEASE = release
}

unix {
LIBS += -L../cacheRest/ -lcacheRest
LIBS += -L../core/ -lccore
}
win32 {
LIBS += -L../cacheRest/$${DEBUGRELEASE} -lcacheRest
LIBS += -L../core/$${DEBUGRELEASE} -lcore
}
