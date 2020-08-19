LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := VideoCapture



# Add your application source files here...
LOCAL_SRC_FILES := dll_video_capture.cpp

LOCAL_SRC_FILES += demo.c


include $(BUILD_SHARED_LIBRARY)
