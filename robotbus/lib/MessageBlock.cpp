#include "MessageBlock.h"
#include "MessageService.h"

using namespace RobotNetwork;

float MessageBlock::TIMEOUT = 10.0f; // 10 sec

MessageBlock::~MessageBlock()
{
    Clear();
    if (remoteIp != 0)
    {
        free(remoteIp);
    }
}

void MessageBlock::init()
{
    onCompleted = 0;
    onRequest = 0;
    remoteIp = 0;
    Clear();
}

bool  MessageBlock::isFinished()
{
    if (message->obj != 0 && message->obj->isEnd() == false)
    {
        return false;
    }
    else
    {
        return true;
    }
}
bool  MessageBlock::needFragmentation()
{
    return (message->obj != 0 && (message->obj->getType() == DataObject::ObjectType::FILE || (message->obj->getSize() + headerSize) > maxPackageSize));
}

MessageBlock::MessageBlock() : MessageSerializable(0)
{
    init();
    message = new RobotMessage();
}
MessageBlock::MessageBlock(const char * ip) : MessageSerializable(0)
{
    init();
    setRemoteIP(ip);
}
MessageBlock::MessageBlock(int what, int value, DataObject * obj) : MessageSerializable(new RobotMessage(what, value))
{
    init();
    message->obj = obj;
}
MessageBlock::MessageBlock(RobotMessage * message) : MessageSerializable(message)
{
    init();
}
void MessageBlock::setRemoteIP(const char * ip)
{
    int size = strlen(ip);

    if (remoteIp != 0)
    {
        free(remoteIp);
    }
    remoteIp = (char*)malloc(size + 1);
    strcpy(remoteIp, ip);
    remoteIp[size] = 0;
}
void  MessageBlock::Clear()
{
    if (onCompleted != 0)
    {
        delete onCompleted;
    }
    if (onRequest != 0)
    {
        delete onRequest;
    }
    onRequest = 0;
    onCompleted = 0;
    retryTimes = 0;
    timeleft = 0;
    Status = STATUS::IDLE;
}
int MessageBlock::onResult(int result)
{
    if (onRequest != 0)
    {
        onRequest->onProgress(uid, 1.0f);
        onRequest->onResult(uid, message->what, result);
    }
    Clear();
    return result;
}
int MessageBlock::Send()
{
    int n = 0;
    if (remoteIp == 0 || strlen(remoteIp) == 0)
    {
        return onResult(-1);
    }
    Status = STATUS::SENDING;
    if (onCompleted != 0)
    {
        needReply = true;
    }
    n = Serialize();
    if (n < 0)
    {
        return onResult(n);
    }
    else if (n > 0)
    {
        n = MessageService::Instance().SendBytes(payload, packageSize, remoteIp);
        if (n < 0)
        {
            return onResult(n);
        }
    }
    if (isFinished() && needReply == false)
    {
        return onResult(0);
    }
    if (message->obj != 0 && message->obj->getSize() > 0 && onRequest != 0)
    {
        onRequest->onProgress(uid, (float)message->obj->getPosition() / message->obj->getSize());
    }
    retryTimes = 0;
    timeleft = TIMEOUT;

    return 0;
}
int MessageBlock::Resend()
{
    int n = MessageService::Instance().SendBytes(payload, packageSize, remoteIp);
    retryTimes++;
    timeleft = TIMEOUT;
    return n;
}
int MessageBlock::onRecv(byte * bytes, int offset, int count)
{
    int result = Deserialize(bytes, offset, count);

    if (result < 0)
    {
        return result;
    }
    if (isFinished() && needFragmentation() == false)
    {
        return 0;
    }
    MessageBlock * block = new MessageBlock(new RobotMessage());

    block->uid = 0;
    block->peer_uid = uid;
    block->message->what = isFinished() ? 0x8FFF : 0x8FFE;
    block->message->value = packageSize;
    block->message->extend = result;

    block->Serialize();

    return MessageService::Instance().SendBytes(block->payload, block->packageSize, remoteIp);
}
int MessageBlock::AckResult(RobotMessage * result)
{
    if (needReply == false)
    {
        return 0;
    }
    if (result == 0)
    {
        result = new RobotMessage(message->what, message->value);
    }
    result->what = message->what;

    MessageBlock * block = new MessageBlock(result);
    block->setRemoteIP(remoteIp);
    block->peer_uid = uid;
    return MessageService::Instance().SendMessage(block);
}
