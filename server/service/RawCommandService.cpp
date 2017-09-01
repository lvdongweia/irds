/*
*/


#include <stdint.h>
#include <sys/types.h>

#include <binder/Parcel.h>
#include <utils/String8.h>
#include "RawCommandService.h"
#include "RDSTask.h"
#include "RDService.h"
#include "RDSLog.h"

namespace android 
{
    RawCommandService::RawCommandService(const void * token, int len) : ServiceObject(token, len)
    {
    }
    RawCommandService::~RawCommandService()
    {
    }
    int RawCommandService::send(int dest, const void * streams, int len)
    {
        RDSTask::Instance()->sendCommand(dest, (const unsigned char *)streams, len);
        return 0;
    }
    int RawCommandService::run(const void * pdata, int len)
    {
        return motorMotion.Run((const unsigned char *)pdata, len, 0);
    }
}; /* namespace android */
