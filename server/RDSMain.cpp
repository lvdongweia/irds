/*
*/

#define LOG_TAG "AR_MOTION_SERVER"
#define LOG_NDEBUG 1

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <utils/Log.h>
#include "RDSLog.h"
#include "RDService.h"

using namespace android;

int main(int argc, char** argv)
{
    signal(SIGPIPE, SIG_IGN);
    sp<ProcessState> proc(ProcessState::self());
    sp<IServiceManager> sm = defaultServiceManager();
    RDService::Instance();
    LOGE("###", "irds service init ok");
    ProcessState::self()->startThreadPool();
    IPCThreadState::self()->joinThreadPool();
}
