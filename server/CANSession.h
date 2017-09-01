#ifndef CANSession_H_
#define CANSession_H_

#include <utils/List.h>
#include "rm_can_data_type.h"
#include "RmCANClient.h"
#include "ITopic.h"

namespace android
{

    class CANSession : public RmCANReceiver
    {
        public:
            CANSession();
            ~CANSession();
        public:
            int Send(int dest, const unsigned char * streams, int len);
            void SetRawTopic(sp<ITopic> topic);
        private:
            virtual void RmRecvCANData(int priority, int src_id, const void *pdata, int len);
            virtual void RmCANServiceDied();
        public:
            static CANSession * instance;
            static CANSession * Instance();
        private:
            int ParseRmSysMessage(int src_id, const unsigned char *streams, int len);
            int ParseRfMessage(const unsigned char *streams, int len);
            int ParseRbMessage(const unsigned char *streams, int len);
            int ParseRcMessage(const unsigned char *streams, int len);

            void RbCompleteTask(int type, const unsigned char *streams, int len);
            void CompleteTask(const unsigned char *streams, int len);
            void ReportEvent(int from, int event, int arg1, int arg2 = 0);
        private:
            sp<RmCANClient> m_canClient;
            sp<ITopic> rawTopic;
    };

};
#endif //RM_TRANSCEIVER_H__
