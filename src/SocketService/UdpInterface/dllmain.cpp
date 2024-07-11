// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。
#include "pch.h"
#include "UdpInterface.h"
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <winuser.h>
#include <winsock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable:4996)


typedef struct {
	int socketNo;
	SOCKET socket;
	struct sockaddr_in sockaddr;
	char *data;
	int wp;
	int rp;
	char ipAddr[16];
	unsigned short port;
	int dataQueueSize;
	int dataQueueLen;
} stUdpInformation_tag;

static DWORD WINAPI UdpSendThreadFunc(LPVOID arg);
static DWORD WINAPI UdpRcvThreadFunc(LPVOID arg);
static void UdpClearParameter(void);

// Common
static int mUdpSendSocketNum;
static int mUdpRcvSocketNum;
// Send Service
static HANDLE *mUdpSendThread;
static stUdpInformation_tag *mUdpSendInformation;
static HANDLE *mUdpSendThreadEvent;
// Rcv Service
static HANDLE *mUdpRcvThread;
static stUdpInformation_tag *mUdpRcvInformation;
static HANDLE *mUdpRcvThreadEvent;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

void UdpInterface_UdpInitDllFunction(int sendNum, int rcvNum)
{
	UdpClearParameter();

	// Create resource for UDP Send Socket
	mUdpSendSocketNum = sendNum;
	if(0 < mUdpSendSocketNum)
	{
		mUdpSendThread = (HANDLE *)malloc((mUdpSendSocketNum * sizeof(HANDLE)));
		memset(mUdpSendThread, 0, (mUdpSendSocketNum * sizeof(HANDLE)));
		mUdpSendInformation = (stUdpInformation_tag *)malloc((mUdpSendSocketNum * sizeof(stUdpInformation_tag)));
		memset(mUdpSendInformation, 0, (mUdpSendSocketNum * sizeof(stUdpInformation_tag)));
		mUdpSendThreadEvent = (HANDLE *)malloc((mUdpSendSocketNum * (eUdpSendServiceEvent_Max * sizeof(HANDLE))));
		memset(mUdpSendThreadEvent, 0, (mUdpSendSocketNum * (eUdpSendServiceEvent_Max * sizeof(HANDLE))));
	}

	// Create resource for UDP Rcv Socket
	mUdpRcvSocketNum = rcvNum;
	if(0 < mUdpRcvSocketNum)
	{
		mUdpRcvThread = (HANDLE *)malloc((mUdpRcvSocketNum * sizeof(HANDLE)));
		memset(mUdpRcvThread, 0, (mUdpRcvSocketNum * sizeof(HANDLE)));
		mUdpRcvInformation = (stUdpInformation_tag *)malloc((mUdpRcvSocketNum * sizeof(stUdpInformation_tag)));
		memset(mUdpRcvInformation, 0, (mUdpRcvSocketNum * sizeof(stUdpInformation_tag)));
		mUdpRcvThreadEvent = (HANDLE *)malloc((mUdpRcvSocketNum * (eUdpRcvServiceEvent_Max * sizeof(HANDLE))));
		memset(mUdpRcvThreadEvent, 0, (mUdpRcvSocketNum * (eUdpRcvServiceEvent_Max * sizeof(HANDLE))));
	}
}

void UdpInterface_UdpSetSendParameter(int socketNo, UdpSendParameter *parameter)
{
	if(mUdpSendSocketNum > socketNo)
	{
		memset((mUdpSendInformation + socketNo), 0, sizeof(stUdpInformation_tag));
		memcpy((mUdpSendInformation + socketNo)->ipAddr, parameter->ipAddr, sizeof(parameter->ipAddr));
		(mUdpSendInformation + socketNo)->port = parameter->port;
		(mUdpSendInformation + socketNo)->dataQueueSize = parameter->dataQueueSize;
		(mUdpSendInformation + socketNo)->dataQueueLen = parameter->dataQueueLen;
	}
}

void UdpInterface_UdpSetRcvParameter(int socketNo, UdpRcvParameter *parameter)
{
	if(mUdpRcvSocketNum > socketNo)
	{
		memset((mUdpRcvInformation + socketNo), 0, sizeof(stUdpInformation_tag));
		(mUdpRcvInformation + socketNo)->port = parameter->port;
		(mUdpRcvInformation + socketNo)->dataQueueSize = parameter->dataQueueSize;
		(mUdpRcvInformation + socketNo)->dataQueueLen = parameter->dataQueueLen;
	}
}

void UdpInterface_InitUdpSendService(int socketNo)
{
	DWORD threadId;

	if(mUdpSendSocketNum > socketNo)
	{
		if(nullptr == *(mUdpSendThread + socketNo))
		{
			(mUdpSendInformation + socketNo)->socketNo = socketNo;
			if(nullptr != ((mUdpSendInformation + socketNo)->data))
			{
				free((mUdpSendInformation + socketNo)->data);
			}
			(mUdpSendInformation + socketNo)->data = (char *)malloc(((mUdpSendInformation + socketNo)->dataQueueSize * ((mUdpSendInformation + socketNo)->dataQueueLen * sizeof(char))));
			(mUdpSendInformation + socketNo)->wp = 0;
			(mUdpSendInformation + socketNo)->rp = 0;
			memset((mUdpSendThreadEvent + (socketNo * eUdpSendServiceEvent_Max)), 0, (eUdpSendServiceEvent_Max * sizeof(HANDLE)));
			*(mUdpSendThread + socketNo) = CreateThread(nullptr, 0, UdpSendThreadFunc, (mUdpSendInformation + socketNo), 0, &threadId);
		}
	}
}

void UdpInterface_InitUdpRcvService(int socketNo)
{
	DWORD threadId;

	if(mUdpRcvSocketNum > socketNo)
	{
		if(nullptr == *(mUdpRcvThread + socketNo))
		{
			(mUdpRcvInformation + socketNo)->socketNo = socketNo;
			if(nullptr != (mUdpRcvInformation + socketNo)->data)
			{
				free((mUdpRcvInformation + socketNo)->data);
			}
			(mUdpRcvInformation + socketNo)->data = (char *)malloc(((mUdpRcvInformation + socketNo)->dataQueueSize * (mUdpRcvInformation + socketNo)->dataQueueLen) * sizeof(char));
			(mUdpRcvInformation + socketNo)->wp = 0;
			(mUdpRcvInformation + socketNo)->rp = 0;
			memset((mUdpRcvThreadEvent + (socketNo * eUdpRcvServiceEvent_Max)), 0, (eUdpRcvServiceEvent_Max * sizeof(HANDLE)));
			*(mUdpRcvThread + socketNo) = CreateThread(nullptr, 0, UdpRcvThreadFunc, (mUdpRcvInformation + socketNo), 0, &threadId);
		}
	}
}

void UdpInterface_PushUdpSendData(int socketNo, char *data, int len)
{
	if(mUdpSendSocketNum > socketNo)
	{
		if(nullptr != (mUdpSendInformation + socketNo))
		{
			memset(((mUdpSendInformation + socketNo)->data + ((mUdpSendInformation + socketNo)->wp * (mUdpSendInformation + socketNo)->dataQueueLen)), 0, (mUdpSendInformation + socketNo)->dataQueueLen);
			memcpy(((mUdpSendInformation + socketNo)->data + ((mUdpSendInformation + socketNo)->wp * (mUdpSendInformation + socketNo)->dataQueueLen)), data, len);
			(mUdpSendInformation + socketNo)->wp = (((mUdpSendInformation + socketNo)->wp + 1) % (mUdpSendInformation + socketNo)->dataQueueSize);
			if((mUdpSendInformation + socketNo)->wp == (mUdpSendInformation + socketNo)->rp)
			{
				(mUdpSendInformation + socketNo)->rp = (((mUdpSendInformation + socketNo)->rp + 1) % (mUdpSendInformation + socketNo)->dataQueueSize);
			}
		}
	}
}

void UdpInterface_RequestUdpSendService(int socketNo, unsigned char event)
{
	if(mUdpSendSocketNum > socketNo)
	{
		if(eUdpSendServiceEvent_Max > event)
		{
			if(nullptr != *(mUdpSendThreadEvent + ((socketNo * eUdpSendServiceEvent_Max) + event)))
			{
				SetEvent(*(mUdpSendThreadEvent + ((socketNo * eUdpSendServiceEvent_Max) + event)));
			}
		}
	}
}

void UdpInterface_RequestUdpRcvService(int socketNo, unsigned char event)
{
	if(mUdpRcvSocketNum > socketNo)
	{
		if(eUdpRcvServiceEvent_Max > event)
		{
			if(nullptr != *(mUdpRcvThreadEvent + ((socketNo * eUdpRcvServiceEvent_Max) + event)))
			{
				SetEvent(*(mUdpRcvThreadEvent + ((socketNo * eUdpRcvServiceEvent_Max) + event)));
			}
		}
	}
}

BOOL UdpInterface_PullUdpRcvData(int socketNo, char *data)
{
	BOOL isRcvData = FALSE;

	if(mUdpRcvSocketNum > socketNo)
	{
		if(nullptr != (mUdpRcvInformation + socketNo))
		{
			if((mUdpRcvInformation + socketNo)->wp != (mUdpRcvInformation + socketNo)->rp)
			{
				memcpy(data, ((mUdpRcvInformation + socketNo)->data + ((mUdpRcvInformation + socketNo)->rp * (mUdpRcvInformation + socketNo)->dataQueueLen)), (mUdpRcvInformation + socketNo)->dataQueueLen);
				(mUdpRcvInformation + socketNo)->rp = (((mUdpRcvInformation + socketNo)->rp + 1) % (mUdpRcvInformation + socketNo)->dataQueueSize);
				isRcvData = TRUE;
			}
		}
	}

	return isRcvData;
}

static DWORD WINAPI UdpSendThreadFunc(LPVOID arg)
{
	stUdpInformation_tag *information;
	HANDLE *event;
	WSADATA wsaData;
	DWORD res;

	information = (stUdpInformation_tag *)arg;
	event = (HANDLE *)(mUdpSendThreadEvent + (information->socketNo * eUdpSendServiceEvent_Max));

	if(0 != WSAStartup(MAKEWORD(2,2), &wsaData))
	{
		return 0;
	}

	for(int i = 0; i < eUdpSendServiceEvent_Max; i++)
	{
		*(event + i) = CreateEvent(NULL, FALSE, FALSE, NULL);
	}

	while(1)
	{
		res = WaitForMultipleObjects(eUdpSendServiceEvent_Max, event, FALSE, INFINITE);
		if((WAIT_OBJECT_0 + eUdpSendServiceEvent_InitSocket) == res)
		{
			information->socket = socket(AF_INET, SOCK_DGRAM, 0);
			if(INVALID_SOCKET != information->socket)
			{
				memset(&information->sockaddr, 0, sizeof(information->sockaddr));
				information->sockaddr.sin_family = AF_INET;
				information->sockaddr.sin_port = htons(information->port);
				information->sockaddr.sin_addr.S_un.S_addr = inet_addr(information->ipAddr);
			}
		}
		else if((WAIT_OBJECT_0 + eUdpSendServiceEvent_SendData) == res)
		{
			sendto(information->socket, (information->data + (information->rp * information->dataQueueLen)), information->dataQueueLen, 0, (struct sockaddr*)&information->sockaddr, sizeof(information->sockaddr));
			information->rp = ((information->rp + 1) % information->dataQueueSize);
		}
		else if((WAIT_OBJECT_0 + eUdpSendServiceEvent_Disconnect) == res)
		{
			closesocket(information->socket);
		}
		else if((WAIT_OBJECT_0 + eUdpSendServiceEvent_Exit) == res)
		{
			break;
		}
	}

	WSACleanup();

	return 0;
}

static DWORD WINAPI UdpRcvThreadFunc(LPVOID arg)
{
	stUdpInformation_tag *information;
	HANDLE *event;
	WSADATA wsaData;
	DWORD res;

	information = (stUdpInformation_tag *)arg;
	event = (HANDLE *)(mUdpRcvThreadEvent + (information->socketNo * eUdpRcvServiceEvent_Max));

	if(0 != WSAStartup(MAKEWORD(2,2), &wsaData))
	{
		return 0;
	}

	for(int i = 0; i < eUdpRcvServiceEvent_Max; i++)
	{
		*(event + i) = CreateEvent(NULL, FALSE, FALSE, NULL);
	}

	while(1)
	{
		res = WaitForMultipleObjects(eUdpRcvServiceEvent_Max, event, FALSE, INFINITE);
		if((WAIT_OBJECT_0 + eUdpRcvServiceEvent_InitSocket) == res)
		{
			information->socket = socket(AF_INET, SOCK_DGRAM, 0);
			if(INVALID_SOCKET != information->socket)
			{
				memset(&information->sockaddr, 0, sizeof(information->sockaddr));
				information->sockaddr.sin_family = AF_INET;
				information->sockaddr.sin_port = htons(information->port);
				information->sockaddr.sin_addr.S_un.S_addr = INADDR_ANY;
				bind(information->socket, (struct sockaddr *)&information->sockaddr, sizeof(information->sockaddr));

				while(1)
				{
					memset((information->data + (information->wp * information->dataQueueLen)), 0, information->dataQueueLen);
					if(recv(information->socket, (information->data + (information->wp * information->dataQueueLen)), information->dataQueueLen, 0))
					{
						information->wp = ((information->wp + 1) % information->dataQueueSize);
						if(information->wp == information->rp)
						{
							information->rp = ((information->rp + 1) % information->dataQueueSize);
						}
					}
					else
					{
						closesocket(information->socket);
						break;
					}
				}
			}
		}
	}

	WSACleanup();

	return 0;
}

static void UdpClearParameter(void)
{
	if(nullptr != mUdpSendThread)
	{
		free(mUdpSendThread);
		mUdpSendThread = nullptr;
	}
	if(nullptr != mUdpSendInformation)
	{
		for(int i = 0; i < mUdpSendSocketNum; i++)
		{
			if(nullptr != (mUdpSendInformation + i)->data)
			{
				free((mUdpSendInformation + i)->data);
				(mUdpSendInformation + i)->data = nullptr;
			}
		}
		free(mUdpSendInformation);
		mUdpSendInformation = nullptr;
	}
	if(nullptr != mUdpSendThreadEvent)
	{
		free(mUdpSendThreadEvent);
		mUdpSendThreadEvent = nullptr;
	}

	if(nullptr != mUdpRcvThread)
	{
		free(mUdpRcvThread);
		mUdpRcvThread = nullptr;
	}
	if(nullptr != mUdpRcvInformation)
	{
		for(int i = 0; i < mUdpRcvSocketNum; i++)
		{
			if(nullptr != (mUdpRcvInformation + i)->data)
			{
				free((mUdpRcvInformation + i)->data);
				(mUdpRcvInformation + i)->data = nullptr;
			}
		}
		free(mUdpRcvInformation);
		mUdpRcvInformation = nullptr;
	}
	if(nullptr != mUdpRcvThreadEvent)
	{
		free(mUdpRcvThreadEvent);
		mUdpRcvThreadEvent = nullptr;
	}
}
