/*
*/

#include <stdint.h>
#include <sys/types.h>

#include <binder/Parcel.h>
#include <utils/String8.h>

#include "IMessageListener.h"
#include "RDSLog.h"

namespace android 
{

    enum
    {
        ONMESSAGE = IBinder::FIRST_CALL_TRANSACTION,
    };

    class BpMessageListener: public BpInterface<IMessageListener>
    {
        public:
            BpMessageListener(const sp<IBinder>& impl)
                : BpInterface<IMessageListener>(impl)
            {
            }
            virtual int onMessage(int from, int what, int arg1, int arg2)
            {
                Parcel data, reply;
                data.writeInterfaceToken(IMessageListener::getInterfaceDescriptor());
                data.writeInt32(from);
                data.writeInt32(what);
                data.writeInt32(arg1);
                data.writeInt32(arg2);
                
                status_t err = remote()->transact(ONMESSAGE, data, &reply);
                
                return (err == NO_ERROR ? 0 : -1);
            }
    };

    IMPLEMENT_META_INTERFACE(MessageListener, "inf.rds.message.listener");

    /**********************************************************************/

    status_t BnMessageListener::onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
    {
        switch (code) {
            case ONMESSAGE:
                {
                    CHECK_INTERFACE(IMessageListener, data, reply);
                    int from = data.readInt32();
                    int what = data.readInt32();
                    int arg1 = data.readInt32();
                    int arg2 = data.readInt32();
                    reply->writeInt32(onMessage(from, what, arg1, arg2));
                    return NO_ERROR;
                } 
                break;
        }
        return BBinder::onTransact(code, data, reply, flags);
    }

}; /* namespace android */
