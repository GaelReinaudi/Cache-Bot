# https://wiki.qt.io/How_to_create_a_library_with_Qt_and_use_it_in_an_application

TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS = \
	core \
	cacheRest \
	tests \
	evoCache \
	cacheLight \
    extraCash
	
cacheRest.depends = core
evoCache.depends = cacheRest,core
cacheLight.depends = cacheRest,core
tests.depends = cacheRest,core
extraCash.depends = cacheRest,core
