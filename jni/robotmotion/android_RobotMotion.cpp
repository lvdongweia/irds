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
#include "IMotionService.h"

#include "ICompletionListener.h"
#include "IResultListener.h"
#include "IDeathNotifier.h"
#include "IServiceListener.h"
#include "IRDService.h"
#include "RDSLog.h"

using namespace android;

#define MOTIONSERVICECLASS "android/robot/motion/RobotMotion"
#define MSLISTENERCLASS MOTIONSERVICECLASS "$Listener"
#define MSONRESULTCLASS MOTIONSERVICECLASS "$OnResult"

struct fields_t {
    jfieldID    context;
    jmethodID   onCompletion;
    jmethodID   onResult;
};
static fields_t fields;

static Mutex sLock;

static void * g_cache = NULL;
static int g_cache_size = 0;



// ----------------------------------------------------------------------------
// ref-counted object for callbacks

class JNIMotionServiceListener: public BnCompletionListener
{
public:
    JNIMotionServiceListener(JNIEnv* env, jobject thiz)
    {
        mListenerObject  = 0;
        mCallbackObject = 0;
    }
    virtual ~JNIMotionServiceListener()
    {
        // remove global references
        JNIEnv *env = AndroidRuntime::getJNIEnv();
        if (mListenerObject != 0) env->DeleteGlobalRef(mListenerObject);
        if (mCallbackObject != 0) env->DeleteGlobalRef(mCallbackObject);
        mListenerObject = 0;
        mCallbackObject = 0;
    }
    virtual void setListenerObject(JNIEnv* env, jobject obj)
    {
        mListenerObject  = env->NewGlobalRef(obj);
    }
    virtual void setCallbackListenerObject(JNIEnv* env, jobject obj)
    {
        mCallbackObject  = env->NewGlobalRef(obj);
    }
    virtual int onCompletion(int session_id, int result, int errorcode)
    {
        if (fields.onCompletion == 0 || (mListenerObject == 0 && mCallbackObject == 0))
        {
            return 0;
        }
        JNIEnv *env = AndroidRuntime::getJNIEnv();
        if(mCallbackObject != 0)
        {
			env->CallVoidMethod(mCallbackObject, fields.onCompletion, session_id, result, errorcode);
			env->DeleteGlobalRef(mCallbackObject);
			mCallbackObject = 0;
		}
		else
			env->CallVoidMethod(mListenerObject, fields.onCompletion, session_id, result, errorcode);
        return 0;
    }
private:
    jobject mListenerObject;
    jobject mCallbackObject;
};

class JNIMotionResultListener: public BnResultListener
{
public:
    JNIMotionResultListener(JNIEnv* env, jobject thiz)
    {
        mListenerObject  = 0;
    }
    virtual ~JNIMotionResultListener()
    {
        // remove global references
        JNIEnv *env = AndroidRuntime::getJNIEnv();
        if (mListenerObject != 0) env->DeleteGlobalRef(mListenerObject);
        mListenerObject = 0;
    }
    virtual void setListenerObject(JNIEnv* env, jobject obj)
    {
        mListenerObject  = env->NewGlobalRef(obj);
    }
    virtual int onCompletion(int id, int result)
    {
		LOGE("###", "hcf onCompletion");
        if (fields.onResult == 0 || mListenerObject == 0)
        {
			LOGE("###", "hcf fields.onResult == 0 || mListenerObject == 0");
            return 0;
        }
        JNIEnv *env = AndroidRuntime::getJNIEnv();
		env->CallVoidMethod(mListenerObject, fields.onResult, id, result);
        return 0;
    }
private:
    jobject mListenerObject;
};

class MotionServiceCtx : public RefBase
{
    private:
        class JNIDeathNotifier : public IDeathNotifier
        {
        public:
            JNIDeathNotifier(MotionServiceCtx * ctx)
            {
                this->ctx = ctx;
            }
            virtual void died()
            {
                if (ctx == NULL)
                {
                    return;
                }
                ctx->motionService = NULL;
                if (ctx->motionListener.get() != 0) ctx->motionListener->onCompletion(0, 0, 0xFE);
            }
        private:
            MotionServiceCtx * ctx;
        };

   private:
        static const char * token;
        sp<JNIDeathNotifier> rdsDeathNotifier;
    public:
        sp<JNIMotionServiceListener> motionListener;
        sp<JNIMotionServiceListener> callbackListener;
        sp<IMotionService> motionService;
    public:
        MotionServiceCtx(JNIEnv* env, jobject thiz)
        {
            motionService = NULL;
            rdsDeathNotifier = new JNIDeathNotifier(this);
            motionListener = new JNIMotionServiceListener(env, thiz);
            callbackListener = new JNIMotionServiceListener(env, thiz);
        }
        virtual ~MotionServiceCtx()
        {
            motionService = NULL;
            motionListener = NULL;
            rdsDeathNotifier = NULL;
        }
        virtual void setListener(JNIEnv* env, jobject obj)
        {
            motionListener->setListenerObject(env, obj);
        }
        virtual void setCallbackListener(JNIEnv* env, jobject obj)
        {
            callbackListener->setListenerObject(env, obj);
        }
        int prepare(JNIEnv* env, jobject thiz)
        {
            const sp<IRDService>& service = IDeathNotifier::GetRDService(); //tiemout 1 sec;

            if (service.get() == 0)
            {
                return -1;
            }
            if (motionService.get() == 0)
            {
                motionService = service->getMotionService(token, strlen(token));
            }
            if (motionService.get() != 0)
            {
                motionService->setCompletionListener(motionListener);
            }
            return 0;
        }
        bool isReady()
        {
            return (motionService.get() != 0);
        }
};

const char * MotionServiceCtx::token = "5B44B9DDC-1976-4F6A-BA7D-67B5A40515C1";

#define GetMotionServiceCtx(name, env, thiz)    \
                                    sp<MotionServiceCtx> name; \
                                    do \
                                    { \
                                        name = getMotionServiceCtx(env, thiz); \
                                        if (name.get() == 0) \
                                        { \
                                            LOGE("###", " Can't find MotionServiceCtx"); \
                                            return -1; \
                                        } \
                                    }\
                                    while(0)


#define GetMotionService(name, env, thiz)    \
                                    sp<IMotionService> name = 0; \
                                    sp<MotionServiceCtx> ctx; \
                                    do \
                                    { \
                                        ctx = getMotionServiceCtx(env, thiz); \
                                        if (ctx.get() == 0) \
                                        { \
                                            LOGE("###", " Can't find MotionServiceCtx"); \
                                            return -1; \
                                        } \
                                        if (ctx->motionService.get() == 0)\
                                        { \
                                           ctx->prepare(env, thiz);\
                                        }\
                                        if (ctx->motionService.get() == 0) \
                                        {\
                                            LOGE("###", " Can't find MotionService"); \
                                            return -2;\
                                        }\
                                        name = ctx->motionService;\
                                    }\
                                    while(0)

static sp<MotionServiceCtx> getMotionServiceCtx(JNIEnv* env, jobject thiz)
{
    Mutex::Autolock l(sLock);
    MotionServiceCtx* const p = (MotionServiceCtx*)env->GetIntField(thiz, fields.context);
    return sp<MotionServiceCtx>(p);
}

static sp<MotionServiceCtx> setMotionServiceCtx(JNIEnv* env, jobject thiz, const sp<MotionServiceCtx>& service)
{
    Mutex::Autolock l(sLock);
    sp<MotionServiceCtx> old = (MotionServiceCtx*)env->GetIntField(thiz, fields.context);
    if (service.get()) {
        service->incStrong((void*)setMotionServiceCtx);
    }
    if (old != 0) {
        old->decStrong((void*)setMotionServiceCtx);
    }
    env->SetIntField(thiz, fields.context, (int)service.get());
    return old;
}

static void android_motion_MotionService_native_init(JNIEnv *env)
{
    jclass clazz;

    clazz = env->FindClass(MSLISTENERCLASS);
    if (clazz == NULL) {
        return;
    }
    fields.onCompletion = env->GetMethodID(clazz, "onCompleted", "(III)V");

    clazz = env->FindClass(MSONRESULTCLASS);
    if (clazz == NULL) {
        return;
    }
    fields.onResult = env->GetMethodID(clazz, "onCompleted", "(II)V");

    clazz = env->FindClass(MOTIONSERVICECLASS);
    if (clazz == NULL) {
        return;
    }
    fields.context = env->GetFieldID(clazz, "mNativeContext", "I");
}

static int android_motion_MotionService_native_setup(JNIEnv *env, jobject thiz)
{
    sp<MotionServiceCtx> ms = new MotionServiceCtx(env, thiz);
    setMotionServiceCtx(env, thiz, ms);
    
    LOGE("###", "MotionService Setup");
    return 0;
}

static int android_motion_MotionService_native_finalize(JNIEnv *env, jobject thiz)
{
    Mutex::Autolock l(sLock);
    sp<MotionServiceCtx> service = (MotionServiceCtx*)env->GetIntField(thiz, fields.context);
    
    if (service != 0) {
        service->decStrong((void*)setMotionServiceCtx);
    }
    env->SetIntField(thiz, fields.context, 0);
    service = NULL;

    LOGE("###", "MotionService Finalizes");
    return 0;
}
static int android_motion_MotionService_native_setListener(JNIEnv *env, jobject thiz, jobject obj)
{
    GetMotionServiceCtx(ctx, env, thiz);
    ctx->setListener(env,obj);
    return 0;
}

static int android_motion_MotionService_prepare(JNIEnv *env, jobject thiz)
{
    GetMotionServiceCtx(ctx, env, thiz);
    return ctx->prepare(env, thiz);
}

static jboolean android_motion_MotionService_isReady(JNIEnv *env, jobject thiz)
{
    GetMotionServiceCtx(ctx, env, thiz);
    return ctx->isReady() ? 1 : 0;
}
static int android_motion_MotionService_stop(JNIEnv *env, jobject thiz, int session_id, int bodyPart, int arg1)
{
    GetMotionService(motionService, env, thiz);
    return motionService->stop(session_id, bodyPart, arg1);
}
static int android_motion_MotionService_setting(JNIEnv *env, jobject thiz, int id, int cmd, int arg1, int arg2, jobject obj)
{
	GetMotionService(motionService, env, thiz);
    if(obj == NULL)
    {
         return motionService->setting(id, cmd, arg1, arg2, NULL);
    }

    sp<JNIMotionResultListener> cb = new JNIMotionResultListener(env, obj);
    cb->setListenerObject(env, obj);
    return motionService->setting(id, cmd, arg1, arg2, cb);
}
static int android_motion_MotionService_reset(JNIEnv *env, jobject thiz, int id)
{
    GetMotionService(motionService, env, thiz);
    return motionService->reset(id);
}

static int android_motion_MotionService_doAction(JNIEnv *env, jobject thiz,int id, int repeat, int duration, int flags)
{
    GetMotionService(motionService, env, thiz);
    return motionService->doAction(id, repeat, duration, flags);
}
static int android_motion_MotionService_runEx(JNIEnv *env, jobject thiz, jbyteArray pdata, int len, int duration, int flags)
{
    GetMotionService(motionService, env, thiz);
    if (len > g_cache_size)
    {
        if (g_cache != NULL)
        {
            free(g_cache);
        }
        g_cache = malloc(len);
        g_cache_size = len;
    }
    env->GetByteArrayRegion(pdata, 0, len, (jbyte*)g_cache);
    int result = motionService->run(g_cache, len, duration, flags);
    return result;
}
static int android_motion_MotionService_run(JNIEnv *env, jobject thiz, jbyteArray pdata, int len)
{
    return android_motion_MotionService_runEx(env, thiz, pdata, len, 0, 0);
}
static int android_motion_MotionService_runMotor(JNIEnv *env, jobject thiz,int id, int angle, int duration, int flags)
{
    GetMotionService(motionService, env, thiz);
    return motionService->runMotor(id, angle, duration, flags);
}
static int android_motion_MotionService_walk(JNIEnv *env, jobject thiz,int distance, int speed, int flags)
{
    GetMotionService(motionService, env, thiz);
    return motionService->walk(distance, speed, flags);
}
static int android_motion_MotionService_walkTo(JNIEnv *env, jobject thiz,int target, int speed, int flags)
{
    GetMotionService(motionService, env, thiz);
    return motionService->walkTo(target, speed, flags);
}
static int android_motion_MotionService_turn(JNIEnv *env, jobject thiz,int angle, int speed, int flags)
{
    GetMotionService(motionService, env, thiz);
    return motionService->turn(angle, speed, flags);
}
static int android_motion_MotionService_emoji(JNIEnv *env, jobject thiz,int id, int duration, int repeat, int flags)
{
    GetMotionService(motionService, env, thiz);
    return motionService->emoji(id, duration, repeat, flags);
}
static JNINativeMethod method_table[] = 
{
    { "_initialize",     "()V",                                          (void *) android_motion_MotionService_native_init        },
    { "_setup",          "()I",                                          (void *) android_motion_MotionService_native_setup       },
    { "_finalize",       "()I",                                          (void *) android_motion_MotionService_native_finalize    },
    { "_setListener",    "(L" MSLISTENERCLASS ";)I",                     (void *) android_motion_MotionService_native_setListener },
    { "_isReady",        "()Z",                                          (void *) android_motion_MotionService_isReady            },
    { "_prepare",        "()I",                                          (void *) android_motion_MotionService_prepare            },
    { "_setting",        "(IIIIL" MSONRESULTCLASS ";)I",                 (void *) android_motion_MotionService_setting            },
    { "_stop",           "(III)I",                                         (void *) android_motion_MotionService_stop               },
    { "_reset",          "(I)I",                                         (void *) android_motion_MotionService_reset              },
    { "_doAction",       "(IIII)I",                                      (void *) android_motion_MotionService_doAction           },
    { "_run",            "([BI)I",                                       (void *) android_motion_MotionService_run                },
    { "_runEx",          "([BIII)I",                                     (void *) android_motion_MotionService_runEx              },
    { "_runMotor",       "(IIII)I",                                      (void *) android_motion_MotionService_runMotor           },
    { "_walk",           "(III)I",                                       (void *) android_motion_MotionService_walk               },
    { "_walkTo",         "(III)I",                                       (void *) android_motion_MotionService_walkTo             },
    { "_turn",           "(III)I",                                       (void *) android_motion_MotionService_turn               },
    { "_emoji",          "(IIII)I",                                      (void *) android_motion_MotionService_emoji              },
};


/*
 * Register several native methods for one class.
 */
static int registerNativeMethods(JNIEnv* env, JNINativeMethod* gMethods, int numMethods) 
{
    jclass clazz;
    clazz = env->FindClass(MOTIONSERVICECLASS);
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
    
    LOGE("###", "JNI_MOTIONSERVICE LOAD");

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
        LOGE("###", "JNI_MOTIONSEVICE UNLOAD");
    }
}

