#ifndef IServiceInterface_H__
#define IServiceInterface_H__

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

namespace android
{
    class IServiceInterface : public IInterface
    {
        protected:
            virtual void release() = 0;
    };
}; /* namespace android */

#endif /* IServiceInterface_H__ */

