#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>
#include "ICompletionListener.h"
#include "IServiceListener.h"
#include "IRDService.h"
#include "IMotionService.h"
#include "client/MotionPlayer.h"
#include "client/MotionService.h"
#include <utils/threads.h>
#include <android/log.h>
#include "ipal_motion_service_bridge.h"

#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO, "ipal_motion_service_bridge", __VA_ARGS__)

class TaskCompletionListener : public BnCompletionListener
{
public:
    TaskCompletionListener()
    {
        mListener = NULL;
    }

    virtual int onCompletion(int session_id,int result, int error_code)
    {
        LOGI("task completioin :%d, %d, %d", session_id, result, error_code);
        if (mListener != NULL) {
           mListener->onCompletion(session_id, result, error_code);
        }
        return 0;
    }

    void setListener(jI_MotionEventListener* listener)
    {
        mListener = listener;
    }

private:
    jI_MotionEventListener* mListener;
};

class jI_MotionPlayerBridgeImpl : public jI_MotionPlayerBridge {
public:
    jI_MotionPlayerBridgeImpl() {
        LOGI("jI_MotionPlayerBridgeImpl::jI_MotionPlayerBridgeImpl\n");
        ProcessState::self()->startThreadPool();
        j__motionService = new MotionService();
        j__listener = new TaskCompletionListener();
        j__motionService->setCompletionListener(j__listener);
    }

public:
    virtual int setCompletionListener(jI_MotionEventListener* onCompletionListener) {
        if (j__listener != NULL) {
            j__listener->setListener(onCompletionListener);
        }
        LOGI("jI_MotionPlayerBridgeImpl IN %s, %d\n", __FUNCTION__, __LINE__);
        return 0;
    }

    virtual int stop(int session_id = 0) {
        LOGI("jI_MotionPlayerBridgeImpl IN %s, %d\n", __FUNCTION__, __LINE__);
        return j__motionService->stop(session_id);
    }

    virtual int reset(int id = 0) {
        LOGI("jI_MotionPlayerBridgeImpl IN %s, %d\n", __FUNCTION__, __LINE__);
        return j__motionService->reset(id);
    }

    virtual int doAction(int id, int repeat, int duration) {
        LOGI("jI_MotionPlayerBridgeImpl IN %s, %d\n", __FUNCTION__, __LINE__);
        return j__motionService->doAction(id, repeat, duration);
    }

    virtual int run(const void * pdata, int len, int duration = 0, int flags = 0) {
        LOGI("jI_MotionPlayerBridgeImpl IN %s, %d\n", __FUNCTION__, __LINE__);
        return j__motionService->run(pdata, len, duration, flags);
    }

    virtual int runMotor(int id, int angle, int duration/*ms*/, int flags) {
        LOGI("jI_MotionPlayerBridgeImpl IN %s, %d\n", __FUNCTION__, __LINE__);
        return j__motionService->runMotor(id, angle, duration, flags);
    }

    virtual int walk(int distance/*cm*/, int speed, int flag = 0) {
        LOGI("jI_MotionPlayerBridgeImpl IN %s, %d\n", __FUNCTION__, __LINE__);
        return j__motionService->walk(distance, speed, flag);
    }

    virtual int walkTo(int target, int speed, int flag = 0) {
        LOGI("jI_MotionPlayerBridgeImpl IN %s, %d\n", __FUNCTION__, __LINE__);
        return j__motionService->walkTo(target, speed, flag);
    }

    virtual int turn(int angle, int speed) {
        LOGI("jI_MotionPlayerBridgeImpl IN %s, %d\n", __FUNCTION__, __LINE__);
        return j__motionService->turn(angle, speed);
    }

    virtual int emoji(int id, int duration, int repeat) {
        LOGI("jI_MotionPlayerBridgeImpl IN %s, %d\n", __FUNCTION__, __LINE__);
        return j__motionService->emoji(id, duration, repeat);
    }

private:
    sp<MotionService> j__motionService;
    TaskCompletionListener* j__listener;
};

jI__EXPORT jI_MotionPlayerBridge*  jI_MotionPlayerBridge__New(void* reserved) {
    (void)reserved;

    static jI_MotionPlayerBridgeImpl sigleInstance;
    return &sigleInstance;
}

jI__EXPORT void  jI_MotionPlayerBridge__Destroy(jI_MotionPlayerBridge* j__bridge) {
    (void)j__bridge;
}
