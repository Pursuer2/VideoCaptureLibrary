LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := VideoCapture


LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(SDL_PATH)/include

# Add your application source files here...
LOCAL_SRC_FILES := dll_video_capture.cpp

LOCAL_SHARED_LIBRARIES := SDL2


include $(BUILD_SHARED_LIBRARY)
