
/*
*/

#include <stdint.h>
#include <sys/types.h>

#include <binder/Parcel.h>
#include <utils/String8.h>

#include "MotionPlayer.h"
#include "RDService.h"
#include "ArmFile.h"
#include "RDSLog.h"

namespace android 
{
	bool MotionPlayer::bEnableRun = true;
    MotionPlayer::MotionPlayer(const void * token, int len) : ServiceObject(token, len)
    {
        motionFrames = NULL;
        currentFrame = 0;
        currentPosition = 0;
        seekToPosition = 0;
        status = UNREADY;

        RDService::Instance()->addFrameListener(this);
    }
    MotionPlayer::~MotionPlayer()
    {
        clear();
    }
    void MotionPlayer::clear()
    {
        status = UNREADY;

        Mutex::Autolock lock(playLock);
        RDService::Instance()->removeFrameListener(this);

        this->onCompletionListener = 0;

        if (motionFrames != NULL)
        {
            delete motionFrames;
        }
        motionFrames = NULL;
    }
    void MotionPlayer::release()
    {
        LOGE("#################################", "motionplayer release");
        isUnset = true;
    }
    int MotionPlayer::update(int timeSincelastFrame)
    {
        if (status != RUNNING || motionFrames == NULL)
        {
            return 0;
        }
        Mutex::Autolock lock(playLock);
        if (currentFrame < 0 || currentFrame > (int)motionFrames->frameList.size())
        {
        	 status = UNREADY;
            if (this->onCompletionListener.get() != 0) onCompletion(0, status, -2);       
            return -1;
        }
        if (currentFrame == (int)motionFrames->frameList.size())
        {
            status = OVER;
            LOGE("###", "player finish->countframe:%d", currentFrame);
            if (this->onCompletionListener.get() != 0)
            {
            	  onCompletion(0, status, 0);
            }
            return 0;
        }
        MotionFrames::Frame * frame =  motionFrames->frameList[currentFrame];
        if (frame != NULL && currentPosition >= frame->timePosition)
        {
            //LOGE("###", "start run action, frame:%d time:%d frame:%p", currentFrame, currentPosition, frame->commandList);
            int duration = frame->duration == 0 ? motionFrames->frameTime : frame->duration; 

            switch (motionFrames->Rhythm)
            {
                case 0: //Soothing
                    break;
                case 1: //Normal
                    duration = frame->runTime == 0 ? duration : frame->runTime;
                    break;
                case 2: //Strong
                    duration = 100;
                    break;
                case 3: //custom
                    duration = 0;
                    break;
            }
            //LOGE("#####", " Run motor duration:%d", duration);
            if (bEnableRun && motorMotion.Run((const unsigned char *)frame->commandList, frame->commandLength, duration) < 0)
            {
                if (this->onCompletionListener.get() != 0)
                {
                    onCompletion(0, status, -3);
                }
            }
            currentFrame++;
        }
        if (frame == NULL)
        {
            currentFrame++;
        }
        currentPosition += timeSincelastFrame; 
        return 0;
    }
    int MotionPlayer::isSafely()
    {
        return 1;
    }
    int MotionPlayer::setCompletionListener(const sp<ICompletionListener> & onCompletionListener)
    {
        Mutex::Autolock lock(playLock);
        this->onCompletionListener = onCompletionListener;
        return 0;
    }
    int MotionPlayer::setDataSource(const char * path, int offset, int size)
    {
        Mutex::Autolock lock(playLock);
        status = UNREADY;

        if (path == NULL || offset < 0 || size == 0)
        {
            return -1;
        }
        int len = strlen(path);
        LOGE("### ", "setDataSource path:%s offset:%d, size:%d", path == NULL ? "null" : path, offset, size);
        if (motionFrames != NULL)
        {
            delete motionFrames;
            motionFrames = NULL;
        }
        motionFrames = ArmFile::LoadFromFile(path, offset, size);
        return 0;
    }

    int MotionPlayer::setDataSourceByStream(const unsigned char * bytes, int size)
    {
        Mutex::Autolock lock(playLock);
        status = UNREADY;
        if (motionFrames != NULL)
        {
            delete motionFrames;
            motionFrames = NULL;
        }
        motionFrames = ArmFile::LoadFromStream(bytes, size);
        return 0;
    }
    int MotionPlayer::prepare()
    {
        Mutex::Autolock lock(playLock);
        status = UNREADY;

        if (motionFrames == NULL)
        {
            LOGE("###", " ArmFile::Load error");
            return -1;
        }
        currentFrame = 0;
        currentPosition = 0;
        seekToPosition = 0;
        status = READY;
        if (this->onCompletionListener.get() != 0)
        {
        	 onCompletion(0, status, 0);
        }
        return 0;
    }
    int MotionPlayer::start()
    {
        currentFrame = 0;
        currentPosition = 0;
        seekToPosition = 0;

        if (status != READY || motionFrames == NULL)
        {
            return -1;
        }
        status = RUNNING;
        return 0;
    }
    int MotionPlayer::pause()
    {
        if (status == RUNNING)
        {
            status = PAUSE;
        }
        return 0; 
    }
    int MotionPlayer::resume()
    {
        if (status == PAUSE)
        {
            status = RUNNING;
        }
        return 0;
    }
    int MotionPlayer::stop()
    {
        status = READY;
        return 0;
    }
    int MotionPlayer::getDuration()
    {
        if (motionFrames == NULL)
        {
            return 0;
        }
        return motionFrames->duration;
    }
    int MotionPlayer::getPosition()
    {
        return currentPosition;
    }
    int MotionPlayer::setPosition(int pos)
    {
        MotionFrames::Frame * frame =  NULL;
		int i = 0;
		currentFrame = -1;
		for(;i < motionFrames->frameList.size(); i++)
		{
			frame =  motionFrames->frameList[i];
			if (frame != NULL && (pos / 1000) >= (frame->timePosition / 1000))
			{
				currentFrame = i;
				currentPosition = frame->timePosition;
			}
		}
		if (currentFrame == -1)
		{
			stop();
		}
        seekToPosition = pos;
        return currentFrame;
    }
	int MotionPlayer::setEnableRun(int bEnable)
	{
		bEnableRun = (bEnable == 0) ? false : true;
		return 0;
	}
	int MotionPlayer::getRunStatus()
	{
		return ((bEnableRun == true) ? 1 : 0);
	}
}; /* namespace android */
