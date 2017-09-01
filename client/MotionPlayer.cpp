/*
 */

#include <utils/Mutex.h>
#include <pthread.h>
#include <utils/threads.h>
#include "IRDService.h"
#include "MotionPlayer.h"
#include "RDSLog.h"

using namespace android;

const char * MotionPlayer::token = "5C34D63D-E8F4-4070-957B-A80868721067";

MotionPlayer::DeathNotifier::DeathNotifier(MotionPlayer * ctx)
{
    this->ctx = ctx;
}
void MotionPlayer::DeathNotifier::died()
{
    if (ctx->mListener.get() != 0)
    {
        ctx->mListener->onCompletion(0, 0, 0xFE);
        LOGE("###", "IRDS died");
    }
}

MotionPlayer::MotionPlayer()
{
    fileName = NULL;
    offset = 0;
    size = 0;
    motionPlayer = NULL;
    mListener = NULL;
    mDeathNotifier = new DeathNotifier(this);
}
MotionPlayer::~MotionPlayer()
{
    mDeathNotifier = NULL;
    motionPlayer = NULL;
    mListener = NULL;
    if (fileName != NULL)
    {
        free(fileName);
    }
    fileName = NULL;
}
int MotionPlayer::setCompletionListener(const sp<ICompletionListener> & onCompletionListener)
{
	mListener = onCompletionListener;
	if (motionPlayer.get() != 0) motionPlayer->setCompletionListener(onCompletionListener);
    return 0;
}
int MotionPlayer::setDataSource(const char * path)
{
    return setDataSource(path, 0, 0);
}

int MotionPlayer::setDataSource(const char * path, int offset, int size)
{
    int len = strlen(path);

    if (fileName != NULL)
    {
        free(fileName);
    }
    fileName = (char*)malloc(len + 1);
    memset(fileName, 0, len + 1);
    strcpy(fileName, path);
    this->offset = offset;
    this->size = size;
    return 0;
}
int MotionPlayer::prepare()
{
    const sp<IRDService>& service = IDeathNotifier::GetRDService(2); //tiemout 2 sec;
    if (service.get() == 0)
    {
        return -1;
    }
    if (motionPlayer.get() == 0)
    {
        motionPlayer = service->getMotionPlayer(token, strlen(token));
    }
    if (motionPlayer.get() == 0)
    {
        return -1;
    }
    if (mListener.get() != 0) motionPlayer->setCompletionListener(mListener);
    if (fileName != NULL) motionPlayer->setDataSource(fileName, offset, size);
    int result = motionPlayer->prepare();
    return result;
}
int MotionPlayer::start()
{
    if (motionPlayer.get() != 0) 
    {
        return motionPlayer->start();
    }
    return 0;     
}
int MotionPlayer::pause()
{
    if (motionPlayer.get() != 0) 
    {
        return motionPlayer->pause();
    }
    return 0;     
}
int MotionPlayer::resume()
{
    if (motionPlayer.get() != 0) 
    {
        return motionPlayer->resume();
    }
    return 0;     
}
int MotionPlayer::stop()
{
    if (motionPlayer.get() != 0) 
    {
        return motionPlayer->stop();
    }
    return 0;     
}
int MotionPlayer::getDuration()
{
    if (motionPlayer.get() != 0) 
    {
        return motionPlayer->getDuration();
    }
    return 0;     
}
int MotionPlayer::getPosition()
{
    if (motionPlayer.get() != 0) 
    {
        return motionPlayer->getPosition();
    }
    return 0;     
}
int MotionPlayer::setPosition(int pos)
{
    if (motionPlayer.get() != 0) 
    {
        return motionPlayer->setPosition(pos);
    }
    return 0;     
}


