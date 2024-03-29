INCLUDEPATH  += $$PWD \
	$$PWD/puppy/include

SOURCES += \
	$$PWD/puppy/src/Primitive.cpp \
	$$PWD/puppy/src/Puppy.cpp \
	$$PWD/puppy/src/Tree.cpp

HEADERS  += \
	$$PWD/puppy/include/puppy/Primitive.hpp \
	$$PWD/puppy/include/puppy/PrimitiveHandle.hpp \
	$$PWD/puppy/include/puppy/PrimitiveInline.hpp \
	$$PWD/puppy/include/puppy/Puppy.hpp \
	$$PWD/puppy/include/puppy/Context.hpp \
	$$PWD/puppy/include/puppy/Randomizer.hpp \
	$$PWD/puppy/include/puppy/TokenT.hpp \
	$$PWD/puppy/include/puppy/Tree.hpp \
	$$PWD/AccRegPrimits.h \
    $$PWD/featureAllOthers.h \
    $$PWD/featureStatDistrib.h \
	$$PWD/featurePeriodicAmount.h \
	$$PWD/featureSalary.h \
	$$PWD/featureOutlier.h \
	$$PWD/featurePriceWindow.h \
	$$PWD/featureLabelDistrib.h \
	$$PWD/featureCategory.h \
	$$PWD/featureCrossCurrent.h \

