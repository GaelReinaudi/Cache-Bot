TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS = \
	core \
	cacheRest \
	cacheLight \
	tests
	
cacheRest.depends = core
cacheLight.depends = cacheRest,core
tests.depends = cacheRest,core
