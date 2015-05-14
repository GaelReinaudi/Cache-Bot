TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS = \
    cacheRest \
#    cacheLight \
	tests
	
cacheLight.depends = cacheRest
tests.depends = cacheRest
