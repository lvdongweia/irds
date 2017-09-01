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
#include "IMotionPlayer.h"

#include "ICompletionListener.h"
#include "IDeathNotifier.h"
#include "IServiceListener.h"
#include "IRDService.h"
#include "RDSLog.h"

using namespace android;

#define ROBOTPLAYERCLASS "android/robot/motion/RobotPlayer"
#define MPLISTENERCLASS ROBOTPLAYERCLASS "$Listener"

struct fields_t {
    jfieldID    context;
    jmethodID   onCompletion;
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
    }
    virtual ~JNIMotionServiceListener()
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
    virtual int onCompletion(int session_id, int result, int errorcode)
    {
        if (fields.onCompletion == 0 || mListenerObject == 0)
        {
            return 0;
        }
        JNIEnv *env = AndroidRuntime::getJNIEnv();
		env->CallVoidMethod(mListenerObject, fields.onCompletion, result, errorcode);
        return 0;
    }
 
private:
    jobject mListenerObject;
};

class MotionPlayerCtx : public RefBase
{
    private:
        class JNIDeathNotifier : public IDeathNotifier
        {
        public:
            JNIDeathNotifier(MotionPlayerCtx * ctx)
            {
                this->ctx = ctx;
            }
            virtual void died()
            {
                LOGE("###", "Motionplayer service died!!!!!");
                if (ctx == NULL)
                {
                    return;
                }
                ctx->motionPlayer = 0;
                if (ctx->motionListener.get() != 0) ctx->motionListener->onCompletion(0, 0, 0xFE);
            }
        private:
            MotionPlayerCtx * ctx;
        };

    private:
        static const char * token;

        sp<JNIDeathNotifier> rdsDeathNotifier;
        jobject   mListenerObject;
        char * fileName;
        int offset;
        int size;
    public:
        sp<JNIMotionServiceListener> motionListener;
        sp<IMotionPlayer> motionPlayer;
    public:
        MotionPlayerCtx()
        {
            motionPlayer = NULL;
            motionListener = NULL;
            mListenerObject = 0;
            fileName = NULL;
            offset = 0;
            size = 0;
        }
        virtual ~MotionPlayerCtx()
        {
            motionPlayer = NULL;
            rdsDeathNotifier = NULL;
            motionListener = NULL;
            if (fileName != NULL)
            {
                free(fileName);
            }
            fileName = NULL;
            if (mListenerObject != 0)
            {
                JNIEnv *env = AndroidRuntime::getJNIEnv();
                env->DeleteGlobalRef(mListenerObject);   
            }
        }
        virtual void setListener(JNIEnv* env, jobject obj)
        {
            if (mListenerObject != 0)
            {
                env->DeleteGlobalRef(mListenerObject);   
            }
            mListenerObject = 0;
            if (motionListener.get() != 0) motionListener->setListenerObject(env, obj);
            else mListenerObject  = env->NewGlobalRef(obj);
        }
        int prepare(JNIEnv* env, jobject thiz)
        {
            int result = 0;

            const sp<IRDService>& service = IDeathNotifier::GetRDService(); //tiemout 2 sec;

            if (service.get() == 0)
            {
                return -1;
            }
            if (motionListener.get() == 0)
            {
                motionListener = new JNIMotionServiceListener(env, thiz);
            }
            if (rdsDeathNotifier.get() == 0)
            {
                rdsDeathNotifier = new JNIDeathNotifier(this);
            }
            if (motionPlayer.get() == 0)
            {
                motionPlayer = service->getMotionPlayer(token, strlen(token));
            }
            if (motionPlayer.get() == 0)
            {
                return -2;
            }
            if (mListenerObject != 0)
            {
                motionListener->setListenerObject(env, mListenerObject);
            }
            motionPlayer->setCompletionListener(motionListener);
            if (fileName != NULL)
            {
                motionPlayer->setDataSource(fileName, offset, size);
            }
            else if (g_cache_size > 0)
            {
                motionPlayer->setDataSourceByStream((const unsigned char *)g_cache, g_cache_size);
            }
            if (motionPlayer->prepare() < 0)
            {
                return -3;
            }
            return 0;
        }
        int setDataSourceByStream(const unsigned char * bytes, int size)
        {
            if (fileName != NULL)
            {
                free(fileName);
            }
            fileName = NULL;

            return 0;
        }

        int setDataSource(const char * path, int offset, int size)
        {
            int len = strlen(path);

            if (fileName != NULL)
            {
                free(fileName);
            }

            if (size <= 0)
            {
                FILE * fp = fopen(path, "rb");
                if (fp == 0)
                {
                    return -1;
                }
                fseek(fp, 0, SEEK_END);
                size = ftell(fp);
                offset = 0;
                fclose(fp);
            }
            fileName = (char*)malloc(len + 1);
            memset(fileName, 0, len + 1);
            strcpy(fileName, path);
            this->offset = offset;
            this->size = size;
            return 0;
        }
};

const char * MotionPlayerCtx::token = "5C34D63D-E8F4-4070-957B-A80868721067";

#define GetMotionPlayerCtx(name, env, thiz)    \
                                    sp<MotionPlayerCtx> name; \
                                    do \
                                    { \
                                        name = getMotionPlayerCtx(env, thiz); \
                                        if (name.get() == 0) \
                                        { \
                                            LOGE("###", " Can't find MotionPlayerCtx"); \
                                            return -1; \
                                        } \
                                    }\
                                    while(0)


#define GetMotionPlayer(name, env, thiz)    \
                                    sp<IMotionPlayer> name = 0; \
                                    sp<MotionPlayerCtx>ctx; \
                                    do \
                                    { \
                                        ctx = getMotionPlayerCtx(env, thiz); \
                                        if (ctx.get() == 0 )\
                                        { \
                                            LOGE("###", " Can't find MotionPlayerCtx"); \
                                            return -1; \
                                        } \
                                        if (ctx->motionPlayer.get() == 0)\
                                        {\
                                            LOGE("###", "Can't find Motionplayer instance");\
                                            return -2;\
                                        }\
                                        name = ctx->motionPlayer;\
                                    }\
                                    while(0)

static sp<MotionPlayerCtx> getMotionPlayerCtx(JNIEnv* env, jobject thiz)
{
    Mutex::Autolock l(sLock);
    MotionPlayerCtx* const p = (MotionPlayerCtx*)env->GetIntField(thiz, fields.context);

    return sp<MotionPlayerCtx>(p);
}

static sp<MotionPlayerCtx> setMotionPlayerCtx(JNIEnv* env, jobject thiz, const sp<MotionPlayerCtx>& player)
{
    Mutex::Autolock l(sLock);
    sp<MotionPlayerCtx> old = (MotionPlayerCtx*)env->GetIntField(thiz, fields.context);
    if (player.get()) {
        player->incStrong((void*)setMotionPlayerCtx);
    }
    if (old != 0) {
        old->decStrong((void*)setMotionPlayerCtx);
    }
    env->SetIntField(thiz, fields.context, (int)player.get());
    return old;
}

static void android_motion_MotionPlayer_native_init(JNIEnv *env)
{
    jclass clazz;

    clazz = env->FindClass(MPLISTENERCLASS);
    if (clazz == NULL) {
        return;
    }
    fields.onCompletion = env->GetMethodID(clazz, "onCompleted", "(II)V");
    clazz = env->FindClass(ROBOTPLAYERCLASS);
    if (clazz == NULL) {
        return;
    }
    fields.context = env->GetFieldID(clazz, "mNativeContext", "I");
}

static int android_motion_MotionPlayer_native_setup(JNIEnv *env, jobject thiz)
{
    sp<MotionPlayerCtx> mp = new MotionPlayerCtx();
    setMotionPlayerCtx(env, thiz, mp);
    return 0;
}

static int android_motion_MotionPlayer_native_finalize(JNIEnv *env, jobject thiz)
{
    Mutex::Autolock l(sLock);
    sp<MotionPlayerCtx> player = (MotionPlayerCtx*)env->GetIntField(thiz, fields.context);
    
    if (player != 0) {
        player->decStrong((void*)setMotionPlayerCtx);
    }
    env->SetIntField(thiz, fields.context, 0);
    player = 0;
    return 0;
}
static int android_motion_MotionPlayer_native_setListener(JNIEnv *env, jobject thiz, jobject obj)
{
    GetMotionPlayerCtx(ctx, env, thiz);
    ctx->setListener(env,obj);
    return 0;
}
static int android_motion_MotionPlayer_setDataSource(JNIEnv *env, jobject thiz, jstring pathObj, int offset, int size)
{
    const char * fileName = env->GetStringUTFChars(pathObj, NULL);
	GetMotionPlayerCtx(ctx, env, thiz);
	ctx->setDataSource(fileName, offset, size);
    if (fileName != NULL) {
        env->ReleaseStringUTFChars(pathObj, fileName);
        fileName = NULL;
    }
	return 0;
}

static int android_motion_MotionPlayer_setDataSourceByStream(JNIEnv *env, jobject thiz, jbyteArray pdata, int offset, int size)
{
    GetMotionPlayerCtx(ctx, env, thiz);

    if (size > g_cache_size)
    {
		if (g_cache != NULL)
		{
			free(g_cache);
		}
		g_cache = malloc(size);
    }
    g_cache_size = size;
    env->GetByteArrayRegion(pdata, offset, size, (jbyte*)g_cache);
    ctx->setDataSourceByStream((const unsigned char *)g_cache, size);

	return 0;
}

static int android_motion_MotionPlayer_prepare(JNIEnv *env, jobject thiz)
{
    GetMotionPlayerCtx(ctx, env, thiz);
    return ctx->prepare(env, thiz);
}
static int android_motion_MotionPlayer_start(JNIEnv *env, jobject thiz)
{
    GetMotionPlayer(motionPlayer, env, thiz);
    return motionPlayer->start();
}
static int android_motion_MotionPlayer_pause(JNIEnv *env, jobject thiz)
{
    GetMotionPlayer(motionPlayer, env, thiz);
    return motionPlayer->pause();
}
static int android_motion_MotionPlayer_resume(JNIEnv *env, jobject thiz)
{
    GetMotionPlayer(motionPlayer, env, thiz);
    return motionPlayer->resume();
}
static int android_motion_MotionPlayer_stop(JNIEnv *env, jobject thiz)
{
    GetMotionPlayer(motionPlayer, env, thiz);
    return motionPlayer->stop();
}
static int android_motion_MotionPlayer_getDuration(JNIEnv *env, jobject thiz)
{
    GetMotionPlayer(motionPlayer, env, thiz);
    return motionPlayer->getDuration();
}

static int android_motion_MotionPlayer_getPosition(JNIEnv *env, jobject thiz)
{
    GetMotionPlayer(motionPlayer, env, thiz);
    return motionPlayer->getPosition();
}
static int android_motion_MotionPlayer_setPosition(JNIEnv *env, jobject thiz, int pos)
{
    GetMotionPlayer(motionPlayer, env, thiz);
    return motionPlayer->setPosition(pos);
}

static int android_motion_MotionPlayer_setEnableRun(JNIEnv *env, jobject thiz, int enable)
{
	GetMotionPlayer(motionPlayer, env, thiz);
    return motionPlayer->setEnableRun(enable);
}
static int android_motion_MotionPlayer_getRunStatus(JNIEnv *env, jobject thiz)
{
	GetMotionPlayer(motionPlayer, env, thiz);
    return motionPlayer->getRunStatus();
}

static JNINativeMethod method_table[] = 
{
    { "_initialize",           "()V",                                          (void *) android_motion_MotionPlayer_native_init        },
    { "_setup",                "()I",                                          (void *) android_motion_MotionPlayer_native_setup       },
    { "_finalize",             "()I",                                          (void *) android_motion_MotionPlayer_native_finalize    },
    { "_setListener",          "(L" MPLISTENERCLASS ";)I",                     (void *) android_motion_MotionPlayer_native_setListener },
    { "_setDataSource",        "(Ljava/lang/String;II)I",                      (void *) android_motion_MotionPlayer_setDataSource      },
    { "_setDataSourceByStream",          "([BII)I",                                     (void *) android_motion_MotionPlayer_setDataSourceByStream },
    { "_prepare",        "()I",                                          (void *) android_motion_MotionPlayer_prepare            },
    { "_start",          "()I",                                          (void *) android_motion_MotionPlayer_start              },
    { "_pause",          "()I",                                          (void *) android_motion_MotionPlayer_pause              },
    { "_resume",         "()I",                                          (void *) android_motion_MotionPlayer_resume             },
    { "_stop",           "()I",                                          (void *) android_motion_MotionPlayer_stop               },
    { "_getDuration",    "()I",                                          (void *) android_motion_MotionPlayer_getDuration        },
    { "_getPosition",    "()I",                                          (void *) android_motion_MotionPlayer_getPosition        },
    { "_setPosition",    "(I)I",                                         (void *) android_motion_MotionPlayer_setPosition        },
    { "_setEnableRun",    "(I)I",                                        (void *) android_motion_MotionPlayer_setEnableRun       },
    { "_getRunStatus",    "()I",                                          (void *) android_motion_MotionPlayer_getRunStatus      },
};


/*
 * Register several native methods for one class.
 */
static int registerNativeMethods(JNIEnv* env, JNINativeMethod* gMethods, int numMethods) 
{
    jclass clazz;
    clazz = env->FindClass(ROBOTPLAYERCLASS);
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
    
    LOGE("###", "JNI_MOTIONPLAYER LOAD");

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
    ILOG();
    if (vm->GetEnv((void**)&env, JNI_VERSION_1_4) == JNI_OK) 
    {
        LOGE("###", "JNI_MOTIONPLAYER UNLOAD");
    }
}

