#ifndef IMotionPlayer_H__
#define IMotionPlayer_H__

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include <utils/KeyedVector.h>

#include "IServiceInterface.h"

namespace android
{
    class Parcel;
    class ICompletionListener;

    class IMotionPlayer: public IServiceInterface
    {
        public:
            DECLARE_META_INTERFACE(MotionPlayer);
            virtual int isSafely() = 0;
            virtual int setCompletionListener(const sp<ICompletionListener> & onCompletionListener) = 0;
            virtual int setDataSource(const char * path, int offset, int size) = 0;
            virtual int setDataSourceByStream(const unsigned char * bytes, int size) = 0;
            virtual int prepare() = 0;
            virtual int start() = 0;
            virtual int pause() = 0;
            virtual int resume() = 0;
            virtual int stop() = 0;
            virtual int getDuration() = 0;
            virtual int getPosition() = 0;
            virtual int setPosition(int pos) = 0;
            virtual int setEnableRun(int enable) = 0;
            virtual int getRunStatus() = 0;
    };

    /*************************************************************************/

    class BnMotionPlayer: public BnInterface<IMotionPlayer>
    {
        public:
            virtual status_t  onTransact( uint32_t code, const Parcel& data,  Parcel* reply, uint32_t flags = 0);
    };

}; /* namespace android */

#endif /* IMotionPlayer_H__ */

