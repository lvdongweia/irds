LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    MotionPlayer.cpp \
    MotionService.cpp \
    SystemController.cpp

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libutils \
    libbinder \
    libirds


LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/../include \

LOCAL_MODULE_TAGS := eng optional
LOCAL_CFLAGS := -O2 -Wall -g
LOCAL_MODULE:= libirds_client

include $(BUILD_SHARED_LIBRARY)
