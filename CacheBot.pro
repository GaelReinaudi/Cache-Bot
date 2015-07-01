# https://wiki.qt.io/How_to_create_a_library_with_Qt_and_use_it_in_an_application

TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS = \
	core \
	cacheRest \
	tests \
	evoCache \
	evoCacheView \
	cacheLight \
	extraCash \
	userViewer \
	metricViewer
	
cacheRest.depends = core
evoCache.depends = cacheRest,core
evoCacheView.depends = cacheRest,core
cacheLight.depends = cacheRest,core
tests.depends = cacheRest,core
extraCash.depends = cacheRest,core
userViewer.depends = cacheRest,core
metricViewer.depends = cacheRest,core
