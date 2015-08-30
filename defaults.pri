INCLUDEPATH += $$PWD/cacheRest
INCLUDEPATH += $$PWD/core
INCLUDEPATH += $$PWD/deps/spdlog/include

SRC_DIR = $$PWD

CONFIG += c++11

CONFIG(debug, debug|release) {
DEBUGRELEASE = debug
}
else {
DEBUGRELEASE = release
}

DESTDIR = $$PWD/bin/$${DEBUGRELEASE}
DEPENDPATH += $$DESTDIR

MOC_DIR = ./$${DEBUGRELEASE}
OBJECTS_DIR = ./$${DEBUGRELEASE}
UI_DIR = ./$${DEBUGRELEASE}

QMAKE_CXXFLAGS += -D_WIN32_WINNT=0x600


#HEADERS  += \
#	$$PWD/core/log.h
