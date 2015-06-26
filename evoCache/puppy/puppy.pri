INCLUDEPATH  += $$PWD \
	$$PWD/puppy/include

SOURCES += \
	$$PWD/puppy/src/Primitive.cpp \
	$$PWD/puppy/src/Puppy.cpp \
	$$PWD/puppy/src/Tree.cpp \
	$$PWD/AccRegPrimits.cpp \
    $$PWD/featureAllOthers.cpp

HEADERS  += \
	$$PWD/puppy/Primitive.hpp \
	$$PWD/puppy/PrimitiveHandle.hpp \
	$$PWD/puppy/PrimitiveInline.hpp \
	$$PWD/puppy/Puppy.hpp \
	$$PWD/puppy/Context.hpp \
	$$PWD/puppy/Randomizer.hpp \
	$$PWD/puppy/Token.hpp \
	$$PWD/puppy/Tree.hpp \
	$$PWD/AccRegPrimits.h \
    $$PWD/featureAllOthers.h
