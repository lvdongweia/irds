/*
*/
#include <stdint.h>
#include <sys/types.h>

#include <binder/Parcel.h>
#include <utils/String8.h>

#include "ITopic.h"
#include "IRawCommandService.h"
#include "ISystemController.h"
#include "IMotionService.h"
#include "IMotionPlayer.h"

#include "IServiceListener.h"
#include "IRDService.h"

namespace android 
{
    enum
    {
        GET_RAW_SERVICE = IBinder::FIRST_CALL_TRANSACTION,
        GET_SYSTEM_CONTROLLER,
        GET_MOTION_SERVICE,
        GET_MOTION_PLAYER,
        RUN_SUBSCRIBE,
    };
    #define CALLREMOTEMETHOD(ReturnType, MethodID)     \
               int flag = 0;\
               Parcel data, reply;\
               do \
               {\
                data.writeInterfaceToken(IRDService::getInterfaceDescriptor());\
                data.writeInt32(len);\
                data.write(token, len);\
                status_t result = remote()->transact(MethodID, data, &reply);\
                if (result == NO_ERROR) \
                {\
                    flag = reply.readInt32();\
                }\
               } while(0);\
               return (flag == 0 ? 0 : interface_cast<ReturnType>(reply.readStrongBinder()));

    class BpRDService: public BpInterface<IRDService>
    {
        public:
            BpRDService(const sp<IBinder>& impl)
                : BpInterface<IRDService>(impl)
            {
            }
            int subscribe(const sp<ITopic> & topic, const void * token, int len)
            {
                Parcel data, reply;
                data.writeInterfaceToken(IRDService::getInterfaceDescriptor());
                data.writeStrongBinder(topic->asBinder());
                data.writeInt32(len);
                data.write(token, len);
                remote()->transact(RUN_SUBSCRIBE, data, &reply);
                return reply.readInt32();
            }

            sp<IRawCommandService> getRawCommandService(const void * token, int len)
            {
                CALLREMOTEMETHOD(IRawCommandService, GET_RAW_SERVICE);
            }
            sp<ISystemController> getSystemController(const void * token, int len)
            {
                CALLREMOTEMETHOD(ISystemController, GET_SYSTEM_CONTROLLER);
            }
            sp<IMotionService> getMotionService(const void * token, int len)
            {
                CALLREMOTEMETHOD(IMotionService, GET_MOTION_SERVICE);
            }
            sp<IMotionPlayer> getMotionPlayer(const void * token, int len)
            {
                CALLREMOTEMETHOD(IMotionPlayer, GET_MOTION_PLAYER);
            }           
    };

    #define CALLNATIVEMETHOD(ReturnType, CallMethod) \
                                       do \
                                        {\
                                            CHECK_INTERFACE(IRDService, data, reply);\
                                            sp<ReturnType> obj;\
                                            int len = data.readInt32(); \
                                            const void *token = data.readInplace(len); \
                                            obj = CallMethod(token, len);\
                                            reply->writeInt32(obj == 0 ? 0 : 1);\
                                            if (obj != 0)\
                                            {\
                                                reply->writeStrongBinder(obj->asBinder());\
                                            }\
                                         } while (0);\
                                         return NO_ERROR

    IMPLEMENT_META_INTERFACE(RDService, "inf.rds.service");

    /**********************************************************************/
    status_t BnRDService::onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
    {
        switch (code)
        {
            case RUN_SUBSCRIBE:
                {
                    CHECK_INTERFACE(IRDService, data, reply);
                    const sp<ITopic> topic = interface_cast<ITopic>(data.readStrongBinder());
                    int len = data.readInt32();
                    const void *token = data.readInplace(len);
                    
                    reply->writeInt32(subscribe(topic, token, len));
                    return NO_ERROR;
                }
                break;
            case GET_RAW_SERVICE:
                {
                    CALLNATIVEMETHOD(IRawCommandService, getRawCommandService);
                }
                break;
            case GET_SYSTEM_CONTROLLER:
                {
                    CALLNATIVEMETHOD(ISystemController, getSystemController);
                }
                break;
            case GET_MOTION_SERVICE:
                {
                    CALLNATIVEMETHOD(IMotionService, getMotionService);
                }
                break;
            case GET_MOTION_PLAYER:
                {
                    CALLNATIVEMETHOD(IMotionPlayer, getMotionPlayer);
                }
                break;
        }
        return BBinder::onTransact(code, data, reply, flags);
    }

    /***************************************************************************/

}; /* namespace android */
