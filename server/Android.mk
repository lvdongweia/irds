LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    service/RawCommandService.cpp \
    service/SystemController.cpp \
    service/MotionService.cpp \
    service/MotionPlayer.cpp \
    service/AdbForwardService.cpp \
    base/ArmFile.cpp\
    base/DeviceTable.cpp \
    base/ServiceObject.cpp \
    base/CommandObject.cpp \
    base/ControllerObject.cpp \
    CANSession.cpp \
    RDSTask.cpp \
    RDService.cpp \
    RDSMain.cpp

LOCAL_SHARED_LIBRARIES := \
    librm_can \
    libirds \
    libbinder \
    libcutils \
    liblog \
    libutils \
    libdl \
    libsysinfo

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/../include \
    $(LOCAL_PATH)/../../include \
    $(LOCAL_PATH)/base\
    $(TOPDIR)external/robot-canbus/include \
    $(TOPDIR)external/robot-control/include/libsysinfo

LOCAL_CFLAGS := -O2 -Wall -g -Wunused-parameter
LOCAL_MODULE_TAGS := eng optional
LOCAL_MODULE:= irds 

include $(BUILD_EXECUTABLE)
