#ifndef MESSAGESERVICE_H_
#define MESSAGESERVICE_H_

#include "Common.h"
#include "RobotMessage.h"
#include "MessageBlock.h"

namespace RobotNetwork
{
    class DLLEXPORT MessageService
    {
    public:
        class Listener
        {
        private:
                static int guid;
        public:
            int id;
            int what;

            Listener()
            {
                id = ++guid;
                what = 0;
            }
            Listener(int what)
            {
                id = ++guid;
                this->what = what;
            }
            virtual void onStart(RobotMessage * msg) = 0;
            virtual RobotMessage * onComplete(RobotMessage * msg, std::string ip)
            {
                return 0;
            }
        };
    private:
        static MessageService * instance;

    public:
        Socket  Sock;
        static int local_port;
        static int remote_port;

        static MessageService & Instance();
        void SetPort(int local_port_, int remote_port_);
        bool Init(Listener * listener);
        bool Start(Listener * listener = 0);
        void Close();
        int SendBytes(byte * bytes, int count, std::string targetip);
        int SendMessage(MessageBlock * block);
        int RegisterListener(Listener * listener);
    private:
        MessageService();
    public:
        ~MessageService();
    };
}
#endif // !MESSAGESERVICE_H_

