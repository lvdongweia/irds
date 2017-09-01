LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    test_main.cpp

LOCAL_SHARED_LIBRARIES := \
    libirds \
    libirds_client\
    libbinder \
    libcutils \
    liblog \
    libutils \
    libdl \
    libsysinfo

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/../include \
    $(LOCAL_PATH)/../../include \
    $(LOCAL_PATH)/../

LOCAL_CFLAGS := -O2 -Wall -g -Wunused-parameter
LOCAL_MODULE_TAGS := eng optional
LOCAL_MODULE:= trds 

include $(BUILD_EXECUTABLE)
