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
