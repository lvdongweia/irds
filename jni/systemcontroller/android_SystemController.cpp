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
#include "IMessageListener.h"
#include "IDeathNotifier.h"
#include "IRDService.h"
#include "RDSLog.h"

using namespace android;

#define SYSCONTRLCLASS "android/robot/system/SystemController"

#define SCLISTENERCLASS SYSCONTRLCLASS "$Listener"
#define SCONRESULT SYSCONTRLCLASS "$OnResult"

struct fields_t {
    jfieldID    context;
    jmethodID   onMessage;
};

static jmethodID onQueryResult;
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

static int android_SystemController_send(JNIEnv *env, jobject thiz, int to, int cmd, int arg1, int arg2, jobject obj)
{
    GetSystemController(systemController, env, thiz);
    if (systemController.get() == 0)
    {
        LOGE("###", "call jni systemcontroller send error");
        return -1;
    }
     LOGE("###", "call jni systemcontroller send");
    sp<JNIOnQueryResult> cb = new JNIOnQueryResult(env, obj);
    return systemController->send(to, cmd, arg1, arg2, cb);
}
static JNINativeMethod method_table[] = 
{
    { "_initialize",     "()V",                                          (void *) android_SystemController_native_init        },
    { "_setup",          "()I",                                          (void *) android_SystemController_native_setup       },
    { "_finalize",       "()I",                                          (void *) android_SystemController_native_finalize    },
    { "_setListener",    "(L"SCLISTENERCLASS ";)I",                      (void *) android_SystemController_native_setListener },
    { "_send",           "(IIIIL" SCONRESULT ";)I",                      (void *) android_SystemController_send               },
};


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
    return JNI_VERSION_1_4;
}

void JNI_OnUnLoad(JavaVM * vm, void * reserved) {

    JNIEnv* env = NULL;
    if (vm->GetEnv((void**)&env, JNI_VERSION_1_4) == JNI_OK) 
    {
        LOGE("###", "HAL_SYSTEM_CONTROL UNLOAD");
    }
}

