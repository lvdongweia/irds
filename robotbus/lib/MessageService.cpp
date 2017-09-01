#include "MessageService.h"
#include "MessagePool.h"

using namespace RobotNetwork;

int MessageService::Listener::guid = 0;
MessageService * MessageService::instance = 0;
int MessageService::local_port = 0;
int MessageService::remote_port = 0;

#ifdef WIN32
#pragma comment(lib, "ws2_32.lib")
#endif

MessageService::~MessageService()
{
}

MessageService & MessageService::Instance()
{
    if (instance == 0)
    {
        instance = new MessageService();
    }
    return *instance;
}
MessageService::MessageService()
{
    Sock = 0;
    instance = this;
    MessagePool::Instance();
}
void MessageService::SetPort(int local_port_, int remote_port_)
{
    local_port = local_port_;
    remote_port = remote_port_;
}
bool MessageService::Init(Listener * listener)
{
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

    Sock = socket(AF_INET, SOCK_DGRAM, 0);

    MessagePool::Instance().Start();
    if (listener != 0)
    {
        MessagePool::Instance().Listen(listener);
    }
    return true;
}
bool MessageService::Start(Listener * listener)
{
    return Init(listener);
}
void MessageService::Close()
{
    MessagePool::Instance().Close();

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
    WSACleanup();
#endif
}
int MessageService::SendBytes(byte * bytes, int count, std::string targetip)
{
    if (Sock == 0 || bytes == 0 || targetip.size() == 0)
    {
        return -11;
    }
    int ret = 0;
    SOCKADDR_IN addrServer;
    memset(&addrServer, 0, sizeof(addrServer));
    addrServer.sin_addr.s_addr = inet_addr(targetip.c_str());
    addrServer.sin_family = AF_INET;
    addrServer.sin_port = htons(remote_port);
    ret = sendto(Sock, (const char *)bytes, count, 0, (SOCKADDR*)&addrServer, sizeof(SOCKADDR));
    printf("send byte to targetip:%s ret:%d\n", targetip.c_str(), ret);
    return ret;
}
int MessageService::SendMessage(MessageBlock * block)
{
    return MessagePool::Instance().SendMessage(block);
}
int MessageService::RegisterListener(Listener * listener)
{
    return MessagePool::Instance().Listen(listener);
}
