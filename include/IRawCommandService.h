#ifndef IRawCommand_H__
#define IRawCommand_H__

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

namespace android
{
    class IRawCommandService: public IInterface
    {
        public:
            DECLARE_META_INTERFACE(RawCommandService);
            virtual int send(int dest, const void * streams, int len) = 0;
            virtual int run(const void * pdata, int len) = 0;
    };

    /*************************************************************************/

    class BnRawCommandService: public BnInterface<IRawCommandService>
    {
        public:
            virtual status_t  onTransact( uint32_t code, const Parcel& data,  Parcel* reply, uint32_t flags = 0);
    };

}; /* namespace android */

#endif /* IRawCommand_H__ */

