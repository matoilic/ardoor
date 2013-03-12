##############################################################################
#  File:      _globals.pro
#  Purpose:   QMake project definitions used for all GL projects.
#  Author:    Marcus Hudritsch
#  Date:      September 2012 (HS12)
#  Copyright: Marcus Hudritsch, Switzerland
#             THIS SOFTWARE IS PROVIDED FOR EDUCATIONAL PURPOSE ONLY AND
#             WITHOUT ANY WARRANTIES WHETHER EXPRESSED OR IMPLIED.
##############################################################################

#################################################
# This pro file should be included in all other
# pro files but not in the CG-External.pro file
#################################################

CONFIG += console
CONFIG -= qt
CONFIG += warn_off

DESTDIR  = ../_bin

CONFIG(debug, debug|release) {
   OBJECTS_DIR = _debug
   LIBS += -L../_external/_debug/ -lCG-External
} else {
   OBJECTS_DIR = _release
   LIBS += -L../_external/_release/ -lCG-External
}

win32 {
    # windows only
    LIBS += -lOpenGL32
    LIBS += -lwinmm
    LIBS += -lgdi32
    LIBS += -luser32
    LIBS += -lkernel32
    DEFINES += GLEW_STATIC
    DEFINES += _GLFW_NO_DLOAD_GDI32
    DEFINES += _GLFW_NO_DLOAD_WINMM
    DEFINES -= UNICODE
    QMAKE_CFLAGS += /openmp
}
macx {
    # mac only
    LIBS += -framework Cocoa
    LIBS += -framework IOKit
    LIBS += -framework OpenGL
    LIBS += -lgomp
    QMAKE_CXXFLAGS += -fopenmp
    QMAKE_CXXFLAGS += -Wno-unused-parameter
    CONFIG -= app_bundle
}
unix:!macx {
    # linux only
    LIBS += -lGL
    LIBS += -lgomp
    LIBS += -lX11
    QMAKE_CXXFLAGS += -fopenmp
    QMAKE_CXXFLAGS += -Wunused-parameter
}

INCLUDEPATH += \
    ../_external\
    ../_external/glew/include\
    ../_external/glfw/include\
    ../_external/glfw/lib\
    ../_external/zlib\
    ../_external/png\
    ../_external/randomc\
    ../_external/jpeg-8\
    ../_globals\
    ../_globals/SL\
    ../_globals/GL\
    ../_globals/math\
    ../_globals/MeshLoader\
    ../_globals/SpacePartitioning\
    include
