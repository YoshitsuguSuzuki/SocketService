// SocketService.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <iostream>
#include "Common.h"
#if defined(_TCP)
#include "TcpInterface.h"
#endif	/* defined(_TCP) */

#if defined(_TCP)
#pragma comment(lib, "TcpInterface.lib")
#endif	/* defined(_TCP) */

#if defined(_TCP)
static void TcpSample(void);
static DWORD WINAPI TcpSendThreadFunc(LPVOID  arg);
static DWORD WINAPI TcpRcvThreadFunc(LPVOID  arg);

static HANDLE mTcpSendThread;
static HANDLE mTcpRcvThread;
#endif	/* defined(_TCP) */

int main()
{
#if defined(_TCP)
	TcpSample();
#endif	/* defined(_TCP) */
}

#if defined(_TCP)
static void TcpSample(void)
{
	DWORD threadId;

	TcpSendParameter tcpSendParameter;

	TcpInterface_TcpInitDllFunction(1, 1);

	mTcpSendThread = CreateThread(NULL, 0, TcpSendThreadFunc, nullptr, 0, &threadId);
	mTcpRcvThread = CreateThread(NULL, 0, TcpRcvThreadFunc, nullptr, 0, &threadId);

	while(1)
	{
		Sleep(10);
	}
}

static DWORD WINAPI TcpSendThreadFunc(LPVOID  arg)
{
	TcpSendParameter parameter;
	char *data = (char *)malloc(TCP_SEND_QUEUE_LEN);
	int count = 0;

	memset(&parameter, 0, sizeof(TcpSendParameter));
	strcpy(parameter.ipAddr, (char *)TCP_IPADDR);
	parameter.port = TCP_PORT;
	parameter.dataQueueSize = TCP_SEND_QUEUE_SIZE;
	parameter.dataQueueLen = TCP_SEND_QUEUE_LEN;

	TcpInterface_TcpSetSendParameter(0, &parameter);
	TcpInterface_InitTcpSendService(0);
	Sleep(100);

	TcpInterface_RequestTcpSendService(0, eTcpSendServiceEvent_InitSocket);
	TcpInterface_RequestTcpSendService(0, eTcpSendServiceEvent_Connect);

	while(1)
	{
		*(data + 0) = (char)count++;
		TcpInterface_PushTcpSendData(0, data, 1);
		TcpInterface_RequestTcpSendService(0, eTcpSendServiceEvent_SendData);

		Sleep(10);
	}

	return 0;
}

static DWORD WINAPI TcpRcvThreadFunc(LPVOID  arg)
{
	TcpRcvParameter parameter;
	char *data = (char *)malloc(TCP_RCV_QUEUE_LEN);

	memset(&parameter, 0, sizeof(TcpRcvParameter));
	parameter.port = TCP_PORT;
	parameter.dataQueueSize = TCP_RCV_QUEUE_SIZE;
	parameter.dataQueueLen = TCP_RCV_QUEUE_LEN;

	TcpInterface_TcpSetRcvParameter(0, &parameter);
	TcpInterface_InitTcpRcvService(0);
	Sleep(100);

	TcpInterface_RequestTcpRcvService(0, eTcpRcvServiceEvent_InitSocket);

	while(1)
	{
		if(TcpInterface_PullTcpRcvData(0, data))
		{
			MyOutputDebugString("[0] %d\r\n", *(data + 0));
			printf("[0] %d\r\n", *(data + 0));
		}

		Sleep(10);
	}

	return 0;
}
#endif	/* defined(_TCP) */
