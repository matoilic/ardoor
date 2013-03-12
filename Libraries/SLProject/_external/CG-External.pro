##############################################################################
#  File:      CG-External.pro
#  Purpose:   QMake project definition file for external libraries
#  Author:    Marcus Hudritsch
#  Date:      September 2012 (HS12)
#  Copyright: Marcus Hudritsch, Switzerland
#             THIS SOFTWARE IS PROVIDED FOR EDUCATIONAL PURPOSE ONLY AND
#             WITHOUT ANY WARRANTIES WHETHER EXPRESSED OR IMPLIED.
##############################################################################

TEMPLATE = lib
TARGET = CG-External
CONFIG += staticlib
CONFIG -= qt
CONFIG += warn_off

CONFIG(debug, debug|release) {
   DESTDIR = _debug
   OBJECTS_DIR = _debug/obj
} else {
   DESTDIR = _release
   OBJECTS_DIR = _release/obj
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
}
macx {
    # mac only
    LIBS += -framework Cocoa
    LIBS += -framework IOKit
    LIBS += -framework OpenGL
    LIBS += -lgomp
    CONFIG -= app_bundle
    QMAKE_CXXFLAGS += -fopenmp
    QMAKE_CXXFLAGS += -Wno-unused-parameter
}
unix:!macx {
    # linux only
    LIBS += -lgl
    LIBS += -lgomp
    LIBS += -lX11
    QMAKE_CXXFLAGS += -fopenmp
    QMAKE_CXXFLAGS += -Wunused-parameter
}


INCLUDEPATH += \
    glew/include\
    glfw/include\
    glfw/lib\
    zlib\
    png\
    jpeg-8\
    randomc

HEADERS += \
    jpeg-8/jversion.h \
    jpeg-8/jpeglib.h \
    jpeg-8/jpegint.h \
    jpeg-8/jmorecfg.h \
    jpeg-8/jmemsys.h \
    jpeg-8/jinclude.h \
    jpeg-8/jerror.h \
    jpeg-8/jdct.h \
    jpeg-8/jconfig.h \
    zlib/zutil.h \
    zlib/zlib.h \
    zlib/zconf.h \
    zlib/trees.h \
    zlib/inftrees.h \
    zlib/inflate.h \
    zlib/inffixed.h \
    zlib/inffast.h \
    zlib/gzguts.h \
    zlib/deflate.h \
    zlib/crypt.h \
    zlib/crc32.h \
    nvwa/debug_new.h \
    glew/include/GL/glew.h \
    glfw/lib/internal.h \
    glfw/lib/win32/platform.h \
    glfw/lib/cocoa/platform.h \
    glfw/lib/x11/platform.h \
    jpeg-8/jversion.h \
    jpeg-8/jpeglib.h \
    jpeg-8/jpegint.h \
    jpeg-8/jmorecfg.h \
    jpeg-8/jmemsys.h \
    jpeg-8/jinclude.h \
    jpeg-8/jerror.h \
    jpeg-8/jdct.h \
    jpeg-8/jconfig.h \
    randomc/randomc.h \
    randomc/random.h

SOURCES += \
    jpeg-8/jutils.c \
    jpeg-8/jquant2.c \
    jpeg-8/jquant1.c \
    jpeg-8/jmemnobs.c \
    jpeg-8/jmemmgr.c \
    jpeg-8/jidctint.c \
    jpeg-8/jidctfst.c \
    jpeg-8/jidctflt.c \
    jpeg-8/jfdctint.c \
    jpeg-8/jfdctfst.c \
    jpeg-8/jfdctflt.c \
    jpeg-8/jerror.c \
    jpeg-8/jdtrans.c \
    jpeg-8/jdsample.c \
    jpeg-8/jdpostct.c \
    jpeg-8/jdmerge.c \
    jpeg-8/jdmaster.c \
    jpeg-8/jdmarker.c \
    jpeg-8/jdmainct.c \
    jpeg-8/jdinput.c \
    jpeg-8/jdhuff.c \
    jpeg-8/jddctmgr.c \
    jpeg-8/jdcolor.c \
    jpeg-8/jdcoefct.c \
    jpeg-8/jdatasrc.c \
    jpeg-8/jdatadst.c \
    jpeg-8/jdarith.c \
    jpeg-8/jdapistd.c \
    jpeg-8/jdapimin.c \
    jpeg-8/jctrans.c \
    jpeg-8/jcsample.c \
    jpeg-8/jcprepct.c \
    jpeg-8/jcparam.c \
    jpeg-8/jcomapi.c \
    jpeg-8/jcmaster.c \
    jpeg-8/jcmarker.c \
    jpeg-8/jcmainct.c \
    jpeg-8/jcinit.c \
    jpeg-8/jchuff.c \
    jpeg-8/jcdctmgr.c \
    jpeg-8/jccolor.c \
    jpeg-8/jccoefct.c \
    jpeg-8/jcarith.c \
    jpeg-8/jcapistd.c \
    jpeg-8/jcapimin.c \
    jpeg-8/jaricom.c \
    png/pngwutil.c \
    png/pngwtran.c \
    png/pngwrite.c \
    png/pngwio.c \
    png/pngtrans.c \
    png/pngset.c \
    png/pngrutil.c \
    png/pngrtran.c \
    png/pngrio.c \
    png/pngread.c \
    png/pngpread.c \
    png/pngmem.c \
    png/pngget.c \
    png/pngerror.c \
    png/png.c \
    zlib/zutil.c \
    zlib/uncompr.c \
    zlib/trees.c \
    zlib/inftrees.c \
    zlib/inflate.c \
    zlib/inffast.c \
    zlib/inffas8664.c \
    zlib/infback.c \
    zlib/gzwrite.c \
    zlib/gzread.c \
    zlib/gzlib.c \
    zlib/gzclose.c \
    zlib/deflate.c \
    zlib/crc32.c \
    zlib/compress.c \
    zlib/adler32.c \
    nvwa/debug_new.cpp \
    glew/src/glew.c \
    glfw/lib/window.c \
    glfw/lib/time.c \
    glfw/lib/thread.c \
    glfw/lib/tga.c \
    glfw/lib/stream.c \
    glfw/lib/joystick.c \
    glfw/lib/input.c \
    glfw/lib/init.c \
    glfw/lib/image.c \
    glfw/lib/glext.c \
    glfw/lib/fullscreen.c \
    glfw/lib/enable.c \
    randomc/sobol.cpp \
    randomc/ranrotw.cpp \
    randomc/ranrotb.cpp \
    randomc/random.cpp \
    randomc/rancombi.cpp \
    randomc/mother.cpp \
    randomc/mersenne.cpp


win32 { #Windows only
INCLUDEPATH += "glfw/lib/win32"
HEADERS += \
    glew/include/GL/wglew.h
SOURCES += \
    glfw/lib/win32/win32_window.c \
    glfw/lib/win32/win32_time.c \
    glfw/lib/win32/win32_thread.c \
    glfw/lib/win32/win32_joystick.c \
    glfw/lib/win32/win32_init.c \
    glfw/lib/win32/win32_glext.c \
    glfw/lib/win32/win32_fullscreen.c \
    glfw/lib/win32/win32_enable.c \
    glfw/lib/win32/win32_dllmain.c
}
unix:!macx { #Linux only
INCLUDEPATH += "glfw/lib/x11"
HEADERS += \
    glew/include/GL/glxew.h
SOURCES += \
    glfw/lib/x11/x11_window.c \
    glfw/lib/x11/x11_time.c \
    glfw/lib/x11/x11_thread.c \
    glfw/lib/x11/x11_keysym2unicode.c \
    glfw/lib/x11/x11_joystick.c \
    glfw/lib/x11/x11_init.c \
    glfw/lib/x11/x11_glext.c \
    glfw/lib/x11/x11_fullscreen.c \
    glfw/lib/x11/x11_enable.c
}
macx { #Mac OSX only
INCLUDEPATH += "glfw/lib/cocoa"
SOURCES += \
    glfw/lib/cocoa/cocoa_thread.c

OBJECTIVE_SOURCES += \
    glfw/lib/cocoa/cocoa_time.m \
    glfw/lib/cocoa/cocoa_joystick.m \
    glfw/lib/cocoa/cocoa_init.m \
    glfw/lib/cocoa/cocoa_glext.m \
    glfw/lib/cocoa/cocoa_fullscreen.m \
    glfw/lib/cocoa/cocoa_enable.m \
    glfw/lib/cocoa/cocoa_window.m
}
