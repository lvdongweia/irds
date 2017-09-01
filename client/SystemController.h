/*
 */

#ifndef SYSTEMCONTROLLER_CLIENT_H_
#define SYSTEMCONTROLLER_CLIENT_H_

#include <utils/Mutex.h>
#include <pthread.h>
#include <utils/threads.h>
#include "IDeathNotifier.h"
#include "ICompletionListener.h"
#include "IMessageListener.h"
#include "ISystemController.h"

using namespace android;

class SystemController : public RefBase
{
    private:
        class DeathNotifier : public IDeathNotifier
        {
        public:
            DeathNotifier(SystemController * ctx);
            virtual void died();
        private:
            SystemController * ctx;
        };
    private:
        static const char * token;
        sp<IDeathNotifier> mDeathNotifier;
        sp<ISystemController> systemController;
        sp<IMessageListener> mListener;
    public:
        SystemController();
        virtual ~SystemController();
    private:
        int prepare();

    public:
        virtual int setMessageListener(sp<IMessageListener> listener);
        virtual int send(int to, int cmd, int arg1, int arg2, const sp<ICompletionListener> & onCompletionListener);
};

#endif//
