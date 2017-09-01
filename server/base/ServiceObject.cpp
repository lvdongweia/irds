/*
 */

#include <stdint.h>
#include <sys/types.h>

#include "RDSLog.h"
#include "ServiceObject.h"

namespace android
{
    ServiceObject::ServiceObject(const void * token, int len)
    {
        this->token = (char *)malloc(len + 1);
        memset(this->token, 0, len + 1);
        memcpy(this->token, token, len);
        uid = (int)this;
        isUnset = false;
    }
    ServiceObject::~ServiceObject()
    {
        if (token != NULL)
        {
            free(token);
        }
    }
    void ServiceObject::onCompletion(int session_id, int result, int errorcode)
    {
        if (onCompletionListener.get() != 0)
        {
            onCompletionListener->onCompletion(session_id, result, errorcode);
        }
    }
    void ServiceObject::clear()
    {
    }
};

