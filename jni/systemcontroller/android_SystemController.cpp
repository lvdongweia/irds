/*
 */
#include <utils/Log.h>
#include <jni.h>
#include <unistd.h>
#include <fcntl.h>
#include <utils/threads.h>
#include <android_runtime/AndroidRuntime.h>
#include <binder/Parcel.h>
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <utils/Mutex.h>

#include "utils/String8.h"
#include "ISystemController.h"
#include "ICompletionListener.h"
#include "IResultListener.h"
#include "IMessageListener.h"
#include "IDeathNotifier.h"
#include "IRDService.h"
#include "RDSLog.h"
#include "RobotSysInfo.h"

using namespace android;

#define SYSCONTRLCLASS "android/robot/hw/RobotSystem"
#define ROBOTSUBSYSSTATE "android/robot/scheduler/RobotSubSysState"

#define SCLISTENERCLASS SYSCONTRLCLASS "$Listener"
#define SCONRESULT SYSCONTRLCLASS "$OnResult"
#define SCSTATUSRESULT SYSCONTRLCLASS "$StatusResult"

struct fields_t {
    jfieldID    context;
    jmethodID   onMessage;
};
jclass clazz_subsys_state;

static jmethodID onQueryResult;
static jmethodID onStatusResult;
static jmethodID constructor_subsys_state;

static jfieldID field_subsys_is_enable;
static jfieldID field_subsys_is_poweron;
static jfieldID field_subsys_is_active;
static jfieldID field_subsys_is_initalized;
static jfieldID field_subsys_is_debug_en;
static jfieldID field_subsys_state;
static jfieldID field_subsys_sub_state;
static jfieldID field_subsys_mode;

static android::RobotSysInfo *g_sys_info_ptr = NULL;

static Mutex sLock;
static fields_t fields;
static void * g_cache = NULL;
static int g_cache_size = 0;

// ----------------------------------------------------------------------------
// ref-counted object for callbacks

class JNIOnQueryResult : public BnCompletionListener
{
    public:
        JNIOnQueryResult(JNIEnv* env, jobject obj)
        {
            mCBObject  = 0;
            set(env, obj);
        }
        virtual ~JNIOnQueryResult()
        {
            clear();
        }
        void set(JNIEnv * env, jobject obj)
        {
            clear();
            isFree = false;
            mCBObject  = env->NewGlobalRef(obj);
        }
        void clear()
        {
            JNIEnv *env = AndroidRuntime::getJNIEnv();
            if (mCBObject != 0) env->DeleteGlobalRef(mCBObject);
            mCBObject = 0;
            isFree = true;
        }
        virtual int onCompletion(int session_id, int result, int errorcode)
        {
            if (onQueryResult == 0 || mCBObject == 0)
            {
                return 0;
            }
            JNIEnv *env = AndroidRuntime::getJNIEnv();
            env->CallVoidMethod(mCBObject, onQueryResult, session_id, result, errorcode);
            clear();
            return 0;
        }
    public:
        bool isFree;
    private:
        jobject mCBObject;
};

class JNIOnStatusResult : public BnResultListener
{
    public:
        JNIOnStatusResult(JNIEnv* env, jobject obj)
        {
            mCBObject  = 0;
            set(env, obj);
        }
        virtual ~JNIOnStatusResult()
        {
            clear();
        }
        void set(JNIEnv * env, jobject obj)
        {
            clear();
            mCBObject  = env->NewGlobalRef(obj);
        }
        void clear()
        {
            JNIEnv *env = AndroidRuntime::getJNIEnv();
            if (mCBObject != 0) env->DeleteGlobalRef(mCBObject);
            mCBObject = 0;
        }
        virtual int onCompletion(int result, int errorcode)
        {
            if (onStatusResult == 0 || mCBObject == 0)
            {
                return 0;
            }
            JNIEnv *env = AndroidRuntime::getJNIEnv();
            env->CallVoidMethod(mCBObject, onStatusResult, result, errorcode);
            clear();
            return 0;
        }
    private:
        jobject mCBObject;
};

class JNISystemControllerListener: public BnMessageListener
{
public:
    JNISystemControllerListener(JNIEnv* env, jobject thiz)
    {
        mListenerObject  = 0;
    }
    virtual ~JNISystemControllerListener()
    {
        // remove global references
        JNIEnv *env = AndroidRuntime::getJNIEnv();
        if (mListenerObject != 0) env->DeleteGlobalRef(mListenerObject);
    }
    virtual void setListenerObject(JNIEnv* env, jobject obj)
    {
        mListenerObject  = env->NewGlobalRef(obj);
    }
    virtual int onMessage(int from, int what, int arg1, int arg2)
    {
        if (fields.onMessage == 0 || mListenerObject == 0)
        {
            return 0;
        }
        JNIEnv *env = AndroidRuntime::getJNIEnv();
        env->CallVoidMethod(mListenerObject, fields.onMessage, from, what, arg1, arg2);   
        return 0;
    }
private:
    jobject mListenerObject;
};


class SystemControllerCtx : public RefBase
{
    private:
        class JNIDeathNotifier : public IDeathNotifier
        {
        public:
            JNIDeathNotifier(SystemControllerCtx * ctx)
            {
                this->ctx = ctx;
            }
            virtual void died()
            {
                if (ctx == NULL)
                {
                    return;
                }
                ctx->systemController = NULL;
                if (ctx->sysCtrlListener.get() != 0) ctx->sysCtrlListener->onMessage(0xFF, 0x8FFFE, 0, 0);
            }
        private:
            SystemControllerCtx * ctx;
        };

   private:
        static const char * token;
        sp<JNIDeathNotifier> rdsDeathNotifier;
    public:
        sp<JNISystemControllerListener> sysCtrlListener;
        sp<ISystemController> systemController;
    public:
        SystemControllerCtx(JNIEnv* env, jobject thiz)
        {
            systemController = NULL;
            rdsDeathNotifier = new JNIDeathNotifier(this);
            sysCtrlListener = new JNISystemControllerListener(env, thiz);
        }
        virtual ~SystemControllerCtx()
        {
            systemController = NULL;
            sysCtrlListener = NULL;
            rdsDeathNotifier = NULL;
        }
        virtual void setListener(JNIEnv* env, jobject obj)
        {
            sysCtrlListener->setListenerObject(env, obj);
        }
        int prepare(JNIEnv* env, jobject thiz)
        {
            const sp<IRDService>& service = IDeathNotifier::GetRDService(); //tiemout 1 sec;

            if (service.get() == 0)
            {
                return -1;
            }
            if (systemController.get() == 0)
            {
                systemController = service->getSystemController(token, strlen(token));
            }
            if (systemController.get() != 0)
            {
                systemController->setMessageListener(sysCtrlListener);
            }
            return 0;
        }
        bool isReady()
        {
            return (systemController.get() != 0);
        }
};

const char * SystemControllerCtx::token = "0D3E1B4E-C3C4-4CF2-9435-C10C75B9899E";

#define GetSystemControllerCtx(name, env, thiz)    \
                                    sp<SystemControllerCtx> name; \
                                    do \
                                    { \
                                        name = getSystemControllerCtx(env, thiz); \
                                        if (name.get() == 0) \
                                        { \
                                            LOGE("###", " Can't find SystemControllerCtx"); \
                                            return -1; \
                                        } \
                                    }\
                                    while(0)


#define GetSystemController(name, env, thiz)    \
                                    sp<ISystemController> name = 0; \
                                    sp<SystemControllerCtx> ctx; \
                                    do \
                                    { \
                                        ctx = getSystemControllerCtx(env, thiz); \
                                        if (ctx.get() == 0) \
                                        { \
                                            LOGE("###", " Can't find SystemControllerCtx"); \
                                            return -1; \
                                        } \
                                        if (ctx->systemController.get() == 0)\
                                        { \
                                           ctx->prepare(env, thiz);\
                                        }\
                                        if (ctx->systemController.get() == 0) \
                                        {\
                                            LOGE("###", " Can't find SystemController"); \
                                            return -2;\
                                        }\
                                        name = ctx->systemController;\
                                    }\
                                    while(0)

static sp<SystemControllerCtx> getSystemControllerCtx(JNIEnv* env, jobject thiz)
{
    Mutex::Autolock l(sLock);
    SystemControllerCtx* const p = (SystemControllerCtx*)env->GetIntField(thiz, fields.context);
    return sp<SystemControllerCtx>(p);
}

static sp<SystemControllerCtx> setSystemControllerCtx(JNIEnv* env, jobject thiz, const sp<SystemControllerCtx>& service)
{
    Mutex::Autolock l(sLock);
    sp<SystemControllerCtx> old = (SystemControllerCtx*)env->GetIntField(thiz, fields.context);
    if (service.get()) {
        service->incStrong((void*)setSystemControllerCtx);
    }
    if (old != 0) {
        old->decStrong((void*)setSystemControllerCtx);
    }
    env->SetIntField(thiz, fields.context, (int)service.get());
    return old;
}

static void android_SystemController_native_init(JNIEnv *env)
{
    jclass clazz;

    clazz = env->FindClass(SCLISTENERCLASS);
    if (clazz == NULL) {
        return;
    }
    fields.onMessage = env->GetMethodID(clazz, "onMessage", "(IIII)V");
    clazz = env->FindClass(SYSCONTRLCLASS);
    if (clazz == NULL) {
        return;
    }
    fields.context = env->GetFieldID(clazz, "mNativeContext", "I");
    clazz = env->FindClass(SCONRESULT);
    if (clazz == NULL) {
        return;
    }
    onQueryResult = env->GetMethodID(clazz, "onCompleted", "(III)V");
    clazz = env->FindClass(SCSTATUSRESULT);
    if (clazz == NULL) {
        return;
    }
    onStatusResult = env->GetMethodID(clazz, "onCompleted", "(II)V");
}

static int android_SystemController_native_setup(JNIEnv *env, jobject thiz)
{
    sp<SystemControllerCtx> ms = new SystemControllerCtx(env, thiz);
    setSystemControllerCtx(env, thiz, ms);
    
    LOGE("###", "SystemController Setup");
    return 0;
}

static int android_SystemController_native_finalize(JNIEnv *env, jobject thiz)
{
    Mutex::Autolock l(sLock);
    sp<SystemControllerCtx> service = (SystemControllerCtx*)env->GetIntField(thiz, fields.context);
    
    if (service != 0) {
        service->decStrong((void*)setSystemControllerCtx);
    }
    env->SetIntField(thiz, fields.context, 0);
    service = NULL;

    LOGE("###", "SystemController Finalizes");
    return 0;
}
static int android_SystemController_native_setListener(JNIEnv *env, jobject thiz, jobject obj)
{
    GetSystemControllerCtx(ctx, env, thiz);
    ctx->setListener(env,obj);
    ctx->prepare(env, thiz);
    return 0;
}

static int android_SystemController_native_send(JNIEnv *env, jobject thiz, int to, int cmd, int arg1, int arg2, jobject obj)
{
    GetSystemController(systemController, env, thiz);
    if (systemController.get() == 0)
    {
        LOGE("###", "get jni systemcontroller send error");
        return -1;
    }
    if(obj == NULL)
    {
        LOGE("###", "call jni systemcontroller send obj = null");
        return systemController->send(to, cmd, arg1, arg2, NULL);
    }

    LOGE("###", "call jni systemcontroller send obj");
    sp<JNIOnQueryResult> cb = new JNIOnQueryResult(env, obj);
    return systemController->send(to, cmd, arg1, arg2, cb);
}
static int android_SystemController_native_setEnable(JNIEnv *env, jobject thiz, int deviceType, int deviceId, int enable, jobject obj)
{
    GetSystemController(systemController, env, thiz);
    if (systemController.get() == 0)
    {
        LOGE("###", "get jni systemcontroller setEnable error");
        return -1;
    }
    if(obj == NULL)
    {
        LOGE("###", "call jni systemcontroller send obj = null");
        return systemController->setEnable(deviceType, deviceType, enable, NULL);
    }
    LOGE("###", "call jni systemcontroller setEnable");
    sp<JNIOnQueryResult> cb = new JNIOnQueryResult(env, obj);
    return systemController->setEnable(deviceType, deviceType, enable, cb);
}
static int android_SystemController_native_isEnable(JNIEnv *env, jobject thiz, int deviceType, int deviceId, jobject obj)
{
    GetSystemController(systemController, env, thiz);
    if (systemController.get() == 0)
    {
        LOGE("###", "get jni systemcontroller isEnable error");
        return -1;
    }
    if(obj == NULL)
    {
        LOGE("###", "call jni systemcontroller isEnable obj = null");
        return systemController->isEnable(deviceType, deviceId, NULL);
    }
    LOGE("###", "call jni systemcontroller isEnable");
    sp<JNIOnQueryResult> cb = new JNIOnQueryResult(env, obj);
    return systemController->isEnable(deviceType, deviceId, cb);
}
static int android_SystemController_native_setting(JNIEnv *env, jobject thiz, int id, int cmd, int arg1, int arg2, jobject obj)
{
    GetSystemController(systemController, env, thiz);
    if (systemController.get() == 0)
    {
        LOGE("###", "get jni systemcontroller setting error");
        return -1;
    }
    if(obj == NULL)
    {
        LOGE("###", "call jni systemcontroller setting obj = null");
        return systemController->setting(id, cmd, arg1, arg2, NULL);
    }
    LOGE("###", "call jni systemcontroller setting");
    sp<JNIOnQueryResult> cb = new JNIOnQueryResult(env, obj);
    return systemController->setting(id, cmd, arg1, arg2, cb);
}
static jstring android_SystemController_native_getSubSysVersion(JNIEnv *env, jobject thiz, jint sub_sys_id)
{
    char version[128] = {0};

    int ret = g_sys_info_ptr->GetSubSysVersion(sub_sys_id, version, 127);
    if (ret < 0) return NULL;

    return env->NewStringUTF(version);
}
static jstring android_SystemController_native_getRobotSN(JNIEnv *env, jobject thiz)
{
    char sn_format[128] = {0};
    bool is_valid = false;

    int ret = g_sys_info_ptr->GetRobotSN(is_valid, sn_format, 120);
    if (ret < 0) return NULL;

    const char *valid_str = is_valid ? ";true" : ";false";
    size_t len = strlen(sn_format);
    strcpy(sn_format + len, valid_str);

    return env->NewStringUTF(sn_format);
}
static jboolean android_SystemController_native_isRobotMoving(JNIEnv *env, jobject thiz)
{
    bool is_moving = true;
    g_sys_info_ptr->isRobotMoving(is_moving);
    return is_moving;
}
static jobject android_SystemController_native_getSubSysState(JNIEnv *env, jobject thiz, jint sub_sys_id)
{
    struct android::RobotSysInfo::RobotSubSysState state;

    int ret = g_sys_info_ptr->GetSubSysState(sub_sys_id, state);
    if (ret < 0) return NULL;

    jobject subSysState = env->NewObject(clazz_subsys_state, constructor_subsys_state);
    if (subSysState == NULL) {
        LOGE("###", "Could not create a %s object", ROBOTSUBSYSSTATE);
        return NULL;
    }

    env->SetBooleanField(subSysState, field_subsys_is_enable, state.is_enable);
    env->SetBooleanField(subSysState, field_subsys_is_poweron, state.is_power_on);
    env->SetBooleanField(subSysState, field_subsys_is_active, state.is_active);
    env->SetBooleanField(subSysState, field_subsys_is_initalized, state.is_initalized);
    env->SetBooleanField(subSysState, field_subsys_is_debug_en, state.is_debug_en);
    env->SetIntField(subSysState, field_subsys_state, state.state);
    env->SetIntField(subSysState, field_subsys_sub_state, state.sub_state);
    env->SetIntField(subSysState, field_subsys_mode, state.mode);

    return subSysState;
}
static jboolean android_SystemController_native_getRobotSex(JNIEnv *env, jobject thiz)
{
    bool is_boy = true;
    g_sys_info_ptr->GetRobotSex(is_boy);
    return is_boy;
}
static jint android_SystemController_native_setRobotSex(JNIEnv *env, jobject thiz, jboolean is_box)
{
    return g_sys_info_ptr->SetRobotSex(is_box);
}
static jintArray android_SystemController_native_getRobotTime(JNIEnv *env, jobject thiz)
{
    jintArray jTimeArray = env->NewIntArray(7);
    jint *nTimeArray = env->GetIntArrayElements(jTimeArray, NULL);

    int year = 0, month = 0, day = 0, weekday = 0, hour = 0, minute = 0, second = 0;
    g_sys_info_ptr->GetRobotTime(year, month, day, weekday, hour, minute, second);

    nTimeArray[0] = year;
    nTimeArray[1] = month;
    nTimeArray[2] = day;
    nTimeArray[3] = weekday;
    nTimeArray[4] = hour;
    nTimeArray[5] = minute;
    nTimeArray[6] = second;

    env->ReleaseIntArrayElements(jTimeArray, nTimeArray, 0);
    return jTimeArray;
}
static jint android_SystemController_native_setRobotTime(JNIEnv *env, jobject thiz,
        jint year, jint month, jint day,
        jint weekday, jint hour, jint minute, jint second)
{
    return g_sys_info_ptr->SetRobotTime(year, month, day, weekday, hour, minute, second);
}
static jint android_SystemController_native_getRobotUpTime(JNIEnv *env, jobject thiz)
{
    int up_time = 0;
    int ret = g_sys_info_ptr->GetRobotUptime(up_time);

    if (!ret)
        return up_time;

    return ret;
}

static JNINativeMethod method_table[] = 
{
    { "_initialize",       "()V",                                          (void *) android_SystemController_native_init },
    { "_setup",            "()I",                                          (void *) android_SystemController_native_setup },
    { "_finalize",         "()I",                                          (void *) android_SystemController_native_finalize },
    { "_setListener",      "(L"SCLISTENERCLASS ";)I",                      (void *) android_SystemController_native_setListener },
    { "_send",             "(IIIIL" SCONRESULT ";)I",                      (void *) android_SystemController_native_send },
    { "_setEnable",        "(IIIL" SCONRESULT ";)I",                       (void *) android_SystemController_native_setEnable },
    { "_isEnable",         "(IIL" SCONRESULT ";)I",                        (void *) android_SystemController_native_isEnable },
    { "_setting",          "(IIIIL" SCONRESULT ";)I",                      (void *) android_SystemController_native_setting },
    {"_getSubSysVersion",  "(I)Ljava/lang/String;",                        (void *) android_SystemController_native_getSubSysVersion},
    {"_getRobotSN",        "()Ljava/lang/String;",                         (void *) android_SystemController_native_getRobotSN},
    {"_isRobotMoving",     "()Z",                                          (void *) android_SystemController_native_isRobotMoving},
    {"_getSubSysState",    "(I)L"ROBOTSUBSYSSTATE";",                      (void *) android_SystemController_native_getSubSysState},
    {"_getRobotSex",       "()Z",                                          (void *) android_SystemController_native_getRobotSex},
    {"_setRobotSex",       "(Z)I",                                         (void *) android_SystemController_native_setRobotSex},
    {"_getRobotTime",      "()[I",                                         (void *) android_SystemController_native_getRobotTime},
    {"_setRobotTime",      "(IIIIIII)I",                                   (void *) android_SystemController_native_setRobotTime},
    {"_getRobotUpTime",    "()I",                                          (void *) android_SystemController_native_getRobotUpTime},
};

static int registerRobotSubSysState(JNIEnv* env)
{
    jclass clazz = env->FindClass(ROBOTSUBSYSSTATE);
    if (clazz == NULL) {
        LOGE("###", "Can't find %s", ROBOTSUBSYSSTATE);
        return -1;
    }

    constructor_subsys_state = env->GetMethodID(clazz, "<init>", "()V");
    if (constructor_subsys_state == NULL) {
        LOGE("###", "Can't find %s constructor", ROBOTSUBSYSSTATE);
        return -1;
    }

    field_subsys_is_enable = env->GetFieldID(clazz, "mIsEnable", "Z");
    if (field_subsys_is_enable == NULL) {
        LOGE("###", "Can't find %s.mIsEnable", ROBOTSUBSYSSTATE);
        return -1;
    }

    field_subsys_is_poweron = env->GetFieldID(clazz, "mIsPowerOn", "Z");
    if (field_subsys_is_poweron == NULL) {
        LOGE("###", "Can't find %s.mIsPowerOn", ROBOTSUBSYSSTATE);
        return -1;
    }

    field_subsys_is_active = env->GetFieldID(clazz, "mIsActive", "Z");
    if (field_subsys_is_active == NULL) {
        LOGE("###", "Can't find %s.mIsActive", ROBOTSUBSYSSTATE);
        return -1;
    }

    field_subsys_is_initalized = env->GetFieldID(clazz, "mIsInitalized", "Z");
    if (field_subsys_is_initalized == NULL) {
        LOGE("###", "Can't find %s.mIsInitalized", ROBOTSUBSYSSTATE);
        return -1;
    }

    field_subsys_is_debug_en = env->GetFieldID(clazz, "mIsDebugEn", "Z");
    if (field_subsys_is_debug_en == NULL) {
        LOGE("###", "Can't find %s.mIsDebugEn", ROBOTSUBSYSSTATE);
        return -1;
    }

    field_subsys_state = env->GetFieldID(clazz, "mState", "I");
    if (field_subsys_state == NULL) {
        LOGE("###", "Can't find %s.mState", ROBOTSUBSYSSTATE);
        return -1;
    }

    field_subsys_sub_state = env->GetFieldID(clazz, "mSubState", "I");
    if (field_subsys_sub_state == NULL) {
        LOGE("###", "Can't find %s.mSubState", ROBOTSUBSYSSTATE);
        return -1;
    }

    field_subsys_mode = env->GetFieldID(clazz, "mMode", "I");
    if (field_subsys_mode == NULL) {
        LOGE("###", "Can't find %s.mMode", ROBOTSUBSYSSTATE);
        return -1;
    }

    g_sys_info_ptr = new android::RobotSysInfo();
    clazz_subsys_state = (jclass)env->NewGlobalRef(clazz);

    return 0;
}

/*
 * Register several native methods for one class.
 */
static int registerNativeMethods(JNIEnv* env, JNINativeMethod* gMethods, int numMethods) 
{
    jclass clazz;
    clazz = env->FindClass(SYSCONTRLCLASS);
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
    
    LOGE("###", "HAL_SYSTEM_CONTROL LOAD");

    if (vm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK) {
        return JNI_ERR;
    }
    if (!registerNativeMethods(env, method_table, (sizeof(method_table) / sizeof(JNINativeMethod)))) {
        return JNI_ERR;
    }
    if (registerRobotSubSysState(env) < 0)
    {
        return JNI_ERR;
    }

    return JNI_VERSION_1_4;
}

void JNI_OnUnLoad(JavaVM * vm, void * reserved) {

    JNIEnv* env = NULL;
    if (vm->GetEnv((void**)&env, JNI_VERSION_1_4) == JNI_OK) 
    {
        LOGE("###", "HAL_SYSTEM_CONTROL UNLOAD");
    }
}

