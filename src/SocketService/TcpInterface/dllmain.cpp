// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。
#include "pch.h"
#include "TcpInterface.h"
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
} stTcpInformation_tag;

static DWORD WINAPI TcpSendThreadFunc(LPVOID arg);
static DWORD WINAPI TcpRcvThreadFunc(LPVOID arg);
static void TcpClearParameter(void);

// Common
static int mTcpSendSocketNum;
static int mTcpRcvSocketNum;
// Send
static HANDLE *mTcpSendThread;
static stTcpInformation_tag *mTcpSendInformation;
static HANDLE *mTcpSendThreadEvent;
// Receive
static HANDLE *mTcpRcvThread;
static stTcpInformation_tag *mTcpRcvInformation;
static HANDLE *mTcpRcvThreadEvent;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        mTcpSendSocketNum = 0;
        mTcpRcvSocketNum = 0;
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        TcpClearParameter();
        break;
    }
    return TRUE;
}

/********************************************************************************************************/
/*																										*/
/*		関数名		：	TCP/IPインタフェース初期化処理													*/
/*																										*/
/*		機能		：	TCP/IPインタフェースを初期化する処理											*/
/*																										*/
/*		引数		：	送信端点数																		*/
/*						受信端点数																		*/
/*																										*/
/*		戻り値		：	なし																			*/
/*																										*/
/********************************************************************************************************/
void TcpInterface_TcpInitDllFunction(int sendNum, int rcvNum)
{
	TcpClearParameter();

	// Create resource for TCP Send Socket
	if(0 < sendNum)
	{
		mTcpSendSocketNum = sendNum;
											/* Threadﾊﾝﾄﾞﾙ 確保											*/
		mTcpSendThread = (HANDLE *)malloc((mTcpSendSocketNum * sizeof(HANDLE)));
		memset(mTcpSendThread, 0, (mTcpSendSocketNum * sizeof(HANDLE)));
											/* 送信用ﾃﾞｰﾀ 確保											*/
		mTcpSendInformation = (stTcpInformation_tag *)malloc((mTcpSendSocketNum * sizeof(stTcpInformation_tag)));
		memset(mTcpSendInformation, 0, (mTcpSendSocketNum * sizeof(stTcpInformation_tag)));
											/* Threadﾊﾝﾄﾞﾙｲﾍﾞﾝﾄ 確保									*/
		mTcpSendThreadEvent = (HANDLE *)malloc((mTcpSendSocketNum * (eTcpSendServiceEvent_Max * sizeof(HANDLE))));
		memset(mTcpSendThreadEvent, 0, (mTcpSendSocketNum * (eTcpSendServiceEvent_Max * sizeof(HANDLE))));
	}

	// Create resource for TCP Rcv Socket
	if(0 < rcvNum)
	{
		mTcpRcvSocketNum = rcvNum;
											/* Threadﾊﾝﾄﾞﾙ 確保											*/
		mTcpRcvThread = (HANDLE *)malloc((mTcpRcvSocketNum * sizeof(HANDLE)));
		memset(mTcpRcvThread, 0, (mTcpRcvSocketNum * sizeof(HANDLE)));
											/* 受信用ﾃﾞｰﾀ 確保											*/
		mTcpRcvInformation = (stTcpInformation_tag *)malloc((mTcpRcvSocketNum * sizeof(stTcpInformation_tag)));
		memset(mTcpRcvInformation, 0, (mTcpRcvSocketNum * sizeof(stTcpInformation_tag)));
											/* Threadﾊﾝﾄﾞﾙｲﾍﾞﾝﾄ 確保									*/
		mTcpRcvThreadEvent = (HANDLE *)malloc((mTcpRcvSocketNum * (eTcpRcvServiceEvent_Max * sizeof(HANDLE))));
		memset(mTcpRcvThreadEvent, 0, (mTcpRcvSocketNum * (eTcpRcvServiceEvent_Max * sizeof(HANDLE))));
	}
}

/********************************************************************************************************/
/*																										*/
/*		関数名		：	TCP/IP送信パラメータ設定処理													*/
/*																										*/
/*		機能		：	TCP/IP送信サービスのパラメータを設定する処理									*/
/*																										*/
/*		引数		：	送信端点No																		*/
/*						送信サービスパラメータ構造体へのポインタ										*/
/*																										*/
/*		戻り値		：	なし																			*/
/*																										*/
/********************************************************************************************************/
void TcpInterface_TcpSetSendParameter(int socketNo, TcpSendParameter *parameter)
{
	if(mTcpSendSocketNum > socketNo)
	{
		if(nullptr != (mTcpSendInformation + socketNo))
		{
			memset((mTcpSendInformation + socketNo), 0, sizeof(stTcpInformation_tag));
			memcpy((mTcpSendInformation + socketNo)->ipAddr, parameter->ipAddr, sizeof(parameter->ipAddr));
			(mTcpSendInformation + socketNo)->port = parameter->port;
			(mTcpSendInformation + socketNo)->dataQueueSize = parameter->dataQueueSize;
			(mTcpSendInformation + socketNo)->dataQueueLen = parameter->dataQueueLen;
		}
	}
}

/********************************************************************************************************/
/*																										*/
/*		関数名		：	TCP/IP受信パラメータ設定処理													*/
/*																										*/
/*		機能		：	TCP/IP受信サービスのパラメータを設定する処理									*/
/*																										*/
/*		引数		：	受信端点No																		*/
/*						受信サービスパラメータ構造体へのポインタ										*/
/*																										*/
/*		戻り値		：	なし																			*/
/*																										*/
/********************************************************************************************************/
void TcpInterface_TcpSetRcvParameter(int socketNo, TcpRcvParameter *parameter)
{
	if(mTcpRcvSocketNum > socketNo)
	{
		if(nullptr != (mTcpRcvInformation + socketNo))
		{
			memset((mTcpRcvInformation + socketNo), 0, sizeof(stTcpInformation_tag));
			(mTcpRcvInformation + socketNo)->port = parameter->port;
			(mTcpRcvInformation + socketNo)->dataQueueSize = parameter->dataQueueSize;
			(mTcpRcvInformation + socketNo)->dataQueueLen = parameter->dataQueueLen;
		}
	}
}

/********************************************************************************************************/
/*																										*/
/*		関数名		：	TCP/IP送信サービス初期化処理													*/
/*																										*/
/*		機能		：	TCP/IP送信サービスを初期化する処理												*/
/*																										*/
/*		引数		：	送信端点No																		*/
/*																										*/
/*		戻り値		：	なし																			*/
/*																										*/
/********************************************************************************************************/
void TcpInterface_InitTcpSendService(int socketNo)
{
	DWORD threadId;

	if(mTcpSendSocketNum > socketNo)
	{
		if(nullptr == *(mTcpSendThread + socketNo))
		{
			(mTcpSendInformation + socketNo)->socketNo = socketNo;
											/* 送信用ﾃﾞｰﾀ確保済みの場合は解放							*/
			if(nullptr != ((mTcpSendInformation + socketNo)->data))
			{
				free((mTcpSendInformation + socketNo)->data);
			}
			(mTcpSendInformation + socketNo)->data = (char *)malloc(((mTcpSendInformation + socketNo)->dataQueueSize * ((mTcpSendInformation + socketNo)->dataQueueLen * sizeof(char))));
			(mTcpSendInformation + socketNo)->wp = 0;
			(mTcpSendInformation + socketNo)->rp = 0;
											/* Threadﾊﾝﾄﾞﾙｲﾍﾞﾝﾄ 生成									*/
			memset((mTcpSendThreadEvent + (socketNo * eTcpSendServiceEvent_Max)), 0, (eTcpSendServiceEvent_Max * sizeof(HANDLE)));
			for(int i = 0; i < eTcpSendServiceEvent_Max; i++)
			{
				*((mTcpSendThreadEvent + (socketNo * eTcpSendServiceEvent_Max)) + i) = CreateEvent(NULL, FALSE, FALSE, NULL);
			}

			*(mTcpSendThread + socketNo) = CreateThread(nullptr, 0, TcpSendThreadFunc, (mTcpSendInformation + socketNo), 0, &threadId);
		}
	}
}

/********************************************************************************************************/
/*																										*/
/*		関数名		：	TCP/IP受信サービス初期化処理													*/
/*																										*/
/*		機能		：	TCP/IP受信サービスを初期化する処理												*/
/*																										*/
/*		引数		：	受信端点No																		*/
/*																										*/
/*		戻り値		：	なし																			*/
/*																										*/
/********************************************************************************************************/
void TcpInterface_InitTcpRcvService(int socketNo)
{
	DWORD threadId;

	if(mTcpRcvSocketNum > socketNo)
	{
		if(nullptr == *(mTcpRcvThread + socketNo))
		{
			(mTcpRcvInformation + socketNo)->socketNo = socketNo;
											/* 受信用ﾃﾞｰﾀ確保済みの場合は解放							*/
			if(nullptr != (mTcpRcvInformation + socketNo)->data)
			{
				free((mTcpRcvInformation + socketNo)->data);
			}
			(mTcpRcvInformation + socketNo)->data = (char *)malloc(((mTcpRcvInformation + socketNo)->dataQueueSize * (mTcpRcvInformation + socketNo)->dataQueueLen) * sizeof(char));
			(mTcpRcvInformation + socketNo)->wp = 0;
			(mTcpRcvInformation + socketNo)->rp = 0;
											/* Threadﾊﾝﾄﾞﾙｲﾍﾞﾝﾄ 生成									*/
			memset((mTcpRcvThreadEvent + (socketNo * eTcpRcvServiceEvent_Max)), 0, (eTcpRcvServiceEvent_Max * sizeof(HANDLE)));
			for(int i = 0; i < eTcpRcvServiceEvent_Max; i++)
			{
				*((mTcpRcvThreadEvent + (socketNo * eTcpRcvServiceEvent_Max)) + i) = CreateEvent(NULL, FALSE, FALSE, NULL);
			}
			*(mTcpRcvThread + socketNo) = CreateThread(nullptr, 0, TcpRcvThreadFunc, (mTcpRcvInformation + socketNo), 0, &threadId);
		}
	}
}

/********************************************************************************************************/
/*																										*/
/*		関数名		：	TCP/IP送信データプッシュ処理													*/
/*																										*/
/*		機能		：	TCP/IP送信データをプッシュする処理												*/
/*																										*/
/*		引数		：	送信端点No																		*/
/*						送信データへのポインタ															*/
/*						送信データ長																	*/
/*																										*/
/*		戻り値		：	なし																			*/
/*																										*/
/********************************************************************************************************/
void TcpInterface_PushTcpSendData(int socketNo, char *data, int len)
{
	if(mTcpSendSocketNum > socketNo)
	{
		if(nullptr != (mTcpSendInformation + socketNo))
		{
											/* 送信ﾃﾞｰﾀ 格納											*/
			memset(((mTcpSendInformation + socketNo)->data + ((mTcpSendInformation + socketNo)->wp * (mTcpSendInformation + socketNo)->dataQueueLen)), 0, (mTcpSendInformation + socketNo)->dataQueueLen);
			memcpy(((mTcpSendInformation + socketNo)->data + ((mTcpSendInformation + socketNo)->wp * (mTcpSendInformation + socketNo)->dataQueueLen)), data, len);
											/* ﾗｲﾄﾎﾟｲﾝﾀ 更新											*/
			(mTcpSendInformation + socketNo)->wp = (((mTcpSendInformation + socketNo)->wp + 1) % (mTcpSendInformation + socketNo)->dataQueueSize);
			if((mTcpSendInformation + socketNo)->wp == (mTcpSendInformation + socketNo)->rp)
			{
				(mTcpSendInformation + socketNo)->rp = (((mTcpSendInformation + socketNo)->rp + 1) % (mTcpSendInformation + socketNo)->dataQueueSize);
			}
		}
	}
}

/********************************************************************************************************/
/*																										*/
/*		関数名		：	TCP/IP送信サービスイベント要求処理												*/
/*																										*/
/*		機能		：	TCP/IP送信サービスへイベント要求をおこなう処理									*/
/*																										*/
/*		引数		：	送信端点No																		*/
/*						イベントNo																		*/
/*																										*/
/*		戻り値		：	なし																			*/
/*																										*/
/********************************************************************************************************/
void TcpInterface_RequestTcpSendService(int socketNo, unsigned char event)
{
	if(mTcpSendSocketNum > socketNo)
	{
		if(eTcpSendServiceEvent_Max > event)
		{
			if(nullptr != *(mTcpSendThreadEvent + ((socketNo * eTcpSendServiceEvent_Max) + event)))
			{
				SetEvent(*(mTcpSendThreadEvent + ((socketNo * eTcpSendServiceEvent_Max) + event)));
			}
		}
	}
}

/********************************************************************************************************/
/*																										*/
/*		関数名		：	TCP/IP受信サービスイベント要求処理												*/
/*																										*/
/*		機能		：	TCP/IP受信サービスへイベント要求をおこなう処理									*/
/*																										*/
/*		引数		：	受信端点No																		*/
/*						イベントNo																		*/
/*																										*/
/*		戻り値		：	なし																			*/
/*																										*/
/********************************************************************************************************/
void TcpInterface_RequestTcpRcvService(int socketNo, unsigned char event)
{
	if(mTcpRcvSocketNum > socketNo)
	{
		if(eTcpRcvServiceEvent_Max > event)
		{
			if(nullptr != *(mTcpRcvThreadEvent + ((socketNo * eTcpRcvServiceEvent_Max) + event)))
			{
				SetEvent(*(mTcpRcvThreadEvent + ((socketNo * eTcpRcvServiceEvent_Max) + event)));
			}
		}
	}
}

/********************************************************************************************************/
/*																										*/
/*		関数名		：	TCP/IP受信データプル処理														*/
/*																										*/
/*		機能		：	TCP/IP受信データをプルする処理													*/
/*																										*/
/*		引数		：	受信端点No																		*/
/*						受信データへのポインタ															*/
/*																										*/
/*		戻り値		：	なし																			*/
/*																										*/
/********************************************************************************************************/
BOOL TcpInterface_PullTcpRcvData(int socketNo, char *data)
{
	BOOL isRcvData = FALSE;

	if(mTcpRcvSocketNum > socketNo)
	{
		if(nullptr != (mTcpRcvInformation + socketNo))
		{
			if((mTcpRcvInformation + socketNo)->wp != (mTcpRcvInformation + socketNo)->rp)
			{
											/* 受信ﾃﾞｰﾀ 取得											*/
				memcpy(data, ((mTcpRcvInformation + socketNo)->data + ((mTcpRcvInformation + socketNo)->rp * (mTcpRcvInformation + socketNo)->dataQueueLen)), (mTcpRcvInformation + socketNo)->dataQueueLen);
				(mTcpRcvInformation + socketNo)->rp = (((mTcpRcvInformation + socketNo)->rp + 1) % (mTcpRcvInformation + socketNo)->dataQueueSize);
				isRcvData = TRUE;
			}
		}
	}

	return isRcvData;
}

/********************************************************************************************************/
/*																										*/
/*		関数名		：	TCP/IP送信スレッド																*/
/*																										*/
/*		機能		：	TCP/IP送信スレッド本体															*/
/*																										*/
/*		引数		：	構成情報へのポインタ															*/
/*																										*/
/*		戻り値		：	処理結果																		*/
/*																										*/
/********************************************************************************************************/
static DWORD WINAPI TcpSendThreadFunc(LPVOID arg)
{
	stTcpInformation_tag *information;
	HANDLE *event;
	WSADATA wsaData;
	socklen_t optLen;
	int sendBuff = 0;
	DWORD res;

	information = (stTcpInformation_tag *)arg;
	event = (HANDLE *)(mTcpSendThreadEvent + (information->socketNo * eTcpSendServiceEvent_Max));

	if(0 != WSAStartup(MAKEWORD(2,2), &wsaData))
	{
		return 0;
	}

	while(1)
	{
		res = WaitForMultipleObjects(eTcpSendServiceEvent_Max, event, FALSE, INFINITE);
		if((WAIT_OBJECT_0 + eTcpSendServiceEvent_InitSocket) == res)
		{
			information->socket = socket(AF_INET, SOCK_STREAM, 0);
			if(INVALID_SOCKET != information->socket)
			{
				optLen = sizeof(sendBuff);
				getsockopt(information->socket, SOL_SOCKET, SO_SNDBUF, (char *)&sendBuff, &optLen);
				sendBuff = information->dataQueueLen;
				setsockopt(information->socket, SOL_SOCKET, SO_SNDBUF, (char*)&sendBuff, sizeof(sendBuff));
				getsockopt(information->socket, SOL_SOCKET, SO_SNDBUF, (char *)&sendBuff, &optLen);
				memset(&information->sockaddr, 0, sizeof(information->sockaddr));
				information->sockaddr.sin_port = htons(information->port);
				information->sockaddr.sin_family = AF_INET;
				information->sockaddr.sin_addr.s_addr = inet_addr(information->ipAddr);
			}
		}
		else if((WAIT_OBJECT_0 + eTcpSendServiceEvent_Connect) == res)
		{
			while(1)
			{
				if(!connect(information->socket, (struct sockaddr *)&information->sockaddr, sizeof(information->sockaddr)))
				{
					break;
				}
			}
		}
		else if((WAIT_OBJECT_0 + eTcpSendServiceEvent_SendData) == res)
		{
			send(information->socket, (information->data + (information->rp * information->dataQueueLen)), information->dataQueueLen, 0);
			information->rp = ((information->rp + 1) % information->dataQueueSize);
		}
		else if((WAIT_OBJECT_0 + eTcpSendServiceEvent_Disconnect) == res)
		{
			closesocket(information->socket);
		}
		else if((WAIT_OBJECT_0 + eTcpSendServiceEvent_Exit) == res)
		{
			break;
		}
	}

	WSACleanup();

	return 0;
}

/********************************************************************************************************/
/*																										*/
/*		関数名		：	TCP/IP受信スレッド																*/
/*																										*/
/*		機能		：	TCP/IP受信スレッド本体															*/
/*																										*/
/*		引数		：	構成情報へのポインタ															*/
/*																										*/
/*		戻り値		：	処理結果																		*/
/*																										*/
/********************************************************************************************************/
static DWORD WINAPI TcpRcvThreadFunc(LPVOID arg)
{
	stTcpInformation_tag *information;
	HANDLE *event;
	WSADATA wsaData;
	int srcSocket;
	int dstSocket;
	struct sockaddr_in dstAddr;
	int dstAddrSize = sizeof(dstAddr);
	fd_set fds, readfds;
	timeval time_out;
	DWORD res;

	information = (stTcpInformation_tag *)arg;
	event = (HANDLE *)(mTcpRcvThreadEvent + (information->socketNo * eTcpRcvServiceEvent_Max));

	if(0 != WSAStartup(MAKEWORD(2,2), &wsaData))
	{
		return 0;
	}

	while(1)
	{
		res = WaitForMultipleObjects(eTcpRcvServiceEvent_Max, event, FALSE, INFINITE);
		if((WAIT_OBJECT_0 + eTcpRcvServiceEvent_InitSocket) == res)
		{
			information->socket = socket(AF_INET, SOCK_STREAM, 0);
			if(INVALID_SOCKET != information->socket)
			{
				memset(&information->sockaddr, 0, sizeof(information->sockaddr));
				information->sockaddr.sin_port = htons(information->port);
				information->sockaddr.sin_family = AF_INET;
				information->sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

				srcSocket = (int)socket(AF_INET, SOCK_STREAM, 0);
				bind(srcSocket, (struct sockaddr*)&information->sockaddr, sizeof(information->sockaddr));
				listen(srcSocket, 1);

				dstSocket = (int)accept((SOCKET)srcSocket, (struct sockaddr*)&dstAddr, &dstAddrSize);
				closesocket(srcSocket);

				FD_ZERO(&readfds);
				FD_SET(dstSocket, &readfds);
				time_out.tv_sec = 0;
				time_out.tv_usec = 10000;

				while(1)
				{
					memcpy(&fds, &readfds, sizeof(fd_set));
					select(0, &fds, NULL, NULL, &time_out);
					if(FD_ISSET(dstSocket, &fds))
					{
						memset((information->data + (information->wp * information->dataQueueLen)), 0, information->dataQueueLen);
						if(recv(dstSocket, (information->data + (information->wp * information->dataQueueLen)), information->dataQueueLen, 0))
						{
							information->wp = ((information->wp + 1) % information->dataQueueSize);
							if(information->wp == information->rp)
							{
								information->rp = ((information->rp + 1) % information->dataQueueSize);
							}
						}
						else
						{
							closesocket(dstSocket);
							break;
						}
					}
				}
			}
		}
	}

	WSACleanup();

	return 0;
}

/********************************************************************************************************/
/*																										*/
/*		関数名		：	TCP/IPパラメータクリア処理														*/
/*																										*/
/*		機能		：	TCP/IPサービスのパラメータをクリアする処理										*/
/*																										*/
/*		引数		：	なし																			*/
/*																										*/
/*		戻り値		：	なし																			*/
/*																										*/
/********************************************************************************************************/
static void TcpClearParameter(void)
{
	if(nullptr != mTcpSendThread)
	{
		free(mTcpSendThread);
		mTcpSendThread = nullptr;
	}
	if(nullptr != mTcpSendInformation)
	{
		for(int i = 0; i < mTcpSendSocketNum; i++)
		{
			if(nullptr != (mTcpSendInformation + i)->data)
			{
				free((mTcpSendInformation + i)->data);
				(mTcpSendInformation + i)->data = nullptr;
			}
		}
		free(mTcpSendInformation);
		mTcpSendInformation = nullptr;
	}
	if(nullptr != mTcpSendThreadEvent)
	{
		free(mTcpSendThreadEvent);
		mTcpSendInformation = nullptr;
	}

	if(nullptr != mTcpRcvThread)
	{
		free(mTcpRcvThread);
		mTcpRcvThread = nullptr;
	}
	if(nullptr != mTcpRcvInformation)
	{
		for(int i = 0; i < mTcpRcvSocketNum; i++)
		{
			if(nullptr != (mTcpRcvInformation + i)->data)
			{
				free((mTcpRcvInformation + i)->data);
				(mTcpRcvInformation + i)->data = nullptr;
			}
		}
		free(mTcpRcvInformation);
		mTcpRcvInformation = nullptr;
	}
	if(nullptr != mTcpRcvThreadEvent)
	{
		free(mTcpRcvThreadEvent);
		mTcpRcvThreadEvent = nullptr;
	}
}
