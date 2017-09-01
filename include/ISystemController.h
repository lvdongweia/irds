#ifndef ISystemController_H__
#define ISystemController_H__

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

#include "IServiceInterface.h"

namespace android
{
    class IMessageListener;
    class ICompletionListener;
    class ISystemController : public IServiceInterface
    {
        public:
            DECLARE_META_INTERFACE(SystemController);
            virtual int setMessageListener(const sp<IMessageListener> & onMessageListener) = 0;
            virtual int send(int to, int cmd, int arg1, int arg2, const sp<ICompletionListener> & onCompletionListener = 0) = 0; 
    };

    /*************************************************************************/

    class BnSystemController : public BnInterface<ISystemController>
    {
        public:
            virtual status_t  onTransact( uint32_t code, const Parcel& data,  Parcel* reply, uint32_t flags = 0);
    };

}; /* namespace android */

#endif /* ISystemController_H__ */

