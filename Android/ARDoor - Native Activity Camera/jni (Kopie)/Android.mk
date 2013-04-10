LOCAL_PATH := $(call my-dir)
SELF_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
OPENCV_SDK_DIR := $(SELF_DIR)/../../OpenCV-2.4.4-android-sdk

include $(CLEAR_VARS)
OPENCV_CAMERA_MODULES:=on
OPENCV_INSTALL_MODULES:=on
OPENCV_LIB_TYPE:=SHARED
include $(OPENCV_SDK_DIR)/sdk/native/jni/OpenCV.mk

include $(CLEAR_VARS)
LOCAL_MODULE    := CameraTest
LOCAL_SRC_FILES := AppEngine.cpp ConcreteApp.cpp main.cpp
LOCAL_LDLIBS    += -llog -landroid -lEGL -lGLESv1_CM
LOCAL_SHARED_LIBRARIES += opencv_java
LOCAL_STATIC_LIBRARIES += android_native_app_glue
LOCAL_C_INCLUDES += $(OPENCV_SDK_DIR)/sdk/native/jni/include
include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
