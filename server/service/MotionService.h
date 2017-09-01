/*
*/

#ifndef MotionService_H__
#define MotionService_H__

#include <utils/Log.h>
#include <utils/String8.h>

#include "IMotionService.h"
#include "ServiceObject.h"

namespace android
{
    class MotionService : public BnMotionService, public ServiceObject
    {
    public:
        MotionService(const void * token, int len);
        virtual ~MotionService();
    public:
        virtual int setCompletionListener(const sp<ICompletionListener> & onCompletionListener);
        virtual int setting(int id, int cmd, int arg1, int arg2, const sp<IResultListener> & onResultListener);
        virtual int stop(int session_id = 0, int bodyPart = 0, int arg1 = 0);
        virtual int reset(int id = 0);
        virtual int doAction(int id, int repeat, int duration, int flags = 0);
        virtual int run(const void * pdata, int len, int duration = 0, int flags = 0x01);
        virtual int runMotor(int id, int angle, int duration/*msec*/, int flags);
        virtual int walk(int distance/*cm*/, int speed, int flags);
        virtual int walkTo(int target, int speed, int flags);
        virtual int turn(int angle, int speed, int flags);
        virtual int emoji(int id, int duration, int repeat, int flags);
    protected:
        virtual  void release();
    private:
        MotorMotion motorMotion;
    };

    /**********************************************************************************/

}; /* namespace android */

#endif /* MotionService_H___ */
