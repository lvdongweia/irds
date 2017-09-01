/*
 */
#include <utils/Mutex.h>
#include <pthread.h>
#include <utils/threads.h>
#include "IRDService.h"
#include "SystemController.h"
#include "RDSLog.h"

using namespace android;

const char * SystemController::token ="665AF541-9E60-4228-9E60-E95807FA2E74";

SystemController::DeathNotifier::DeathNotifier(SystemController * ctx)
{
    this->ctx = ctx;
}
void SystemController::DeathNotifier::died()
{
    if (ctx->mListener.get() != 0)
    {
        ctx->mListener->onMessage(0xFF, 0x8FFE, 0, 0);
    }
}

SystemController::SystemController()
{
    systemController = NULL;
    mListener = NULL;
    mDeathNotifier = new DeathNotifier(this);

    prepare();
}
SystemController::~SystemController()
{
    mDeathNotifier = NULL;
    systemController = NULL;
    mListener = NULL;
}

int SystemController::prepare()
{
    const sp<IRDService>& service = IDeathNotifier::GetRDService(2); //tiemout 2 sec;
    if (service.get() == 0)
    {
        return -1;
    }
    systemController = service->getSystemController(token, strlen(token));
    if (systemController.get() == 0)
    {
        return -1;
    }
    return 0;
}
int SystemController::setMessageListener(sp<IMessageListener> listener)
{
	mListener = listener;
    if (systemController.get() != 0)  systemController->setMessageListener(listener);
    return 0;
}

int SystemController::send(int to, int cmd, int arg1, int arg2, const sp<ICompletionListener> & onCompletionListener)
{
    if (systemController.get() != 0) return systemController->send(to, cmd, arg1, arg2, onCompletionListener);

    return -1;   
}



