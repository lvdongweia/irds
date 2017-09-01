#ifndef COMMON_H_

#define COMMON_H_


#ifdef WIN32

#ifdef ROBOTNETWORK_EXPORTS

#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif

#include <WinSock2.h>
#include <windows.h>

typedef SOCKET Socket;
typedef int pthread_t;
typedef int socklen_t;

#else
#define DLLEXPORT

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

typedef int Socket;
typedef int HANDLE;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <list>

typedef unsigned char byte;


#endif // !COMMON_H_
