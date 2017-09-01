
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# This is the target being built.
LOCAL_MODULE:= librobotplayer_jni


# All of the source files that we will compile.
LOCAL_SRC_FILES:= \
	android_RobotPlayer.cpp


# All of the shared libraries we link against.
LOCAL_SHARED_LIBRARIES := \
	libirds\
    liblog \
    libutils \
    libdl \
    libandroid_runtime \
	libbinder\
    libsysinfo

# No static libraries.
LOCAL_STATIC_LIBRARIES :=

# Also need the JNI headers.
LOCAL_C_INCLUDES += \
	$(JNI_H_INCLUDE)  \
    $(TOPDIR)frameworks/robot/irds/include

LOCAL_CFLAGS := -O2 -Wall -Wunused-parameter

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

