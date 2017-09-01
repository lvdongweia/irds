#ifndef RDPSERVICE_H_
#define RDPSERVICE_H_

#include "Common.h"

#define HEADER_LEN 8

namespace RobotNetwork
{
    class DLLEXPORT RdpService 
    {
        public:
            class RemoteClient
            {
                public:
                    byte* bytes;
                    int byteCount;
                    int lastheartbeat;
                    char* targetIP;
                public:
                    RemoteClient();
                    ~RemoteClient();
                    void set(byte * bytes, int byteCount, char* targetip);
                
            };
            class STATUS
            {
                public:
                    enum { Unknown = 0, Ready = 1, UnConnected = 2, Connected = 3 };
            };
        public:
            RdpService();
            ~RdpService();
        public:
            void start(char* hostname);
            void Notify();
            void stop();
        private:
            HANDLE workThread;
            HANDLE launcherThread;
            HANDLE receiverThread;
        public:
            static int HEARBEATCYCLE;
            bool bStop;
            int status;
            int lenLineMessage;
            RemoteClient* remoteClient;
            const byte notifyHeader[HEADER_LEN] = {0x05, 0x13, 0x28, 0x07, 0x10, 0x01, 0x00, 0x00};
            byte* heartbeatMessage;
            byte* onLineMessage;
            char* hostName;
            Socket listenerSock;
            Socket Sock;
    };

    
    
}


#endif

