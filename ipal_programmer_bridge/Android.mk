LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    $(patsubst $(LOCAL_PATH)/%,%,$(wildcard $(LOCAL_PATH)/*.cpp \
    $(LOCAL_PATH)/*/*.cpp $(LOCAL_PATH)/*/*/*.cpp))

LOCAL_SHARED_LIBRARIES := \
    libirds \
    libirds_client\
    libbinder \
    libcutils \
    liblog \
    libutils \
    libdl \
    libsysinfo \
    libmedia


LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/../include \
    $(LOCAL_PATH)/../../include \
    $(LOCAL_PATH)/../ \
    $(LOCAL_PATH)/../../../av/include

LOCAL_CFLAGS := -O2 -Wall -g -Wunused-parameter -fpermissive
LOCAL_MODULE_TAGS := eng
LOCAL_MODULE:= libiPalProgBridge

include $(BUILD_SHARED_LIBRARY)
