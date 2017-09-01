/*
*/

#include <stdint.h>
#include <sys/types.h>

#include <binder/Parcel.h>

#include "RDSLog.h"
#include "ITopic.h"

namespace android 
{
    enum
    {
        RECV_NOTIFY = IBinder::FIRST_CALL_TRANSACTION,
        GET_TOPICID,
        GET_CLSID,
    };

    class BpTopic: public BpInterface<ITopic>
    {
        public:
            BpTopic(const sp<IBinder>& impl)
                : BpInterface<ITopic>(impl)
            {
            }
            virtual int onNotify(int priority, int src_id, const void *pdata, int len)
            {
                Parcel data, reply;
                data.writeInterfaceToken(ITopic::getInterfaceDescriptor());
                data.writeInt32(priority);
                data.writeInt32(src_id);
                data.writeInt32(len);
                data.write((const void * )pdata, len);
                status_t err = remote()->transact(RECV_NOTIFY, data, &reply);
                
                return (err == NO_ERROR ? 0 : -1);
            }
            virtual int getTopicID()
            {
                Parcel data, reply;
                data.writeInterfaceToken(ITopic::getInterfaceDescriptor());
                status_t err = remote()->transact(GET_TOPICID, data, &reply);
                
                if (err == NO_ERROR)
                {
                    return reply.readInt32();
                }
                return 0;
            }
            virtual int getClsID()
            {
                Parcel data, reply;
                data.writeInterfaceToken(ITopic::getInterfaceDescriptor());
                status_t err = remote()->transact(GET_CLSID, data, &reply);
                
                if (err == NO_ERROR)
                {
                    return reply.readInt32();
                }
                return 0;
            }
    };

    IMPLEMENT_META_INTERFACE(Topic, "inf.rds.topic");

    /**********************************************************************/

    status_t BnTopic::onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
    {
        switch (code) {
            case RECV_NOTIFY:
                {
                    CHECK_INTERFACE(ITopic, data, reply);

                    int priority = data.readInt32();
                    int src_id = data.readInt32();
                    int len = data.readInt32();
                    const void *pdata = data.readInplace(len);                  
                    reply->writeInt32(onNotify(priority, src_id, pdata, len));
                    return NO_ERROR;
                } 
                break;
            case GET_TOPICID:
                {
                    CHECK_INTERFACE(ITopic, data, reply);
                    reply->writeInt32(getTopicID());
                    return NO_ERROR;
                } 
                break;
            case GET_CLSID:
                {
                    CHECK_INTERFACE(ITopic, data, reply);
                    reply->writeInt32(getClsID());
                    return NO_ERROR;
                } 
                break;
        }
        return BBinder::onTransact(code, data, reply, flags);
    }
    int BnTopic::getClsID()
    {
        return (int)this;
    }
    /***************************************************************************/

}; /* namespace android */

