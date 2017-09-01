#ifndef MESSAGEPOOL_H_
#define MESSAGEPOOL_H_

#include "Common.h"
#include "MessageService.h"
#include "Mutex.h"

namespace RobotNetwork
{
    class MessagePool
    {
    private:
        static MessagePool * instance;
        static int g_id;
        HANDLE updateThread;
        HANDLE listenerThread;
    public:
        bool bClose;
        Socket listenerSock;

        std::list<MessageService::Listener *> listenerList;
        std::list<MessageBlock *> outMessageQueue;
        std::list<MessageBlock *> inMessageQueue;

       Mutex* mutex_listener;
       Mutex* mutex_out;
       Mutex* mutex_int;
    private:
        void init();
        MessagePool();
    public:
        static MessagePool & Instance();
    public:
        ~MessagePool();
        void Start();
        void Close();
        int SendMessage(MessageBlock * mb);
        int SendFile(int what, int value, std::string fileName, std::string targetIp, MessageBlock::OnCompleted * onComplete = 0);
        int Listen(MessageService::Listener * listener);
        int RemoveListen(int id);
        int onReceiveMessage(byte* bytes, int count, const char* ip);
    private:
        int onOutMessageEvent(MessageBlock * mb);
        int onListenEvent(MessageBlock* mblock);
    };
}

#endif
