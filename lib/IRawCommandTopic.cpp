
#include <stdint.h>
#include <sys/types.h>

#include <binder/Parcel.h>

#include "RDSLog.h"
#include "IRawCommandTopic.h"
#include "RDSMacros.h"

namespace android
{
    int IRawCommandTopic::getTopicID()
    {
        return RDS_EVENT_ALL;
    }
    int IRawCommandTopic::onNotify(int priority, int src_id, const void *pdata, int len)
    {
        return onRecv(priority, src_id, pdata, len);
    }
}; /* namespace android */

