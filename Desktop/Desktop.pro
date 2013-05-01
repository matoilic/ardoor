#-------------------------------------------------
#
# Project created by QtCreator 2013-04-19T00:44:29
#
#-------------------------------------------------

QT       += core gui multimedia multimediawidgets opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Desktop
TEMPLATE = app

include(../_globals.pro)


SOURCES += \
    main.cpp \
    MainWindow.cpp \
    CalibrationDialog.cpp \
    CameraImageProcessor.cpp \
    ImageWidget.cpp \
    QtUtil.cpp \
    test.cpp

HEADERS += \
    MainWindow.h \
    CalibrationDialog.h \
    CameraImageProcessor.h \
    ImageWidget.h \
    QtUtil.h \
    GLRenderer.h

FORMS += \
    MainWindow.ui \
    CalibrationDialog.ui

CONFIG += link_pkgconfig
PKGCONFIG += opencv

LIBS += -lglut -lGLU

# ARDoorCommon Library
INCLUDEPATH += ../Libraries/ARDoorCommon
LIBS += -L$$BUILDPATH/ARDoorCommon -lARDoorCommon
