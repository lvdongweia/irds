#ifndef IRawCommandTopic_H__
#define IRawCommandTopic_H__

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include "ITopic.h"

namespace android
{
    class IRawCommandTopic : public BnTopic 
    {
        public:
            virtual int getTopicID();
            virtual int onNotify(int priority, int src_id, const void *pdata, int len);
            virtual int onRecv(int priority, int src_id, const void *pdata, int len) = 0;
    };
}; /* namespace android */

#endif /* IRawCommandTopic_H__ */

