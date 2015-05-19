TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS = \
	core \
	cacheRest \
	cacheLight \
	tests
	
cacheLight.depends = cacheRest,core
tests.depends = cacheRest,core
