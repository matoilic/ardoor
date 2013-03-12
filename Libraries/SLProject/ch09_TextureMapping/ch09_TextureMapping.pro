##############################################################################
#  File:      ch09_TextureMapping.pro
#  Purpose:   QMake project definition file for the Hello Cube demo w. OpenGL
#  Author:    Marcus Hudritsch
#  Date:      September 2012 (HS12)
#  Copyright: Marcus Hudritsch, Switzerland
#             THIS SOFTWARE IS PROVIDED FOR EDUCATIONAL PURPOSE ONLY AND
#             WITHOUT ANY WARRANTIES WHETHER EXPRESSED OR IMPLIED.
##############################################################################

TEMPLATE = app
TARGET = ch00_TextureMapping

include(../_globals/_globals.pro)

SOURCES += \
    ../_globals/GL/glUtils.cpp \
    ../_globals/SL/SLImage.cpp \
    TextureMapping_Sacher.cpp

OTHER_FILES += \
    ../_globals/oglsl/ADSTex.vert \
    ../_globals/oglsl/ADSTex.frag \
    ../_globals/oglsl/ADSTex_Weissen.vert \
    ../_globals/oglsl/ADSTex_Weissen.frag \
    ../_globals/oglsl/ADSTex_Loesung.frag \
    ../_globals/oglsl/ADSTex_Loesung.vert \
    ../_globals/oglsl/ADSTex_Misteli.vert \
    ../_globals/oglsl/ADSTex_Misteli.frag \
    ../_globals/oglsl/ADSTex_Grenacher.frag \
    ../_globals/oglsl/ADSTex_Grenacher.vert \
    ../_globals/oglsl/ADSTex_Vischi.vert \
    ../_globals/oglsl/ADSTex_Vischi.frag \
    ../_globals/oglsl/ADSTex_Arnosti.vert \
    ../_globals/oglsl/ADSTex_Arnosti.frag \
    ../_globals/oglsl/ADSTex_Bolzern.vert \
    ../_globals/oglsl/ADSTex_Bolzern.frag \
    ../_globals/oglsl/ADSTex_Giedemann.vert
