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
	userLauncher

evoCacheView.depends = cacheRest,core,evoCache,extraCash
cacheLight.depends = cacheRest,core,evoCache,extraCash
userViewer.depends = cacheRest,core,evoCache,extraCash
metricViewer.depends = cacheRest,core,evoCache,extraCash
userLauncher.depends = evoCacheView,cacheLight,userViewer,metricViewer
}

cacheRest.depends = core
tests.depends = cacheRest,core
evoCache.depends = cacheRest,core
extraCash.depends = cacheRest,core,evoCache
