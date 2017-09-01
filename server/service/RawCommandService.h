/*
*/

#ifndef RawCommandService_H__
#define RawCommandService_H__

#include <utils/Log.h>
#include <utils/String8.h>

#include "IRawCommandService.h"
#include "ServiceObject.h"
#include "CommandObject.h"
namespace android
{
    class RDService;

    class RawCommandService : public BnRawCommandService, public ServiceObject
    {
    public:
        RawCommandService(const void * token, int len);
        virtual ~RawCommandService();
    public:
        virtual int send(int dest, const void * streams, int len);
        virtual int run(const void * pdata, int len);
    private:
        MotorMotion motorMotion;
    };

    /**********************************************************************************/

}; /* namespace android */

#endif /* RawCommandService_H__ */
