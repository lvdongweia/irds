#include <stdio.h>

#include "RdpService.h"
#include "MessageService.h"

using namespace RobotNetwork;

int RdpService::HEARBEATCYCLE = 9;

#ifdef WIN32
DWORD WINAPI workLooper(LPVOID lpParameter);
DWORD WINAPI NotifyThread(LPVOID lpParameter);
DWORD WINAPI receiver(LPVOID lpParameter);

#else
void * workLooper(void * lpParameter);
void * NotifyThread(void * lpParameter);
void * receiver(void * lpParameter);

#endif

RdpService::RdpService()
{
    remoteClient = new RemoteClient();
    status = STATUS::Unknown;
    bStop = false;
    lenLineMessage = 0;
}

RdpService::~RdpService()
{
    if(remoteClient != 0)
    {
        delete remoteClient;
    }
    if(heartbeatMessage != 0)
    {
        delete heartbeatMessage;
    }
    if(onLineMessage != 0)
    {
        delete onLineMessage;
    }
    if(hostName != 0)
    {
        delete hostName;
    }
    if (listenerSock != 0)
    {
#ifdef WIN32
        closesocket(listenerSock);
#else
        close(listenerSock);
#endif
    }
    listenerSock = 0;
    if (Sock != 0)
    {
#ifdef WIN32
        closesocket(Sock);
#else
        close(Sock);
#endif
    }
    Sock = 0;
    
#ifdef WIN32
    WSACleanup()
    CloseHandle(workThread);
    CloseHandle(launcherThread);
    CloseHandle(receiverThread);
#endif
}


RdpService::RemoteClient::RemoteClient()
{
    byteCount = 0;
    lastheartbeat = RdpService::HEARBEATCYCLE;
}

RdpService::RemoteClient::~RemoteClient()
{
    if(targetIP != 0)
    {
        delete targetIP;
    }
}

void RdpService::RemoteClient::set(byte* bytes, int byteCount, char* targetip)
{
    if(bytes != 0 && targetip != 0)
    {
        this->bytes = bytes;
        this->byteCount = byteCount;
        int size = strlen(targetip);
        targetip = new char[size + 1];
        memcpy(targetIP, targetip, size);
    }
}

void RdpService::start(char* hostname)
{
    status = STATUS::Unknown;
    int hostNameLen = strlen(hostname);
    hostName = new char[hostNameLen + 1];
    memcpy(hostName, hostname, hostNameLen);
#ifdef WIN32
    workThread = CreateThread(NULL, 0, workLooper, this, 0, NULL);
    launcherThread = CreateThread(NULL, 0, receiver, this, 0, NULL);
#else
    pthread_t th_work;
    int pResult = pthread_create( &th_work, NULL, workLooper, this);
    if(pResult != 0)  
    {  
        printf("Create workLooper thread failed! \n");    
        return;  
    }

    pthread_t th_launcher;
    pResult = pthread_create( &th_launcher, NULL, receiver, this);
    if(pResult != 0)  
    {  
        printf("Create receiver thread failed! \n");  
        return;  
    }
    
#endif

}

void RdpService::stop()
{
    bStop =  true;
}

void RdpService::Notify()
{
#ifdef WIN32
        receiverThread = CreateThread(NULL, 0, NotifyThread, this, 0, NULL);
#else
        pthread_t th_notify;
        int pResult = pthread_create( &th_notify, NULL, NotifyThread, this);
        if(pResult != 0)  
        {  
            printf("Create NotifyThread thread failed! \n");  
        }
#endif
    return;
}

#ifdef WIN32
DWORD WINAPI NotifyThread(LPVOID lpParameter)
#else
void * NotifyThread(void * lpParameter)
#endif
{
    RdpService* rdpService = (RdpService*)lpParameter;

#ifdef WIN32
    WORD wVerisonRequested;
    WSADATA wsaData;
    wVerisonRequested = MAKEWORD(1, 1);
    int err = WSAStartup(wVerisonRequested, &wsaData);
    if (err != 0)
    {
        return false;
    }
#endif
    
    rdpService->Sock = socket(AF_INET, SOCK_DGRAM, 0);

    SOCKADDR_IN addrServer;
    memset(&addrServer, 0, sizeof(addrServer));
    addrServer.sin_addr.s_addr=htonl(INADDR_BROADCAST);
    addrServer.sin_family = AF_INET;
    addrServer.sin_port = htons(13070);
     const int opt=-1;
    int nb=0;
     nb=setsockopt(rdpService->Sock, SOL_SOCKET, SO_BROADCAST, (char*)&opt,sizeof(opt));
     if(nb==-1)
     {
         printf("nb == -1");
         pthread_exit(0);
     }
    int ret = sendto(rdpService->Sock, (const char *)rdpService->onLineMessage, rdpService->lenLineMessage, 0, (SOCKADDR*)&addrServer, sizeof(SOCKADDR));
}


#ifdef WIN32
DWORD WINAPI receiver(LPVOID lpParameter)
#else
void * receiver(void * lpParameter)
#endif
{
    RdpService* rdpService = (RdpService*)lpParameter;
    byte* buf = new byte[1024];
    memset(buf, 0, 1024);
    
#ifdef WIN32
    WORD wVerisonRequested;
    WSADATA wsaData;
    wVerisonRequested = MAKEWORD(1, 1);
    int err = WSAStartup(wVerisonRequested, &wsaData);
    if (err != 0)
    {
        return false;
    }
#endif

    rdpService->listenerSock = socket(AF_INET, SOCK_DGRAM, 0);

    SOCKADDR_IN addrServer;
    memset(&addrServer, 0, sizeof(addrServer));
    addrServer.sin_addr.s_addr = htonl(INADDR_ANY);
    addrServer.sin_family = AF_INET;
    addrServer.sin_port = htons(13071);

    if (bind(rdpService->listenerSock, (SOCKADDR *)&addrServer, sizeof(SOCKADDR)) < 0 ) 
    {
        printf("Bind socket failed!\n ");
        pthread_exit(0);
    }

    while (rdpService->bStop== false)
    {
        int recvlen = 0;
        socklen_t len = sizeof(SOCKADDR);
        recvlen = recvfrom(rdpService->listenerSock, buf, 1024, 0, (SOCKADDR *)&addrServer, &len);
        if(buf[0] != 0x05 || buf[1] != 0x13 || buf[2] != 0x28 || buf[3] != 0x07 || (buf[4] & 0xF0) == 0x10)
        {
            printf("recviver rdp server recevie data error\n");
            continue;
        }

        char* recvIP = inet_ntoa(addrServer.sin_addr);
        int type = buf[5];
        int _flag = buf[4] & 0x0F;
        int _id = ((int)buf[6] & 0xFF) | (((int)buf[7] & 0xFF) << 8);
        switch(type)
        {
            case 0x01: //online notify
               if (rdpService->status != RdpService::STATUS::Connected)
              {
                  rdpService->onLineMessage[4] |= 0x08; 
                  rdpService->onLineMessage[6] = buf[6];
                  rdpService->onLineMessage[7] = buf[7];
                  MessageService::Instance().SendBytes(rdpService->onLineMessage, rdpService->lenLineMessage, recvIP);
              }
             else if(strcmp(rdpService->remoteClient->targetIP, recvIP) == 0)
             {
                 rdpService->remoteClient->lastheartbeat = RdpService::HEARBEATCYCLE; 
                MessageService::Instance().SendBytes(rdpService->heartbeatMessage, HEADER_LEN, recvIP);
             }
               break;
             case 0x02: //connected request
            if (rdpService->status == RdpService::STATUS::UnConnected )
            {
                rdpService->remoteClient->set(buf, recvlen, recvIP);
                rdpService->status = RdpService::STATUS::Connected;
            }
            break;
        case 0x03: //disconnected
            if (rdpService->status == RdpService::STATUS::Connected && strcmp(rdpService->remoteClient->targetIP, recvIP) == 0)
            {
                 delete rdpService->remoteClient->targetIP;
                rdpService->remoteClient->targetIP = NULL;
                rdpService->status = RdpService::STATUS::Ready;
            }
            break;
        }
    }
    if(buf != 0)
        delete buf;
}


#ifdef WIN32
DWORD WINAPI workLooper(LPVOID lpParameter)
#else
void * workLooper(void * lpParameter)
#endif
{
    RdpService* rdpService = (RdpService*)lpParameter;
    rdpService->heartbeatMessage = new byte[HEADER_LEN];
    memcpy(rdpService->heartbeatMessage, rdpService->notifyHeader, HEADER_LEN);
    rdpService->heartbeatMessage[5] = 0x02;
    if(rdpService->hostName != 0 && strlen(rdpService->hostName) != 0)
    {
        rdpService->lenLineMessage = HEADER_LEN + 2 + strlen(rdpService->hostName);
        rdpService->onLineMessage = new byte[rdpService->lenLineMessage];
        memcpy(rdpService->onLineMessage, rdpService->notifyHeader, HEADER_LEN);
        byte* bytes=(byte*)rdpService->hostName;// todo:use UTF-8
        memcpy(rdpService->onLineMessage + 10, bytes, strlen(rdpService->hostName));
    }
    else
    {
        rdpService->lenLineMessage = HEADER_LEN;
        rdpService->onLineMessage = new byte[HEADER_LEN];
        memcpy(rdpService->onLineMessage, rdpService->notifyHeader, HEADER_LEN);
    }

    rdpService->status = RdpService::STATUS::Ready;
    
    while(rdpService->bStop == false)
    {
#ifdef WIN32
    Sleep(2);
#else
    usleep(1000 * 2000);// 2 sec
#endif
        switch(rdpService->status)
        {
            case RdpService::STATUS::Unknown:
                break;
            case RdpService::STATUS::Ready:
                rdpService->Notify();
                rdpService->status = RdpService::STATUS::UnConnected;
                break;
            case RdpService::STATUS::UnConnected:
                break;
            case RdpService::STATUS::Connected:
                if(rdpService->remoteClient->targetIP != 0 && rdpService->remoteClient->lastheartbeat > 0)
                {    
                    rdpService->remoteClient->lastheartbeat = -2;
                }
                if(rdpService->remoteClient->lastheartbeat <= 0)
                {
                    rdpService->status = RdpService::STATUS::Ready;
                    delete rdpService->remoteClient->targetIP;
                    rdpService->remoteClient->targetIP = NULL;                    
                }
                break;
        }
    }
    pthread_exit(0);
}

