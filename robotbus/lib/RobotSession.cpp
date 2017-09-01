#include "RobotSession.h"
#include "MessageService.h"
#include "MessageType.h"
#include "FileObject.h"
#include "DataObject.h"
#include <stdio.h>

#include "RdpService.h"
using namespace RobotNetwork;

RobotSession * RobotSession::instance_ = 0;

static byte msgblock[10];

RobotSession::RobotSession()
{
    instance_ = this;
}

RobotSession::~RobotSession()
{
    if (messageBlock != 0)
    {
        delete messageBlock;
    }
    if (bytesObject != 0)
    {
        delete bytesObject;
    }
    if(remoteIp != 0)
    {
        delete remoteIp;
    }
}

RobotSession & RobotSession::Instance()
{
    if (instance_ == 0)
    {
        new RobotSession();
    }
    return *instance_;
}

int RobotSession::Start(const char * robotip)
{
    if(remoteIp == 0)
    {
        remoteIp = new char[strlen(robotip)];
        memcpy(remoteIp, robotip, strlen(robotip));
    }
     
    MessageService::Instance().SetPort(12569, 12568);
    MessageService::Instance().Start();// start tow thread

    return 0;
}
int RobotSession::Close()
{
    MessageService::Instance().Close();

    return 0;
}
int RobotSession::MotorRun(int motorid, int angle, int duration, int flags, MessageBlock::OnCompleted* onComplete)
{
    messageBlock = new MessageBlock(new RobotMessage());
    messageBlock->setRemoteIP(remoteIp);
    bytesObject = new BytesObject();

    int ang = angle;
    int direct = 0;

    if (motorid >= 0x20 && motorid <= 0x29) // IsFinger
    {
        ang = (angle > 45 ? 1 : 0);
    }
    else if (motorid >= 0x30 && motorid <= 0x32)
    {
        ang = abs(angle);
        direct = angle > 0 ? 1 : 0;
    }
    printf("motor run id:%d angle:%d\n", motorid, angle);
    msgblock[0] = 0xF1;// 
    msgblock[1] = 0x8E;// 
    msgblock[2] = 7;// 
    msgblock[3] = (byte)motorid;
    msgblock[4] = (byte)(ang & 0xFF);
    msgblock[5] = (byte)((ang >> 8) & 0xFF);
    msgblock[6] = (byte)flags;
    msgblock[7] = (byte)(duration / 100);
    msgblock[8] = (byte)direct;
    msgblock[9] = (byte)0;

    bytesObject->setData(msgblock, 10);
    messageBlock->message->what = 0x15;// MessageType::MOTIONSID::RUN_MOTOR;
    messageBlock->message->value = 0x06;//;
    messageBlock->message->extend = 0x01;
    messageBlock->message->obj = bytesObject;
    messageBlock->onCompleted = onComplete;

    MessageService::Instance().SendMessage(messageBlock);
    return 0;
}

int RobotSession::doAction(int actionid, MessageBlock::OnCompleted* onComplete)
{
    messageBlock = new MessageBlock(new RobotMessage());
    messageBlock->setRemoteIP(remoteIp);

    messageBlock->message->what = 0x15;
    messageBlock->message->value = 0x01;
    messageBlock->message->extend = actionid;
    messageBlock->onCompleted = onComplete;

    MessageService::Instance().SendMessage(messageBlock);
    return 0;
}

int RobotSession::Emoji(int emojiid, MessageBlock::OnCompleted* onComplete)
{
    messageBlock = new MessageBlock(new RobotMessage());
    messageBlock->setRemoteIP(remoteIp);

    messageBlock->message->what = 0x15;
    messageBlock->message->value = 0x02;
    messageBlock->message->extend = emojiid;
    messageBlock->onCompleted = onComplete;

    MessageService::Instance().SendMessage(messageBlock);
    return 0;
}

int RobotSession::StopMotion(MessageBlock::OnCompleted* onComplete)
{
    messageBlock = new MessageBlock(new RobotMessage());
    messageBlock->setRemoteIP(remoteIp);

    messageBlock->message->what = 0x15;
    messageBlock->message->value = 0x09;
    messageBlock->message->extend = 0x00;
    messageBlock->onCompleted = onComplete;

    MessageService::Instance().SendMessage(messageBlock);
    return 0;

}

int RobotSession::ResetMotion(int motorid, MessageBlock::OnCompleted* onComplete)
{
    messageBlock = new MessageBlock(new RobotMessage());
    messageBlock->setRemoteIP(remoteIp);

    messageBlock->message->what = 0x15;
    messageBlock->message->value = 0x10;
    messageBlock->message->extend = motorid;
    messageBlock->onCompleted = onComplete;

    MessageService::Instance().SendMessage(messageBlock);
    return 0;
}
