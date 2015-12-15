# https://wiki.qt.io/How_to_create_a_library_with_Qt_and_use_it_in_an_application

TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS = \
	core \
	cacheRest \
	tests \
	evoCache \
	extraCash

macx | win32 {
SUBDIRS += \
	evoCacheView \
	cacheLight \
	userViewer \
	metricViewer \
	spinner

evoCacheView.depends = cacheRest,core,evoCache
cacheLight.depends = cacheRest,core,extraCash
userViewer.depends = cacheRest,core,evoCache
metricViewer.depends = cacheRest,core,evoCache,extraCash
}

cacheRest.depends = core
tests.depends = cacheRest,core
evoCache.depends = cacheRest,core
extraCash.depends = cacheRest,core,evoCache
