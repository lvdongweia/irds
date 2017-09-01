/*
 */

#ifndef MOTIONPLAYER_CLIENT_H_
#define MOTIONPLAYER_CLIENT_H_

#include <utils/Mutex.h>
#include <pthread.h>
#include <utils/threads.h>
#include "IDeathNotifier.h"

#include "ICompletionListener.h"
#include "IServiceListener.h"
#include "IMotionPlayer.h"

using namespace android;

class MotionPlayer : public RefBase
{
    private:
        class DeathNotifier : public IDeathNotifier
        {
        public:
            DeathNotifier(MotionPlayer * ctx);
            virtual void died();
        private:
            MotionPlayer * ctx;
        };
    private:
        static const char * token;
        sp<IDeathNotifier> mDeathNotifier;
        sp<IMotionPlayer> motionPlayer;
        sp<ICompletionListener> mListener;

        char * fileName;
        int offset;
        int size;
    public:
        MotionPlayer();
        virtual ~MotionPlayer();
    public:
        int setCompletionListener(const sp<ICompletionListener> & onCompletionListener);
        int setDataSource(const char * path);
        int setDataSource(const char * path, int offset, int size);
        int prepare();
        int start();
        int pause();
        int resume();
        int stop();
        int  getDuration();
        int  getPosition();
        int setPosition(int pos);
};

#endif//
