#include <stdint.h>
#include <sys/types.h>

#include <binder/Parcel.h>
#include <utils/String8.h>
#include "RDSLog.h"
#include "IResultListener.h"

namespace android 
{
    enum
    {
        ONRESULTCOMPLETION = IBinder::FIRST_CALL_TRANSACTION,
    };

    class BpResultListener: public BpInterface<IResultListener>
    {
        public:
            BpResultListener(const sp<IBinder>& impl)
                : BpInterface<IResultListener>(impl)
            {
            }
            virtual int onCompletion(int id, int result)
            {
                Parcel data, reply;
                data.writeInterfaceToken(IResultListener::getInterfaceDescriptor());
                data.writeInt32(id);
                data.writeInt32(result);
                
                status_t err = remote()->transact(ONRESULTCOMPLETION, data, &reply);
                
                return (err == NO_ERROR ? 0 : -1);
            }
    };

    IMPLEMENT_META_INTERFACE(ResultListener, "inf.rds.result.listener");

    /**********************************************************************/

    status_t BnResultListener::onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
    {
        switch (code) {
            case ONRESULTCOMPLETION:
                {
                    CHECK_INTERFACE(IResultListener, data, reply);
                    int id = data.readInt32();
                    int result = data.readInt32();

                    reply->writeInt32(onCompletion(id, result));
                    return NO_ERROR;
                } 
                break;
        }
        return BBinder::onTransact(code, data, reply, flags);
    }

    /***************************************************************************/

}; /* namespace android */
