#ifndef IRDService_H__
#define IRDService_H__

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

namespace android
{
    class ITopic;
    class IRawCommandService;
    class ISystemController;
    class IMotionService;
    class IMotionPlayer;
    class IServiceListener;

    class IRDService: public IInterface
    {
        public:
            DECLARE_META_INTERFACE(RDService);
            virtual sp<IRawCommandService> getRawCommandService(const void * token, int len) = 0;
            virtual sp<ISystemController> getSystemController(const void * token, int len) = 0;
            virtual sp<IMotionService> getMotionService(const void * token, int len) = 0;
            virtual sp<IMotionPlayer> getMotionPlayer(const void * token, int len) = 0;
            virtual int subscribe(const sp<ITopic> & topic, const void * token, int len) = 0;
    };

    /*************************************************************************/

    class BnRDService: public BnInterface<IRDService>
    {
        public:
            virtual status_t  onTransact( uint32_t code, const Parcel& data,  Parcel* reply, uint32_t flags = 0);
    };

}; /* namespace android */

#endif /* IRDService_H___ */

