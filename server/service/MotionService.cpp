/*
*/

#include <stdint.h>
#include <sys/types.h>

#include <binder/Parcel.h>
#include <utils/String8.h>

#include "RDSLog.h"
#include "RDSTask.h"
#include "ICompletionListener.h"
#include "MotionService.h"

namespace android 
{
    MotionService::MotionService(const void * token, int len) : ServiceObject(token, len)
    {
        LOGE("###", "MotionService create");
    }
    MotionService::~MotionService()
    {
        LOGE("###", "MotionService release");
    }
    int MotionService::setCompletionListener(const sp<ICompletionListener> & onCompletionListener)
    {
        this->onCompletionListener = onCompletionListener;
        return 0;
    }
    int MotionService::setting(int id, int cmd, int arg1, int arg2, const sp<IResultListener> & onResultListener)
    {
        //LOGE("###", "query motion id=0x%x cmd=%d", id, cmd);

        SettingObject * so = new SettingObject(uid);
        return so->Run(id, cmd, arg1, arg2, onResultListener);
    }
    int MotionService::stop(int session_id, int bodyPart, int arg1)
    {
        if (session_id == 0)
        {
            StopMotion::Run(0, arg1);
        }
        RDSTask::Instance()->cancelCommand(session_id);
        return 0;
    }
    int MotionService::reset(int id)
    {
        return ResetMotion::Run(id);
    }
    int MotionService::doAction(int id, int repeat, int duration, int flags)
    {
        LOGE("###", "doAction id=0x%x repeat=%d duration:%d", id, repeat, duration);
        ActionMotion * ao = new ActionMotion(uid);
        ao->flags = flags;
        return ao->Run(id, repeat, duration);
    }
    int MotionService::runMotor(int id, int angle, int duration/*msec*/, int flags)
    {
        //LOGE("###", "runMotor id=%d angle=%d", id, angle);
        MotorMotion * mo = new MotorMotion(uid);
        mo->flags = flags;
        return mo->Run(id, angle, duration, flags);
    }
    int MotionService::run(const void * pdata, int len, int duration, int flags)
    {
        if ((flags & 0x40) != 0)
        {
            LOGE("###", "----run motor............flag = 0x01");
            return motorMotion.Run((const unsigned char *)pdata, len, duration);
        }
        MotorMotion * mo = new MotorMotion(uid);
        return mo->Run((const unsigned char *)pdata, len, duration);
    }

    int MotionService::walk(int distance, int speed, int flags)
    {
        PositionMotion * mo = new PositionMotion(uid);
        mo->flags = flags;
        return mo->Walk(distance, speed, flags);
    }
    int MotionService::walkTo(int target, int speed, int flags)
    {
        PositionMotion * mo = new PositionMotion(uid);
        return mo->WalkTo(target, speed, flags);
    }
    int MotionService::turn(int angle, int speed, int flags)
    {
        PositionMotion * mo = new PositionMotion(uid);
        mo->flags = flags;
        return mo->Turn(angle, speed);
    }
    int MotionService::emoji(int id, int duration, int repeat, int flags)
    {
        EmojiMotion * mo = new EmojiMotion(uid);
        mo->flags = flags;
        return mo->Run(id, duration, repeat);
    }
    void MotionService::release()
    {
        isUnset = true;
    }
}; /* namespace android */
