#ifndef  MESSAGEBLOCK_H_
#define MESSAGEBLOCK_H_

#include "Common.h"
#include "RobotMessage.h"
#include "MessageSerializable.h"

namespace RobotNetwork
{
    class DLLEXPORT MessageBlock : public MessageSerializable
    {
    public:
        class STATUS
        {
        public:
            enum { IDLE = 0, READY = 1, SENDING = 2, RECVING = 3 };
        };
        class OnRequest
        {
        public:
            virtual ~OnRequest(){}
            virtual void onResult(int uid, int what, int result) = 0;
            virtual void onProgress(int uid, float rate) = 0;
        };
        class OnCompleted
        {
        public:
            virtual ~OnCompleted() {};
            virtual void run(RobotMessage * msg) = 0;
        };
        static float TIMEOUT;
        int Status;
        char * remoteIp;
        int retryTimes;
        float timeleft;
        OnCompleted * onCompleted;
        OnRequest * onRequest;
    private:
        void init();
    public:
        void setRemoteIP(const char * ip);
        bool isFinished();
        bool needFragmentation();
        void Clear();
        int onResult(int result);
        int Send();
        int Resend();
        int onRecv(byte * bytes, int offset, int count);
        int AckResult(RobotMessage * result);
    public:
        MessageBlock();
        MessageBlock(const char * ip);
        MessageBlock(int what, int value, DataObject * obj);
        MessageBlock(RobotMessage * message);
        ~MessageBlock();
    };

}
#endif // ! MESSAGEBLOCK_H_
