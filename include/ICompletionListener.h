#ifndef ICompletion_Listner_H__
#define ICompletion_Listner_H__

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include <utils/KeyedVector.h>

namespace android
{

    class Parcel;

    class ICompletionListener: public IInterface
    {
        public:
            DECLARE_META_INTERFACE(CompletionListener);

            virtual int onCompletion(int session_id, int result, int errorcode) = 0;
    };

    /*************************************************************************/

    class BnCompletionListener: public BnInterface<ICompletionListener>
    {
        public:
            virtual status_t  onTransact( uint32_t code, const Parcel& data,  Parcel* reply, uint32_t flags = 0);
    };


}; /* namespace android */

#endif /* ICompletion_Listner_H__ */

