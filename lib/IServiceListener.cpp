/*
*/

#include <stdint.h>
#include <sys/types.h>

#include <binder/Parcel.h>
#include <utils/String8.h>
#include "RDSLog.h"
#include "IServiceListener.h"


namespace android 
{
    enum
    {
        ONCHANGED = IBinder::FIRST_CALL_TRANSACTION,
    };

    class BpServiceListener: public BpInterface<IServiceListener>
    {
        public:
            BpServiceListener(const sp<IBinder>& impl)
                : BpInterface<IServiceListener>(impl)
            {
            }
            virtual int onChanged(int status)
            {
                Parcel data, reply;
                data.writeInterfaceToken(IServiceListener::getInterfaceDescriptor());
                data.writeInt32(status);
        		LOGE("###", "ServiceListener transact() %d", status);
                remote()->transact(ONCHANGED, data, &reply);
                return reply.readInt32();
            }
    };

    IMPLEMENT_META_INTERFACE(ServiceListener, "inf.rds.service.listener");

    /**********************************************************************/

    status_t BnServiceListener::onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
    {
        switch (code) {
            case ONCHANGED:
                {
                    CHECK_INTERFACE(IServiceListener, data, reply);

                    LOGE("###", "ServiceListner onTransact() begin read");
                    int status = data.readInt32();
                    LOGE("###", "ServiceListner onTransact() end read");

                    reply->writeInt32(onChanged(status));
                    return NO_ERROR;
                } 
                break;
        }
        return BBinder::onTransact(code, data, reply, flags);
    }
    BnServiceListener::~BnServiceListener()
    {
        LOGE("###", "Device Listener destroy");
    }

    /***************************************************************************/

}; /* namespace android */
