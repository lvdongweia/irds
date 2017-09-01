#ifndef ITopic_H__
#define ITopic_H__

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include "ITopic.h"

namespace android
{

    class ITopic: public IInterface
    {
        public:
            DECLARE_META_INTERFACE(Topic);
            virtual int getTopicID() = 0;
            virtual int getClsID() = 0;
            virtual int onNotify(int priority, int src_id, const void *pdata, int len) = 0;
    };

    /*************************************************************************/

    class BnTopic: public BnInterface<ITopic>
    {
        public:
            virtual status_t  onTransact( uint32_t code, const Parcel& data,  Parcel* reply, uint32_t flags = 0);
        private:
            virtual int getClsID();
    };

}; /* namespace android */

#endif /* ITopic_H__ */

