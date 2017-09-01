/*
 */
#include <utils/Mutex.h>
#include <pthread.h>
#include <utils/threads.h>
#include "IRDService.h"
#include "MotionService.h"
#include "RDSLog.h"

using namespace android;

const char * MotionService::token ="B44B9DDC-1976-4F6A-BA7D-67B5A40515C1";

MotionService::DeathNotifier::DeathNotifier(MotionService * ctx)
{
    this->ctx = ctx;
}
void MotionService::DeathNotifier::died()
{
    if (ctx->mListener.get() != 0)
    {
        ctx->mListener->onCompletion(0, 0, 0xFE);
    }
}

MotionService::MotionService(bool bprepare)
{
    motionService = NULL;
    mListener = NULL;
    mDeathNotifier = new DeathNotifier(this);

    if (bprepare == true)
    {
        prepare();
    }
}
MotionService::~MotionService()
{
    mDeathNotifier = NULL;
    motionService = NULL;
    mListener = NULL;
}


int MotionService::prepare()
{
    const sp<IRDService>& service = IDeathNotifier::GetRDService(2); //tiemout 2 sec;
    if (service.get() == 0)
    {
        return -1;
    }
    if (motionService.get() == 0)
    {
        motionService = service->getMotionService(token, strlen(token));
    }
    if (motionService.get() == 0)
    {
        return -1;
    }
    return 0;
}
int MotionService::setting(int id, int cmd, int arg1, int arg2, const sp<IResultListener> & onResultListener)
{
    if (motionService.get() != 0) return motionService->setting(id,cmd, arg1, arg2, onResultListener);

    return -1;
}

int MotionService::setCompletionListener(sp<ICompletionListener> listener)
{
	mListener = listener;
    if (motionService.get() != 0) motionService->setCompletionListener(listener);
    return 0;
}
int MotionService::stop(int session_id, int bodyPart, int arg1)
{
    if (motionService.get() != 0) return motionService->stop(session_id, bodyPart, arg1);

    return -1;
}

int MotionService::reset(int id)
{
    if (motionService.get() != 0) return motionService->reset(id);

    return -1;   
}
int MotionService::doAction(int id, int repeat, int duration, int flags)
{
    if (motionService.get() != 0) return motionService->doAction(id, repeat, duration, flags);

    return -1;   
}
int MotionService::run(const void * pdata, int len, int duration, int flags)
{
    if (motionService.get() != 0) return motionService->run(pdata, len, duration, flags);

    return -1;   
}
int MotionService::runMotor(int id, int angle, int duration/*msec*/, int flags)
{
    if (motionService.get() != 0) return motionService->runMotor(id, angle, duration, flags);

    return -1;   
}
int MotionService::walk(int distance/*cm*/, int speed, int flags)
{
    if (motionService.get() != 0) return motionService->walk(distance, speed, flags);

    return -1;   
}
int MotionService::walkTo(int target, int speed, int flags)
{
    if (motionService.get() != 0) return motionService->walkTo(target, speed, flags);

    return -1;   
}
int MotionService::turn(int angle, int speed, int flags)
{
    if (motionService.get() != 0) return motionService->turn(angle, speed, flags);

    return -1;   
}
int MotionService::emoji(int id, int duration, int repeat, int flags)
{
    if (motionService.get() != 0) return motionService->emoji(id, duration, repeat, flags);

    return -1;   
}


