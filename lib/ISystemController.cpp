/*
*/

#include <stdint.h>
#include <sys/types.h>

#include <binder/Parcel.h>
#include <utils/String8.h>

#include "ICompletionListener.h"
#include "IMessageListener.h"
#include "ISystemController.h"
#include "RDSLog.h"

namespace android 
{
    enum
    {
        SET_LISTENER = IBinder::FIRST_CALL_TRANSACTION,
        RUN_SEND,
        RELEASE,
        SET_ENABLE,
        IS_ENABLE,
        SETTING,
    };

    class BpSystemController : public BpInterface<ISystemController>
    {
        public:
            BpSystemController(const sp<IBinder>& impl)
                : BpInterface<ISystemController>(impl)
            {
            }
            virtual ~BpSystemController()
            {
                release();
            }
            int setMessageListener(const sp<IMessageListener> & onMessageListener)
            {
                int flag = onMessageListener.get() == 0 ? 0 : 1;
                Parcel data, reply;
                data.writeInterfaceToken(ISystemController::getInterfaceDescriptor());
                data.writeInt32(flag);
                if (flag == 1)
                {
                    data.writeStrongBinder(onMessageListener->asBinder());
                }
                remote()->transact(SET_LISTENER, data, &reply);
                return reply.readInt32();
            }
            int send(int to, int cmd, int arg1, int arg2, const sp<ICompletionListener> & onCompletionListener)
            {
                int flag = onCompletionListener.get() == 0 ? 0 : 1;
                Parcel data, reply;
                data.writeInterfaceToken(ISystemController::getInterfaceDescriptor());
                data.writeInt32(to);
                data.writeInt32(cmd);
                data.writeInt32(arg1);
                data.writeInt32(arg2);
                data.writeInt32(flag);
                if (flag == 1)
                {
                    data.writeStrongBinder(onCompletionListener->asBinder());
                }
                remote()->transact(RUN_SEND, data, &reply);
                return reply.readInt32();
            }
            void release()
            {
                Parcel data, reply;
                data.writeInterfaceToken(ISystemController::getInterfaceDescriptor());
                remote()->transact(RELEASE, data, &reply);
            }
            int setEnable(int deviceType, int deviceId, int enable, const sp<ICompletionListener> & onCompletionListener)
            {
                int flag = onCompletionListener.get() == 0 ? 0 : 1;
                Parcel data, reply;
                data.writeInterfaceToken(ISystemController::getInterfaceDescriptor());
                data.writeInt32(deviceType);
                data.writeInt32(deviceId);
                data.writeInt32(enable);
                data.writeInt32(flag);
                if (flag == 1)
                {
                    data.writeStrongBinder(onCompletionListener->asBinder());
                }
                remote()->transact(SET_ENABLE, data, &reply);
                return reply.readInt32();
            }
            int isEnable(int deviceType, int deviceId, const sp<ICompletionListener> & onCompletionListener)
            {
                int flag = onCompletionListener.get() == 0 ? 0 : 1;
                Parcel data, reply;
                data.writeInterfaceToken(ISystemController::getInterfaceDescriptor());
                data.writeInt32(deviceType);
                data.writeInt32(deviceId);
                data.writeInt32(flag);
                if (flag == 1)
                {
                    data.writeStrongBinder(onCompletionListener->asBinder());
                }
                remote()->transact(IS_ENABLE, data, &reply);
                return reply.readInt32();
            }
            int setting(int id, int cmd, int arg1, int arg2, const sp<ICompletionListener> & onCompletionListener)
            {
                int flag = onCompletionListener.get() == 0 ? 0 : 1;
                Parcel data, reply;
                data.writeInterfaceToken(ISystemController::getInterfaceDescriptor());
                data.writeInt32(id);
                data.writeInt32(cmd);
                data.writeInt32(arg1);
                data.writeInt32(arg2);
                data.writeInt32(flag);
                if (flag == 1)
                {
                    data.writeStrongBinder(onCompletionListener->asBinder());
                }
                remote()->transact(SETTING, data, &reply);
                return reply.readInt32();
            }
    };

    IMPLEMENT_META_INTERFACE(SystemController, "inf.rds.systemcontroller.service");

    /**********************************************************************/

    status_t BnSystemController::onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
    {
        switch (code) {
            case SET_LISTENER:
                {
                    CHECK_INTERFACE(ISystemController, data, reply);
                    int flag = data.readInt32();
                    int result = setMessageListener(flag == 1 ? interface_cast<IMessageListener>(data.readStrongBinder()) : 0);
                    reply->writeInt32(result);
                    return NO_ERROR;
                }
                break;
            case RUN_SEND:
                {
                    CHECK_INTERFACE(ISystemController, data, reply);
                    int to = data.readInt32();
                    int cmd = data.readInt32();
                    int arg1 = data.readInt32();
                    int arg2 = data.readInt32();
                    int flag = data.readInt32();
                    reply->writeInt32(send(to, cmd, arg1, arg2, flag == 1 ? interface_cast<ICompletionListener>(data.readStrongBinder()) : 0));
                    return NO_ERROR;
                }
                break;
            case RELEASE:
                {
                    CHECK_INTERFACE(ISystemController, data, reply);
                    release();
                    return NO_ERROR;
                }
                break;
            case SET_ENABLE:
            {
                CHECK_INTERFACE(ISystemController, data, reply);
                int deviceType = data.readInt32();
                int deviceId = data.readInt32();
                int enable = data.readInt32();
                int flag = data.readInt32();
                int result = setEnable(deviceType, deviceId, enable, flag == 1 ? interface_cast<ICompletionListener>(data.readStrongBinder()) : 0);
                reply->writeInt32(result);
                return NO_ERROR;
            }
            break;
            case IS_ENABLE:
            {
                CHECK_INTERFACE(ISystemController, data, reply);
                int deviceType = data.readInt32();
                int deviceId = data.readInt32();
                int flag = data.readInt32();
                int result = isEnable(deviceType, deviceId, flag == 1 ? interface_cast<ICompletionListener>(data.readStrongBinder()) : 0);
                reply->writeInt32(result);
                return NO_ERROR;
            }
            break;
            case SETTING:
            {
                CHECK_INTERFACE(ISystemController, data, reply);
                int id = data.readInt32();
                int cmd = data.readInt32();
                int arg1 = data.readInt32();
                int arg2 = data.readInt32();
                int flag = data.readInt32();
                int result = setting(id, cmd, arg1, arg2, flag == 1 ? interface_cast<ICompletionListener>(data.readStrongBinder()) : 0);
                reply->writeInt32(result);
                return NO_ERROR;
            }
            break;
        }
        return BBinder::onTransact(code, data, reply, flags);
    }
    /***************************************************************************/

}; /* namespace android */
