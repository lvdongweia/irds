#include "rm_audio_system.h"

#include "RDSLog.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>

#ifdef ROBOT_PLATFORM_RK3066
#define MIC_CTL_NODE_PATH "/sys/devices/platform/rk30_i2c.0/i2c-0/0-001a/mic_ctl"
#define MIC_SELECT_NODE_PATH ""
#elif defined ROBOT_PLATFORM_RK3288
#define MIC_CTL_NODE_PATH "/sys/devices/ff660000.i2c/i2c-2/2-001c/mic_ctl"
#define MIC2_CTL_NODE_PATH "sys/devices/ff660000.i2c/i2c-2/2-001c/mic2_ctl"
#define MIC_SELECT_NODE_PATH "/sys/devices/ff660000.i2c/i2c-2/2-001c/h2w_ctl"
#elif defined ROBOT_PLATFORM_BAYTRAIL
#define MIC_CTL_NODE_PATH "/sys/devices/platform/80860F41:01/i2c-2/2-001c/mic_ctl"
#define MIC_SELECT_NODE_PATH ""
#else
#define MIC_CTL_NODE_PATH "/sys/devices/platform/rk30_i2c.0/i2c-0/0-001a/mic_ctl"
#define MIC_SELECT_NODE_PATH ""
#define MIC2_CTL_NODE_PATH ""
#endif

// write the follow values to h2w_ctl node
enum MicTypeHw
{
    HW_MIC_ARRAY = 0,
    HW_MIC_SINGLE = 1,

};

struct MicTypeValueMap
{
    int app_type;
    int hw_type;
    const char *node_path;
}gMicValuesMap[] = {
    {RmAudioSystem::MIC_SINGLE, HW_MIC_SINGLE, MIC2_CTL_NODE_PATH},
    {RmAudioSystem::MIC_ARRAY, HW_MIC_ARRAY, MIC_CTL_NODE_PATH},
    {-1, -1}
};

//helper function
static int getAppMicType(int hwType) {
    int i = 0;
    while (gMicValuesMap[i].hw_type >= 0)
    {
        if (gMicValuesMap[i].hw_type == hwType)
            return gMicValuesMap[i].app_type;

        ++i;
    }

    return -1;
}

static int getHwMicType(int appType) {
    int i = 0;
    while (gMicValuesMap[i].app_type >= 0)
    {
        if (gMicValuesMap[i].app_type == appType)
            return gMicValuesMap[i].hw_type;

        ++i;
    }

    return -1;
}

static const char *getMicCtlNodePath(int appType) {
    int i = 0;
    while (gMicValuesMap[i].app_type >= 0)
    {
        if (gMicValuesMap[i].app_type == appType)
            return gMicValuesMap[i].node_path;

        ++i;
    }

    return NULL;
}

RmAudioSystem *RmAudioSystem::mAudioSystem = NULL;

RmAudioSystem* RmAudioSystem::instance()
{
    if (mAudioSystem == NULL)
        mAudioSystem = new RmAudioSystem();
    return mAudioSystem;
}

void RmAudioSystem::destroy()
{
    if (mAudioSystem != NULL)
    {
        delete mAudioSystem;
        mAudioSystem = NULL;
    }
}

RmAudioSystem::RmAudioSystem()
{
    mNodeFd = -1;
    mMicNode = MIC_NO;

    mMicType = -1;

    //openMicCtlNode();
}

RmAudioSystem::~RmAudioSystem()
{
    closeMicNode();
}

int RmAudioSystem::openMicCtlNode()
{
    mMicNode = MIC_CTL;

    if (mMicType < 0)
    {
        LOGE("###", "open mic control node: mic type error");
        return -1;
    }

    const char* node_path = getMicCtlNodePath(mMicType);
    if (node_path == NULL)
    {
        LOGE("###", "open mic control node: mic node path is null, mic type: %d", mMicType);
        return -1;
    }

    return openMicNode(node_path);
}

int RmAudioSystem::openMicSelectNode()
{
    mMicNode = MIC_SELECT;
    return openMicNode(MIC_SELECT_NODE_PATH);
}

int RmAudioSystem::openMicNode(const char* path)
{
    if (!path || strlen(path) == 0) return -1;

    mNodeFd = open(path, O_RDWR);
    if (mNodeFd <= 0)
    {
        LOGE("###", "open mic node (%s) failed, %s\n", path, strerror(errno));
        return -1;
    }

    return 0;
}

int RmAudioSystem::closeMicNode()
{
    mMicNode = MIC_NO;

    if (mNodeFd > 0)
    {
        close(mNodeFd);
        mNodeFd = -1;
        return 0;
    }

    return -1;
}

bool RmAudioSystem::checkMicNode(int node)
{
    if (mNodeFd > 0) return true;

    if (node == MIC_CTL)
        return (openMicCtlNode() == 0 ? true : false);
    else if (node == MIC_SELECT)
        return (openMicSelectNode() == 0 ? true : false);

    return false;
}

int RmAudioSystem::writeMicPara(const char *value, size_t len, int node)
{
    if (!checkMicNode(node)) return -1;

    int ret = write(mNodeFd, value, len);
    if (ret < 0)
    {
        LOGE("###", "write mic para error, %s\n", strerror(errno));
    }

    closeMicNode();

    return (ret > 0 ? 0 : -1);
}

int RmAudioSystem::readMicPara(int &value, int node)
{
    if (!checkMicNode(node)) return -1;

    char mode[2] = {0};
    int ret = read(mNodeFd, mode, 1);
    if (ret < 0)
    {
        LOGE("###", "read mic para error, %s\n", strerror(errno));
        return -1;
    }

    if (!isdigit(mode[0]))
    {
        LOGE("###", "mic para data error: %s", mode);
        return -1;
    }

    value = atoi(mode);

    closeMicNode();

    return 0;
}

//value: 0~8
int RmAudioSystem::setMicMode(int value)
{
    int tmp = value > 8 ? 8 : value;
    tmp = value < 0 ? 0 : value;

    char tmp_str[2] = {0};
    snprintf(tmp_str, 2, "%d", tmp);

    if (!checkMicType())
    {
        LOGE("###", "setMicMode: check mic type fault");
        return -1;
    }

    android::Mutex::Autolock lock(mLock);
    return writeMicPara(tmp_str, 1, MIC_CTL);
}

int RmAudioSystem::getMicMode(int &value)
{
    if (!checkMicType())
    {
        LOGE("###", "getMicMode: check mic type fault");
        return -1;
    }

    android::Mutex::Autolock lock(mLock);

    int tmp_value = 0;
    int ret = readMicPara(tmp_value, MIC_CTL);

    if (ret == 0)
    {
        value = tmp_value > 8 ? 8 : tmp_value;
        value = tmp_value < 0 ? 0 : tmp_value;
    }

    return ret;
}

int RmAudioSystem::setMicType(int type)
{
    if (type != MIC_SINGLE && type != MIC_ARRAY) return -1;

    char tmp_str[2] = {0};
    snprintf(tmp_str, 2, "%d", getHwMicType(type));

    android::Mutex::Autolock lock(mLock);
    int ret = writeMicPara(tmp_str, 1, MIC_SELECT);
    if (ret == 0) mMicType = type;

    return ret;
}

int RmAudioSystem::getMicType(int &type)
{
    android::Mutex::Autolock lock(mLock);

    int tmp_type = 0;
    int ret = readMicPara(tmp_type, MIC_SELECT);
    if (ret == 0)
    {
        if (tmp_type != HW_MIC_SINGLE && tmp_type != HW_MIC_ARRAY)
        {
            LOGE("###", "get mic type error: %d type", tmp_type);
            ret = -1;
            mMicType = -1; //error, restore init state
        }
        else
        {
            type = getAppMicType(tmp_type);
            mMicType = type; //update, keep in sync with hardware node
        }
    }

    return ret;
}

bool RmAudioSystem::checkMicType() {
    int tmp_type = -1;
    {
        android::Mutex::Autolock lock(mLock);
        tmp_type = mMicType;
    }

    if (tmp_type < 0)
        return (getMicType(tmp_type) == 0 ? true : false);

    return true;
}

/*****************************************************************************
 ************************ jni interface **************************************
 *****************************************************************************/
#define RM_AUDIO_SYSTEM_CLASS "android/robot/hw/RobotMic"

static int android_RobotMic_native_setMicMode(JNIEnv *env, jobject thiz, jint value)
{
    return RmAudioSystem::instance()->setMicMode(value);
}

static int android_RobotMic_native_getMicMode(JNIEnv *env, jobject thiz)
{
    int value = 0;
    if (RmAudioSystem::instance()->getMicMode(value) < 0)
        value = -1;

    return value;
}

static int android_RobotMic_native_setMicType(JNIEnv *env, jobject thiz, jint type)
{
    return RmAudioSystem::instance()->setMicType(type);
}

static int android_RobotMic_native_getMicType(JNIEnv *env, jobject thiz)
{
    int type = 0;
    if (RmAudioSystem::instance()->getMicType(type) < 0)
        type = -1;

    return type;
}

static JNINativeMethod method_table[] = {
    {"_setMicMode",          "(I)I",                 (void *) android_RobotMic_native_setMicMode},
    {"_getMicMode",          "()I",                  (void *) android_RobotMic_native_getMicMode},
    {"_setMicType",          "(I)I",                 (void *) android_RobotMic_native_setMicType},
    {"_getMicType",          "()I",                  (void *) android_RobotMic_native_getMicType},
};

int unregisterAudioSystem(JNIEnv* env)
{
    RmAudioSystem::destroy();
    return 0;
}
static int registerNativeMethods(JNIEnv* env, JNINativeMethod* gMethods, int numMethods) 
{
    jclass clazz;
    clazz = env->FindClass(RM_AUDIO_SYSTEM_CLASS);
    if (clazz == NULL) {
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
        return JNI_FALSE;
    }
    return JNI_TRUE;
}
jint JNI_OnLoad(JavaVM* vm, void* reserved) 
{
    JNIEnv* env = NULL;
    
    LOGE("###", "HAL_ROBOT_MIC LOAD +++");

    if (vm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK) {
        return JNI_ERR;
    }
    if (!registerNativeMethods(env, method_table, (sizeof(method_table) / sizeof(JNINativeMethod)))) {
        return JNI_ERR;
    }
    
    LOGE("###", "HAL_ROBOT_MIC LOAD ---");
    return JNI_VERSION_1_4;
}

void JNI_OnUnLoad(JavaVM * vm, void * reserved) {

    JNIEnv* env = NULL;
    if (vm->GetEnv((void**)&env, JNI_VERSION_1_4) == JNI_OK) 
    {
        LOGE("###", "HAL_ROBOT_MIC UNLOAD");
        unregisterAudioSystem(env);
    }
}
