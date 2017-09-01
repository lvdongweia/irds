#ifndef IMessageListener_H__
#define IMessageListener_H__

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

namespace android
{
    class Parcel;

    class IMessageListener: public IInterface
    {
        public:
            DECLARE_META_INTERFACE(MessageListener);

            virtual int onMessage(int from, int what, int arg1, int arg2) = 0;
    };

    /*************************************************************************/

    class BnMessageListener: public BnInterface<IMessageListener>
    {
        public:
            virtual status_t  onTransact( uint32_t code, const Parcel& data,  Parcel* reply, uint32_t flags = 0);
    };

}; /* namespace android */

#endif /* IMessageListener_H__ */

