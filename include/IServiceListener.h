#ifndef IService_Listner_H__
#define IService_Listner_H__

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

namespace android
{

    class Parcel;

    class IServiceListener: public IInterface
    {
        public:
            DECLARE_META_INTERFACE(ServiceListener);

            virtual int onChanged(int status) = 0;
    };

    /*************************************************************************/

    class BnServiceListener: public BnInterface<IServiceListener>
    {
        public:
            virtual status_t  onTransact( uint32_t code, const Parcel& data,  Parcel* reply, uint32_t flags = 0);
        public:
            virtual ~BnServiceListener();
    };

}; /* namespace android */

#endif /* IDevice_Listner_H__ */

