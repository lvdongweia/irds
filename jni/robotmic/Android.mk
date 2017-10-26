
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# This is the target being built.
LOCAL_MODULE:= librobotmic_jni


# All of the source files that we will compile.
LOCAL_SRC_FILES:= \
    rm_audio_system.cpp


# All of the shared libraries we link against.
LOCAL_SHARED_LIBRARIES := \
    liblog \
    libutils \
    libdl \
    libandroid_runtime \

# No static libraries.
LOCAL_STATIC_LIBRARIES :=

# Also need the JNI headers.
LOCAL_C_INCLUDES += \
    $(JNI_H_INCLUDE)  \
    $(TOPDIR)frameworks/robot/irds/include \

LOCAL_CFLAGS := -O2 -Wall -Wunused-parameter

ifeq ($(TARGET_BOARD_PLATFORM), baytrail)
LOCAL_CFLAGS += -DROBOT_PLATFORM_BAYTRAIL
else ifeq ($(TARGET_BOARD_PLATFORM), rk3066)
LOCAL_CFLAGS += -DROBOT_PLATFORM_RK3066
else ifeq ($(TARGET_BOARD_PLATFORM), rk3288)
LOCAL_CFLAGS += -DROBOT_PLATFORM_RK3288
endif

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

