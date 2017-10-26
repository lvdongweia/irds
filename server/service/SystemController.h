/*
*/

#ifndef SystemController_H__
#define SystemController_H__

#include <utils/Log.h>
#include <utils/String8.h>

#include "ISystemController.h"
#include "ServiceObject.h"
#include "CommandObject.h"
#include "ControllerObject.h"

namespace android
{
    class SystemController : public BnSystemController, public ServiceObject
    {
    public:
        SystemController(const void * token, int len);
        virtual ~SystemController();
    public:
        virtual int setMessageListener(const sp<IMessageListener> & onMessageListener);
        virtual int send(int to, int cmd, int arg1, int arg2, const sp<ICompletionListener> & onCompletionListener);
        virtual int setEnable(int deviceType, int deviceId, int enable, const sp<ICompletionListener> & onCompletionListener);
        virtual int isEnable(int deviceType, int deviceId, const sp<ICompletionListener> & onCompletionListener);
        virtual int setting(int id, int cmd, int arg1, int arg2, const sp<ICompletionListener> & onCompletionListener);
        virtual void release();
        static void checkAllListener();
    public:
        sp<IMessageListener> listener;
        static bool bTouchListener;
    };

    /**********************************************************************************/

}; /* namespace android */

#endif /* SystemController_H__ */
