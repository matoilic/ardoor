LOCAL_PATH := $(call my-dir)
SELF_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
OPENCV_SDK_DIR := $(SELF_DIR)/../../OpenCV-2.4.4-android-sdk

include $(CLEAR_VARS)

OPENCV_CAMERA_MODULES:=on
OPENCV_INSTALL_MODULES:=on
OPENCV_LIB_TYPE:=SHARED
include $(OPENCV_SDK_DIR)/sdk/native/jni/OpenCV.mk

LOCAL_SRC_FILES  := ARDoorLib_jni.cpp ARDoorLib.cpp
LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_LDLIBS     += -llog -ldl -landroid -lEGL -lGLESv2

LOCAL_MODULE     := ardoor

include $(BUILD_SHARED_LIBRARY)