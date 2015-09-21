INCLUDEPATH += $$PWD/cacheRest
INCLUDEPATH += $$PWD/core
INCLUDEPATH += $$PWD/deps/spdlog/include

GIT_VERSION = $$system(git --git-dir $$PWD/.git --work-tree $$PWD describe --always --tags)
DEFINES += GIT_VERSION=\\\"$$GIT_VERSION\\\"
VERSION = $$GIT_VERSION
win32 {
	VERSION ~= s/-\d+-g[a-f0-9]{6,}//
}

SRC_DIR = $$PWD

CONFIG += c++11

CONFIG(debug, debug|release) {
DEBUGRELEASE = debug
}
else {
DEBUGRELEASE = release
QMAKE_CXXFLAGS += -O3
QMAKE_CXXFLAGS_RELEASE -= -O
QMAKE_CXXFLAGS_RELEASE -= -O1
QMAKE_CXXFLAGS_RELEASE -= -O2
}

DESTDIR = $$PWD/bin/$${DEBUGRELEASE}
DEPENDPATH += $$DESTDIR

MOC_DIR = ./$${DEBUGRELEASE}
OBJECTS_DIR = ./$${DEBUGRELEASE}
UI_DIR = ./$${DEBUGRELEASE}

win32 {
QMAKE_CXXFLAGS += -D_WIN32_WINNT=0x600
}


#HEADERS  += \
#	$$PWD/core/log.h
