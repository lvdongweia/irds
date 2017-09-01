/*
*/


#include <stdint.h>
#include <sys/types.h>

#include <binder/Parcel.h>
#include <utils/String8.h>
#include "IMessageListener.h"
#include "SystemController.h"
#include "RDSTask.h"
#include "RDService.h"
#include "RDSLog.h"

namespace android 
{
    SystemController::SystemController(const void * token, int len) : ServiceObject(token, len)
    {
    }
    SystemController::~SystemController()
    {
    }
    int SystemController::setMessageListener(const sp<IMessageListener> & onMessageListener)
    {
        this->listener = onMessageListener;
        return 0;
    }
    int SystemController::send(int to, int cmd, int arg1, int arg2, const sp<ICompletionListener> & onCompletionListener)
    {
        LOGE("###", "system controller recevie message to:%d, cmd:%d arg1:%d arg2:%d", to, cmd, arg1, arg2);
        ControllerObject * co = new ControllerObject(uid);
        return co->Run(to, cmd, arg1, arg2, onCompletionListener);
    }
    void SystemController::release()
    {
        isUnset = true;
    }
}; /* namespace android */
