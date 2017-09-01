/*
*/

#include <stdint.h>
#include <sys/types.h>

#include <binder/Parcel.h>
#include <utils/String8.h>
#include "IRawCommandService.h"

namespace android 
{
    enum
    {
        RUN_SEND = IBinder::FIRST_CALL_TRANSACTION,
        RUN_CMDSET,
    };

    class BpRawCommandService: public BpInterface<IRawCommandService>
    {
        public:
            BpRawCommandService(const sp<IBinder>& impl)
                : BpInterface<IRawCommandService>(impl)
            {
            }
            int send(int dest, const void * streams, int len)
            {
                Parcel data, reply;
                data.writeInterfaceToken(IRawCommandService::getInterfaceDescriptor());
                data.writeInt32(dest);
                data.writeInt32(len);
                data.write(streams, len);
                remote()->transact(RUN_SEND, data, &reply);
                return reply.readInt32();
            }
            int run(const void * pdata, int len)
            {
                Parcel data, reply;
                data.writeInterfaceToken(IRawCommandService::getInterfaceDescriptor());
                data.writeInt32(len);
                data.write(pdata, len);
                remote()->transact(RUN_CMDSET, data, &reply);

                return reply.readInt32();
            }

    };

    IMPLEMENT_META_INTERFACE(RawCommandService, "inf.rds.rawcommand.service");

    /**********************************************************************/

    status_t BnRawCommandService::onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
    {
        switch (code) {
            case RUN_SEND:
                {
                    CHECK_INTERFACE(IRawCommandService, data, reply);

                    int dst = data.readInt32();
                    int len = data.readInt32();
                    const void *pdata = data.readInplace(len);                  

                    reply->writeInt32(send(dst, pdata, len));
                    return NO_ERROR;
                }
                break;
            case RUN_CMDSET:
                {
                    CHECK_INTERFACE(IRawCommandService, data, reply);

                    int len = data.readInt32();
                    const void *pdata = data.readInplace(len);
                    //int flag = data.readInt32();
                    int result = run(pdata, len);
                    reply->writeInt32(result);
                    return NO_ERROR;
                }
                break;
        }
        return BBinder::onTransact(code, data, reply, flags);
    }
    /***************************************************************************/

}; /* namespace android */
