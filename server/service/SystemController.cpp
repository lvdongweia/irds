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
#include "cutils/properties.h"

#define TOUCH_LISTENER "persist.sys.touch_listener"

namespace android 
{
    bool SystemController::bTouchListener = true;
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
    int SystemController::setEnable(int deviceType, int deviceId, int enable, const sp<ICompletionListener> & onCompletionListener)
    {
        bTouchListener = (enable == 0 ? false : true);
        int result = property_set(TOUCH_LISTENER, enable ? "true":"false");

        if(onCompletionListener.get() != 0)
            onCompletionListener->onCompletion(0, result, 0);

        return 0;
    }
    int SystemController::isEnable(int deviceType, int deviceId, const sp<ICompletionListener> & onCompletionListener)
    {
        if(onCompletionListener.get() != 0)
            onCompletionListener->onCompletion(1, bTouchListener == true ? 1 : 0, 0);

        return 1;
    }
    int SystemController::setting(int id, int cmd, int arg1, int arg2, const sp<ICompletionListener> & onCompletionListener)
    {
        SettingObject * so = new SettingObject(uid);
        so->callback = onCompletionListener;
        return so->Run(id, cmd, arg1, arg2, NULL);
    }
    void SystemController::checkAllListener()
    {
        char buf[PROPERTY_VALUE_MAX] = {0};
        int len = property_get(TOUCH_LISTENER, buf, "");
        if(len > 1)
        {
            if(!strcmp(buf, "false"))
            {
                bTouchListener = false;
            }
        }
    }
    void SystemController::release()
    {
        isUnset = true;
    }
}; /* namespace android */
