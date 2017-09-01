LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    IMotionService.cpp \
    IMotionPlayer.cpp \
    ISystemController.cpp \
    IRawCommandService.cpp \
    ITopic.cpp \
    IRawCommandTopic.cpp \
    ICompletionListener.cpp \
    IMessageListener.cpp \
    IServiceListener.cpp \
    IDeathNotifier.cpp \
    IRDService.cpp \
    IResultListener.cpp


LOCAL_SHARED_LIBRARIES := \
    libandroid_runtime \
    libcutils \
    libutils \
    libbinder \


LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/../include \

LOCAL_MODULE_TAGS := eng optional
LOCAL_CFLAGS := -O2 -Wall -g
LOCAL_MODULE:= libirds

include $(BUILD_SHARED_LIBRARY)
