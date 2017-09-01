/*
 */
#ifndef MOTIONSERVICE_CLIENT_H_
#define MOTIONSERVICE_CLIENT_H_

#include <utils/Mutex.h>
#include <pthread.h>
#include <utils/threads.h>
#include "IDeathNotifier.h"

#include "ICompletionListener.h"
#include "IResultListener.h"
#include "IServiceListener.h"
#include "IMotionService.h"

using namespace android;

class MotionService : public RefBase
{
    private:
        class DeathNotifier : public IDeathNotifier
        {
        public:
            DeathNotifier(MotionService * ctx);
            virtual void died();
        private:
            MotionService * ctx;
        };
    private:
        static const char * token;
        sp<IDeathNotifier> mDeathNotifier;
        sp<IMotionService> motionService;
        sp<ICompletionListener> mListener;
    public:
        MotionService(bool bprepare = true);
        virtual ~MotionService();
        virtual int prepare();
        virtual int setting(int id, int cmd, int arg1, int arg2, const sp<IResultListener> & onResultListener);
        virtual int setCompletionListener(sp<ICompletionListener> listener);
        virtual int stop(int session_id = 0, int bodyPart = 0, int arg1 = 0);
        virtual int reset(int id = 0);
        virtual int doAction(int id, int repeat, int duration, int flags = 0);
        virtual int run(const void * pdata, int len, int duration, int flags = 0);
        virtual int runMotor(int id, int angle, int duration/*msec*/, int flags = 0);
        virtual int walk(int distance, int speed, int flags = 0);
        virtual int walkTo(int target, int speed, int flags = 0);
        virtual int turn(int angle, int speed, int flags = 0);
        virtual int emoji(int id, int duration, int repeat, int flags = 0);
    public:
};

#endif//
