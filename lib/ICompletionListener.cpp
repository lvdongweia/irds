
/*
*/

#include <stdint.h>
#include <sys/types.h>

#include <binder/Parcel.h>
#include <utils/String8.h>
#include "RDSLog.h"
#include "ICompletionListener.h"

namespace android 
{
    enum
    {
        ONCOMPLETION = IBinder::FIRST_CALL_TRANSACTION,
    };

    class BpCompletionListener: public BpInterface<ICompletionListener>
    {
        public:
            BpCompletionListener(const sp<IBinder>& impl)
                : BpInterface<ICompletionListener>(impl)
            {
            }
            virtual int onCompletion(int session_id, int result, int errorcode)
            {
                Parcel data, reply;
                data.writeInterfaceToken(ICompletionListener::getInterfaceDescriptor());
                data.writeInt32(session_id);
                data.writeInt32(result);
                data.writeInt32(errorcode);
                
                status_t err = remote()->transact(ONCOMPLETION, data, &reply);
                
                return (err == NO_ERROR ? 0 : -1);
            }
    };

    IMPLEMENT_META_INTERFACE(CompletionListener, "inf.rds.completion.listener");

    /**********************************************************************/

    status_t BnCompletionListener::onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
    {
        switch (code) {
            case ONCOMPLETION:
                {
                    CHECK_INTERFACE(ICompletionListener, data, reply);
                    int session_id = data.readInt32();
                    int result = data.readInt32();
                    int errorcode = data.readInt32();

                    reply->writeInt32(onCompletion(session_id, result, errorcode));
                    return NO_ERROR;
                } 
                break;
        }
        return BBinder::onTransact(code, data, reply, flags);
    }

    /***************************************************************************/

}; /* namespace android */
