/*
*/

#include <stdint.h>
#include <sys/types.h>

#include "ICompletionListener.h"
#include "IResultListener.h"
#include "IMotionService.h"
#include "RDSLog.h"

namespace android 
{
    enum
    {
        SET_LISTENER  = IBinder::FIRST_CALL_TRANSACTION,
        SETTING,
        RUN_STOP,
        RUN_RESET,
        RUN_ACTION,
        RUN_CMDSET,//more than one command or ran/arm file
        RUN_MOTOR,
        RUN_WALK,
        RUN_WALKTO,
        RUN_TURN,
        RUN_EMOJI,
        RELEASE,
    };

    class BpMotionService : public BpInterface<IMotionService>
    {
        public:
            BpMotionService(const sp<IBinder>& impl)
                : BpInterface<IMotionService>(impl)
            {
            }
            virtual ~BpMotionService()
            {
                release();
            }
            int setCompletionListener(const sp<ICompletionListener> & onCompletionListener)
            {
                int flag = onCompletionListener.get() == 0 ? 0 : 1;
                Parcel data, reply;
                data.writeInterfaceToken(IMotionService::getInterfaceDescriptor());
                data.writeInt32(flag);
                if (flag == 1)
                {
                    data.writeStrongBinder(onCompletionListener->asBinder());
                }
                remote()->transact(SET_LISTENER, data, &reply);
                return reply.readInt32();
            }
            int setting(int id, int cmd, int arg1, int arg2, const sp<IResultListener> & onResultListener)
            {
                int flag = onResultListener.get() == 0 ? 0 : 1;
                Parcel data, reply;
                data.writeInterfaceToken(IMotionService::getInterfaceDescriptor());
                data.writeInt32(id);
                data.writeInt32(cmd);
                data.writeInt32(arg1);
                data.writeInt32(arg2);
                data.writeInt32(flag);
                if (flag == 1)
                {
                    data.writeStrongBinder(onResultListener->asBinder());
                }
                remote()->transact(SETTING, data, &reply);
                return reply.readInt32();
            }

            int stop(int session_id, int bodyPart, int arg1)
            {
                Parcel data, reply;
                data.writeInterfaceToken(IMotionService::getInterfaceDescriptor());
                data.writeInt32(session_id);
                data.writeInt32(bodyPart);
                data.writeInt32(arg1);
                remote()->transact(RUN_STOP, data, &reply);
                return reply.readInt32();
            }
            int reset(int id)
            {
                Parcel data, reply;
                data.writeInterfaceToken(IMotionService::getInterfaceDescriptor());
                data.writeInt32(id);
                remote()->transact(RUN_RESET, data, &reply);
                return reply.readInt32();
            }
            int doAction(int id, int repeat, int duration, int flags)
            {
                Parcel data, reply;
                data.writeInterfaceToken(IMotionService::getInterfaceDescriptor());
                data.writeInt32(id);
                data.writeInt32(repeat);
                data.writeInt32(duration);
                data.writeInt32(flags);
                remote()->transact(RUN_ACTION, data, &reply);
                return reply.readInt32();
            }
            int runMotor(int id, int angle, int duration/*ms*/, int flags)
            {
                Parcel data, reply;
                data.writeInterfaceToken(IMotionService::getInterfaceDescriptor());
                data.writeInt32(id);
                data.writeInt32(angle);
                data.writeInt32(duration);
                data.writeInt32(flags);
                remote()->transact(RUN_MOTOR, data, &reply);
                return reply.readInt32();
            }
            int run(const void * pdata, int len, int duration, int flags)
            {
                Parcel data, reply;
                data.writeInterfaceToken(IMotionService::getInterfaceDescriptor());
                data.writeInt32(len);
                data.write(pdata, len);
                data.writeInt32(duration);
                data.writeInt32(flags);
                remote()->transact(RUN_CMDSET, data, &reply);

                return reply.readInt32();
            }
            int walk(int distance, int speed, int flags)
            {
                Parcel data, reply;
                data.writeInterfaceToken(IMotionService::getInterfaceDescriptor());
                data.writeInt32(distance);
                data.writeInt32(speed);
                data.writeInt32(flags);
                remote()->transact(RUN_WALK, data, &reply);
                return reply.readInt32();
            }
            int walkTo(int target, int speed, int flags)
            {
                Parcel data, reply;
                data.writeInterfaceToken(IMotionService::getInterfaceDescriptor());
                data.writeInt32(target);
                data.writeInt32(speed);
                data.writeInt32(flags);
                remote()->transact(RUN_WALKTO, data, &reply);
                return reply.readInt32();
            }
            int turn(int angle, int speed, int flags)
            {
                Parcel data, reply;
                data.writeInterfaceToken(IMotionService::getInterfaceDescriptor());
                data.writeInt32(angle);
                data.writeInt32(speed);
                data.writeInt32(flags);
                remote()->transact(RUN_TURN, data, &reply);
                return reply.readInt32();
            }
            int emoji(int id, int duration, int repeat, int flags)
            {
                Parcel data, reply;
                data.writeInterfaceToken(IMotionService::getInterfaceDescriptor());
                data.writeInt32(id);
                data.writeInt32(duration);
                data.writeInt32(repeat);
                data.writeInt32(flags);
                remote()->transact(RUN_EMOJI, data, &reply);
                return reply.readInt32();
            }
            void release()
            {
                Parcel data, reply;
                data.writeInterfaceToken(IMotionService::getInterfaceDescriptor());
                remote()->transact(RELEASE, data, &reply);
            }
    };

    IMPLEMENT_META_INTERFACE(MotionService, "inf.rds.motion.service");

    /**********************************************************************/

    status_t BnMotionService::onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
    {
        switch (code) {
            case SETTING: 
                {
                    CHECK_INTERFACE(IMotionService, data, reply);
                    int id = data.readInt32();
                    int cmd = data.readInt32();
                    int arg1 = data.readInt32();
                    int arg2 = data.readInt32();
                    int flag = data.readInt32();
                    int result = setting(id, cmd, arg1, arg2, flag == 1 ? interface_cast<IResultListener>(data.readStrongBinder()) : 0);
                    reply->writeInt32(result);
                    return NO_ERROR;
                }
                break;
            case SET_LISTENER:
                 {
                    CHECK_INTERFACE(IMotionService, data, reply);
                    int flag = data.readInt32();
                    int result = setCompletionListener(flag == 1 ? interface_cast<ICompletionListener>(data.readStrongBinder()) : 0);
                    reply->writeInt32(result);
                    return NO_ERROR;
                }
                break;                      
            case RUN_STOP:
                {
                    CHECK_INTERFACE(IMotionService, data, reply);
                    int session_id = data.readInt32();
                    int bodyPart = data.readInt32();
                    int arg1 = data.readInt32();
                    int result = stop(session_id, bodyPart, arg1);
                    reply->writeInt32(result);
                    return NO_ERROR;
                }
                break;           
            case RUN_RESET:
                {
                    CHECK_INTERFACE(IMotionService, data, reply);
                    int id = data.readInt32();
                    int result = reset(id);
                    reply->writeInt32(result);
                    return NO_ERROR;
                }
                break;           
            case RUN_ACTION:
                {
                    CHECK_INTERFACE(IMotionService, data, reply);
                    int id = data.readInt32();
                    int repeat = data.readInt32();
                    int duration = data.readInt32();
                    int flags = data.readInt32();
                    int result = doAction(id, repeat, duration, flags);
                    reply->writeInt32(result);
                    return NO_ERROR;
               }
                break;
            case RUN_MOTOR:
                {
                    CHECK_INTERFACE(IMotionService, data, reply);
                    int id = data.readInt32();
                    int angle = data.readInt32();
                    int duration = data.readInt32();
                    int flags = data.readInt32();
                    int result = runMotor(id, angle, duration, flags);
                    reply->writeInt32(result);
                    return NO_ERROR;
                }
                break;
            case RUN_CMDSET:
                {
                    CHECK_INTERFACE(IMotionService, data, reply);

                    int len = data.readInt32();
                    const void *pdata = data.readInplace(len);
                    int duration = data.readInt32();
                    int flags = data.readInt32();
                    int result = run(pdata, len, duration, flags);
                    reply->writeInt32(result);
                    return NO_ERROR;
                }
                break;
            case RUN_WALK:
                {
                    CHECK_INTERFACE(IMotionService, data, reply);
                    int distance = data.readInt32();
                    int speed = data.readInt32();
                    int flags = data.readInt32();
                    int result = walk(distance, speed, flags);
                    reply->writeInt32(result);
                    return NO_ERROR;
                }
                break;
            case RUN_WALKTO:
                {
                    CHECK_INTERFACE(IMotionService, data, reply);
                    int target = data.readInt32();
                    int speed = data.readInt32();
                    int flags = data.readInt32();
                    int result = walkTo(target, speed, flags);
                    reply->writeInt32(result);
                    return NO_ERROR;
                }
                break;
            case RUN_TURN:
                {
                    CHECK_INTERFACE(IMotionService, data, reply);
                    int angle = data.readInt32();
                    int speed = data.readInt32();
                    int flags = data.readInt32();
                    int result = turn(angle, speed, flags);
                    reply->writeInt32(result);
                    return NO_ERROR;
                }
                break;
            case RUN_EMOJI:
                {
                    CHECK_INTERFACE(IMotionService, data, reply);
                    int id = data.readInt32();
                    int duration = data.readInt32();
                    int repeat = data.readInt32();
                    int flags = data.readInt32();
                    int result = emoji(id, duration, repeat, flags);
                    reply->writeInt32(result);
                    return NO_ERROR;
                }
                break;
            case RELEASE:
                {
                    CHECK_INTERFACE(IMotionService, data, reply);
                    release();
                    return NO_ERROR;
                }
                break;
        }
        return BBinder::onTransact(code, data, reply, flags);
    }

    /***************************************************************************/

}; /* namespace android */
