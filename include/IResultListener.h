#ifndef IResult_Listner_H__
#define IResult_Listner_H__

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include <utils/KeyedVector.h>

namespace android
{

    class Parcel;

    class IResultListener: public IInterface
    {
        public:
            DECLARE_META_INTERFACE(ResultListener);

            virtual int onCompletion(int id, int result) = 0;
    };

    /*************************************************************************/

    class BnResultListener: public BnInterface<IResultListener>
    {
        public:
            virtual status_t  onTransact( uint32_t code, const Parcel& data,  Parcel* reply, uint32_t flags = 0);
    };


}; /* namespace android */

#endif /* IResult_Listner_H__ */

