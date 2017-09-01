#ifndef IMotionService_H__
#define IMotionService_H__

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

#include "IServiceInterface.h"

namespace android
{
    class ICompletionListener;
    class IResultListener;

    class IMotionService : public IServiceInterface
    {
        public:
            DECLARE_META_INTERFACE(MotionService);
            virtual int setCompletionListener(const sp<ICompletionListener> & onCompletionListener) = 0;
            virtual int setting(int id, int cmd, int arg1, int arg2, const sp<IResultListener> & onResultListener) = 0;
            virtual int stop(int session_id = 0, int bodyPart = 0, int arg1 = 0) = 0;
            virtual int reset(int id = 0) = 0;
            virtual int doAction(int id, int repeat, int duration, int flags = 0) = 0;
            virtual int run(const void * pdata, int len, int duration = 0, int flags = 0) = 0;
            virtual int runMotor(int id, int angle, int duration/*ms*/, int flags) = 0;
            virtual int walk(int distance/*cm*/, int speed, int flags = 0) = 0;
            virtual int walkTo(int target, int speed, int flags = 0) = 0;
            virtual int turn(int angle, int speed, int flags = 0) = 0;
            virtual int emoji(int id, int duration, int repeat, int flags = 0) = 0;
    };

    /*************************************************************************/

    class BnMotionService: public BnInterface<IMotionService>
    {
        public:
            virtual status_t  onTransact( uint32_t code, const Parcel& data,  Parcel* reply, uint32_t flags = 0);
    };

}; /* namespace android */

#endif /* IMotionService_H__ */

