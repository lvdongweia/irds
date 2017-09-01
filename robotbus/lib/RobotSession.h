#ifndef ROBOTSESSION_H_
#define ROBOTSESSION_H_

#include "MessageBlock.h"
#include "BytesObject.h"

namespace RobotNetwork
{
    class DLLEXPORT RobotSession
    {
    private:
        MessageBlock * messageBlock;
        BytesObject * bytesObject;
        static RobotSession * instance_;
        char* remoteIp;
    private:
        RobotSession();
    public:
        ~RobotSession();
        static RobotSession & Instance();

        int Start(const char * robotip);
        int Close();
        int MotorRun(int motorid, int angle, int duration = 1000, int flags = 1, MessageBlock::OnCompleted* onComplete = NULL);
        int doAction(int actionid, MessageBlock::OnCompleted* onComplete = NULL);
        int Emoji(int emojiid, MessageBlock::OnCompleted* onComplete = NULL);
        int StopMotion(MessageBlock::OnCompleted* onComplete = NULL);
        int ResetMotion(int motorid, MessageBlock::OnCompleted* onComplete = NULL);
    };
}

#endif

