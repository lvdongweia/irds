
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
#include "RDSLog.h"
#include "client/MotionPlayer.h"
#include "client/MotionService.h"

#include <utils/threads.h>

using namespace android;
sp<IRDService> sService;

char hello[] = "Hello binder";

static sp<MotionPlayer> motionPlayer = new MotionPlayer();

class TaskCompletionListener : public BnCompletionListener
{
    public:
        virtual int onCompletion(int session_id,int result, int errorcode)
        {
            LOGE("###", "task completioin :%d", result);
            return 0;
        }
};

int main(int argc, char **argv)
{
    int ret = 0;

    ProcessState::self()->startThreadPool();

    /*
    MotionPlayer player;
    //player.setListener(new MotionPlayerListener());
    player.setDataSource("/sdcard/media/1.arm", 0, 1091);
    player.prepare();
    player.start();
    */
    sp<MotionService> motionService = new MotionService();
    motionService->setCompletionListener(new TaskCompletionListener());
    motionService->runMotor(1, 100, 1000, 1);
    printf("test motor => motor id:%d turn angle:%d\n", 1, 100);
    sleep(2);
    motionService->walk(500, 1, 0);
    printf("test walk\n");

    sleep(2);
    motionService->turn(60, 1);
    printf("test turn\n");

    sleep(2);
    motionService->stop(0);
    getchar();
    return ret;
}
