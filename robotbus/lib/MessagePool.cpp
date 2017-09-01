#include <time.h>
#include "MessagePool.h"
#include "MessageBlock.h"
#include "Mutex.h"

using namespace RobotNetwork;

MessagePool * MessagePool::instance = 0;
int MessagePool::g_id = 0;

#ifdef WIN32
DWORD WINAPI updateLooper(LPVOID lpParameter);
DWORD WINAPI monitorLooper(LPVOID lpParameter);
#else
void * updateLooper(void * lpParameter);
void * monitorLooper(void * lpParameter);
#endif


MessagePool & MessagePool::Instance()
{
	if (instance == 0) instance = new MessagePool(); return *instance;
}
MessagePool::MessagePool()
{
	init();
	instance = this;
}

MessagePool::~MessagePool()
{
	if(mutex_listener != 0) delete mutex_listener;
	if(mutex_out != 0) delete mutex_out;
	if(mutex_int != 0) delete mutex_int;
}

void MessagePool::init()
{
	mutex_listener = new Mutex();
	mutex_out = new Mutex();
	mutex_int = new Mutex();
}
void MessagePool::Start()
{
	bClose = false;
#ifdef WIN32
	updateThread = CreateThread(NULL, 0, updateLooper, this, 0, NULL);
	listenerThread = CreateThread(NULL, 0, monitorLooper, this, 0, NULL);
#else
    pthread_t th_update;
    pthread_t th_monitor; 
    int pResult = pthread_create( &th_update, NULL, updateLooper, this);
	if(pResult != 0)  
    {  
        printf("Create updateLooper thread failed! \n");  
        return;  
    }
	
    pResult = pthread_create( &th_monitor, NULL, monitorLooper, this);
	if(pResult != 0)
    {  
       printf("Create monitorLooper thread failed! \n");
		return;
    }
	//pthread_join(th_update,NULL);
    //pthread_join(th_monitor,NULL);

	printf("MessagePool::Start()! \n");
#endif
}

void MessagePool::Close()
{
	bClose = true;
	
	mutex_out->Lock();
	mutex_int->Lock();
	std::list<MessageBlock *>::iterator iter;
	MessageBlock* mb = NULL;
	for (iter = outMessageQueue.begin(); iter != outMessageQueue.end();iter++)
	{	
		if(*iter != NULL)
		{    
			mb = *iter;
			mb->Clear();
		}
	}
	outMessageQueue.clear();
	for (iter = inMessageQueue.begin(); iter != inMessageQueue.end();iter++)
	{	
		if(*iter != NULL)
		{    
			mb = *iter;
			mb->Clear();
		}	
	}
	inMessageQueue.clear();
	mutex_out->Unlock();
	mutex_int->Unlock();
	if (listenerSock != 0)
	{
#ifdef WIN32
		closesocket(listenerSock);
#else
        close(listenerSock);
#endif
	}
	listenerSock = 0;
#ifdef WIN32
	WSACleanup();
	CloseHandle(updateThread);
	CloseHandle(listenerThread);
#endif
	
}

int MessagePool::SendMessage(MessageBlock * mb)
{
	if (mb->message->obj != NULL)
    {
       mb->message->obj->Reset();
    }

	if (mb->onCompleted != 0 || mb->needFragmentation())
	{
        this->mutex_out->Lock();
        if(outMessageQueue.size() > 500)
        {
        	this->mutex_out->Unlock();
        	return -1;
        }
		mb->uid = ++g_id;
		mb->Status = MessageBlock::STATUS::READY;
		outMessageQueue.push_back(mb);
		this->mutex_out->Unlock();
	}
	else
	{
		mb->Send();
	}
	return 0;
}
int MessagePool::SendFile(int what, int value, std::string fileName, std::string targetIp, MessageBlock::OnCompleted * onComplete)
{
	return 0;
}
int MessagePool::Listen(MessageService::Listener * listener)
{
    this->mutex_listener->Lock();
    listenerList.push_back(listener);
    this->mutex_listener->Unlock();

    return listener->id;
}
int MessagePool::RemoveListen(int id)
{
	this->mutex_listener->Lock();
	std::list<MessageService::Listener* >::iterator iter;
	MessageService::Listener* listener = NULL;
    for (iter = listenerList.begin(); iter != listenerList.end();iter++)
    {
    	listener = *iter;
    	if(listener != NULL && listener->id == id)
    	{
			listenerList.erase(iter);
			break;
		}
        iter++;
    }
	this->mutex_listener->Unlock();
	return 0;
}

int MessagePool::onReceiveMessage(byte* bytes, int count, const char* ip)
{
	int result = 0;
	MessageBlock* mblock = new MessageBlock(ip);
	
	std::list<MessageBlock* >::iterator iter;
	MessageBlock* outMsg = NULL;
	
	bool isPendding = false;
	mblock->Status = MessageBlock::STATUS::RECVING;
    mblock->DeserializeHeader(bytes, 0, count);
	if(count != (mblock->packageSize + MessageBlock::headerSize))
    {
		result = -1;
		goto end;
    }
	if(mblock->peer_uid != 0 && mblock->message->what == 0x8FFF)
	{
		onOutMessageEvent(mblock);
	}
	if (mblock->peer_uid != 0 && mblock->message->what == 0x8FFE)
   {
		result = onOutMessageEvent(mblock);
		goto end;
   }
   
	mutex_int->Lock();
	for (iter = outMessageQueue.begin(); iter != outMessageQueue.end();iter++)
	{	
		if(*iter != NULL)
		{    
			outMsg = *iter;
			if (outMsg->uid == mblock->uid && strcmp(mblock->remoteIp, outMsg->remoteIp) == 0)
           {
               mblock->timeleft = MessageBlock::TIMEOUT;
               isPendding = true;
               mblock = outMsg;
               break;
           }
		}
	}
	mutex_int->Unlock();

	if (mblock->onRecv(bytes, 0, count) < 0)
    {
		 result = -2;
		 goto end;
    }

	if (mblock->isFinished())
	{
		if (mblock->peer_uid != 0)
		{
			onOutMessageEvent(mblock);
		}
		else
		{
			onListenEvent(mblock);
		}

		mblock->Clear();
	}
	else if (isPendding == false)
	{
		mutex_int->Lock();
		mblock->timeleft = MessageBlock::TIMEOUT;
		inMessageQueue.push_back(mblock);
		mutex_int->Unlock();
		return 0;
	}

end:
	if(isPendding == false)
		delete mblock;

	return result;
}

int MessagePool::onListenEvent(MessageBlock* mblock)
{
	mutex_listener->Lock();
	std::list<MessageService::Listener* >::iterator iter;
	MessageService::Listener* listener = NULL;
	for (iter = listenerList.begin(); iter != listenerList.end();iter++)
    {
    	if(*iter != NULL)
    	{
			listener = *iter;
			if(listener->what == 0 || listener->what == mblock->message->what)
			{
				mblock->AckResult(listener->onComplete(mblock->message, mblock->remoteIp));
			}
		}
	}
	mutex_listener->Unlock();

	return 0;
}

int MessagePool::onOutMessageEvent(MessageBlock * mb)
{
	MessageBlock* penddingMessage = NULL;
	mutex_out->Lock();
	std::list<MessageBlock* >::iterator iter;
	MessageBlock* outMsg = NULL;
	for (iter = outMessageQueue.begin(); iter != outMessageQueue.end(); iter++)
	{	
		if(*iter != NULL)
		{    
			outMsg = *iter;
			if (outMsg->uid == mb->peer_uid && strcmp(mb->remoteIp, outMsg->remoteIp) == 0)
	        {
	            penddingMessage = outMsg;
	            break;
	        }
		}
	}
	if (penddingMessage == NULL)
    {
		 mutex_out->Unlock();
        return 0;
    }
	mutex_out->Unlock();

	if (!penddingMessage->isFinished() && mb->message->what == 0x8FFE)
	{
	    penddingMessage->Send();
	}
    if (penddingMessage->isFinished() && mb->message->what == 0x8FFF && penddingMessage->onCompleted == NULL)
    {
        penddingMessage->Clear();
    }

    if (penddingMessage->isFinished() && mb->message->what == penddingMessage->message->what)
    {
        if (penddingMessage->onCompleted != 0)
        {
            penddingMessage->onCompleted->run(mb->message);
        }
        penddingMessage->Clear();
    }
	
	return 0;
}


#ifdef WIN32
DWORD WINAPI updateLooper(LPVOID lpParameter)
#else
void * updateLooper(void * lpParameter)
#endif
{
	MessagePool * messagePool = (MessagePool*)lpParameter;
	float timeSinceLastFrame = 0.1f;
	while (messagePool->bClose == false)
	{
#ifdef WIN32
		Sleep(2);
#else
		usleep(1000 * 100);// 0.1 sec

#endif
		std::list<MessageBlock* >::iterator iter;
		messagePool->mutex_out->Lock();
		MessageBlock* outMsg = NULL;
		for (iter = messagePool->outMessageQueue.begin(); iter != messagePool->outMessageQueue.end();iter++)
		{	
			if(*iter != NULL)
			{    
				outMsg = *iter;
				if(outMsg->Status == MessageBlock::STATUS::READY)
				{
					outMsg->Send();
				}
				else if(outMsg->Status == MessageBlock::STATUS::SENDING && outMsg->retryTimes < 6)
				{
					outMsg->timeleft -= timeSinceLastFrame;
                  if (outMsg->timeleft <= 0)
                  {
                      outMsg->Resend();
                  }
				}
				else if (outMsg->Status == MessageBlock::STATUS::SENDING && outMsg->retryTimes >= 6)
              {
                   outMsg->onResult(-1);
              }
				if(outMsg->Status == MessageBlock::STATUS::IDLE)
				{
					iter = messagePool->outMessageQueue.erase(iter);
				}
			}
			else
			{
				messagePool->outMessageQueue.erase(iter);
			}
		}
		messagePool->mutex_out->Unlock();

		messagePool->mutex_int->Lock();
		for (iter = messagePool->inMessageQueue.begin(); iter != messagePool->inMessageQueue.end();iter++)
		{
			if(*iter != NULL)
			{    
				outMsg = *iter;
				outMsg->timeleft -= timeSinceLastFrame;
               if (outMsg->timeleft < 0)
               {
                   outMsg->Clear();
               }
			   if(outMsg->Status == MessageBlock::STATUS::IDLE)
				{
					messagePool->inMessageQueue.erase(iter);
				}
			}
			else
			{
				messagePool->inMessageQueue.erase(iter);
			}
		}
		messagePool->mutex_int->Unlock();
	}
	pthread_exit(0);
	return 0;
}

#ifdef WIN32
DWORD WINAPI monitorLooper(LPVOID lpParameter)
#else
void * monitorLooper(void * lpParameter)
#endif
{
	MessagePool * messagePool = (MessagePool*)lpParameter;
	byte* bytes = new byte[20480];
	memset(bytes, 0, 20480);
	
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

	messagePool->listenerSock = socket(AF_INET, SOCK_DGRAM, 0);

	SOCKADDR_IN addrServer;
	memset(&addrServer, 0, sizeof(addrServer));
	addrServer.sin_addr.s_addr = htonl(INADDR_ANY);
	addrServer.sin_family = AF_INET;
	addrServer.sin_port = htons(MessageService::Instance().local_port);

	if (bind(messagePool->listenerSock, (SOCKADDR *)&addrServer, sizeof(SOCKADDR)) < 0 ) 
	{
		printf("Bind socket failed!\n ");
		pthread_exit(0);
	}

	while (messagePool->bClose == false)
	{
		int recvlen = 0;
		socklen_t len = sizeof(SOCKADDR);
		recvlen = recvfrom(messagePool->listenerSock, bytes, 20480, 0, (SOCKADDR *)&addrServer, &len);
		char* recvIP = inet_ntoa(addrServer.sin_addr);
		printf("recvlen:%d, recvIP:%s\n", recvlen, recvIP);
		if (strlen(recvIP) == 0 || MessageSerializable::CheckType(bytes, 0, recvlen) == false)
       {
          continue;
       }
		messagePool->onReceiveMessage(bytes, recvlen, recvIP);
	}
	delete bytes;
	pthread_exit(0);
	return 0;
}
