/*
*/

#ifndef MotionPlayer_H__
#define MotionPlayer_H__

#include <utils/Log.h>
#include <utils/String8.h>
#include <utils/Mutex.h>

#include "IMotionPlayer.h"
#include "ServiceObject.h"
#include "IFrameListener.h"
#include "MotionFrames.h"

namespace android
{
    class MotionPlayer : public BnMotionPlayer, public ServiceObject, public IFrameListener
    {
    public:
        enum STATUS
        {
            UNREADY = 0,
            OVER,
            READY,
            PAUSE,
            RUNNING,
        };
    public:
        MotionPlayer(const void * token, int len);
        virtual ~MotionPlayer();
    protected:
        virtual void release();
    public:
        virtual int update(int timeSincelastFrame);
        virtual int isSafely();
        virtual int setCompletionListener(const sp<ICompletionListener> & onCompletionListener);
        virtual int setDataSource(const char * path, int offset, int size);
        virtual int setDataSourceByStream(const unsigned char * bytes, int size);
        virtual int prepare();
        virtual int start();
        virtual int pause();
        virtual int resume();
        virtual int stop();
        virtual int getDuration();
        virtual int getPosition();
        virtual int setPosition(int pos);
		 virtual int setEnableRun(int bEnable);
		 virtual int getRunStatus();
    private:
        void clear();
    private:
		 static bool bEnableRun;
        MotorMotion motorMotion;
        MotionFrames * motionFrames;
    private:
        STATUS status; //
        Mutex playLock;
        int currentFrame;
        int currentPosition;
        int seekToPosition;
    };

    /**********************************************************************************/

}; /* namespace android */

#endif /* MotionPlayer_H___ */
