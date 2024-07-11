// SocketService.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <iostream>
#include "Common.h"
#if defined(_TCP)
#include "TcpInterface.h"
#else defined(_UDP)
#include "UdpInterface.h"
#endif	/* defined(_TCP) */

#if defined(_TCP)
#pragma comment(lib, "TcpInterface.lib")
#else defined(_UDP)
#pragma comment(lib, "UdpInterface.lib")
#endif	/* defined(_TCP) */

#if defined(_TCP)
static void TcpSample(void);
static DWORD WINAPI TcpSendThreadFunc(LPVOID  arg);
static DWORD WINAPI TcpRcvThreadFunc(LPVOID  arg);

static HANDLE mTcpSendThread;
static HANDLE mTcpRcvThread;
#else defined(_UDP)
static void UdpSample(void);
static DWORD WINAPI UdpSendThreadFunc(LPVOID  arg);
static DWORD WINAPI UdpRcvThreadFunc(LPVOID  arg);

static HANDLE mUdpSendThread;
static HANDLE mUdpRcvThread;
#endif	/* defined(_TCP) */

int main()
{
#if defined(_TCP)
	TcpSample();
#else defined(_UDP)
	UdpSample();
#endif	/* defined(_TCP) */
}

#if defined(_TCP)
static void TcpSample(void)
{
	DWORD threadId;

	TcpInterface_TcpInitDllFunction(2, 2);

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

	/*-- 端点1 --*/
	memset(&parameter, 0, sizeof(TcpSendParameter));
	strcpy(parameter.ipAddr, (char *)TCP_IPADDR);
	parameter.port = TCP_PORT1;
	parameter.dataQueueSize = TCP_SEND_QUEUE_SIZE;
	parameter.dataQueueLen = TCP_SEND_QUEUE_LEN;

	TcpInterface_TcpSetSendParameter(0, &parameter);
	TcpInterface_InitTcpSendService(0);

	TcpInterface_RequestTcpSendService(0, eTcpSendServiceEvent_InitSocket);
	TcpInterface_RequestTcpSendService(0, eTcpSendServiceEvent_Connect);

	/*-- 端点2 --*/
	parameter.port = TCP_PORT2;

	TcpInterface_TcpSetSendParameter(1, &parameter);
	TcpInterface_InitTcpSendService(1);

	TcpInterface_RequestTcpSendService(1, eTcpSendServiceEvent_InitSocket);
	TcpInterface_RequestTcpSendService(1, eTcpSendServiceEvent_Connect);

	while(1)
	{
		/*-- 端点1 --*/
		*(data + 0) = (char)count;
		TcpInterface_PushTcpSendData(0, data, 1);
		TcpInterface_RequestTcpSendService(0, eTcpSendServiceEvent_SendData);

		/*-- 端点2 --*/
		*(data + 0) = (char)(count + 10);
		TcpInterface_PushTcpSendData(1, data, 1);
		TcpInterface_RequestTcpSendService(1, eTcpSendServiceEvent_SendData);

		count++;

		Sleep(1000);
	}

	return 0;
}

static DWORD WINAPI TcpRcvThreadFunc(LPVOID  arg)
{
	TcpRcvParameter parameter;
	char *data = (char *)malloc(TCP_RCV_QUEUE_LEN);

	/*-- 端点1 --*/
	memset(&parameter, 0, sizeof(TcpRcvParameter));
	parameter.port = TCP_PORT1;
	parameter.dataQueueSize = TCP_RCV_QUEUE_SIZE;
	parameter.dataQueueLen = TCP_RCV_QUEUE_LEN;

	TcpInterface_TcpSetRcvParameter(0, &parameter);
	TcpInterface_InitTcpRcvService(0);

	TcpInterface_RequestTcpRcvService(0, eTcpRcvServiceEvent_InitSocket);

	/*-- 端点2 --*/
	parameter.port = TCP_PORT2;

	TcpInterface_TcpSetRcvParameter(1, &parameter);
	TcpInterface_InitTcpRcvService(1);

	TcpInterface_RequestTcpRcvService(1, eTcpRcvServiceEvent_InitSocket);

	while(1)
	{
		/*-- 端点1 --*/
		if(TcpInterface_PullTcpRcvData(0, data))
		{
			MyOutputDebugString("[1] %d\r\n", *(data + 0));
			printf("[1] %d\r\n", *(data + 0));
		}

		/*-- 端点2 --*/
		if(TcpInterface_PullTcpRcvData(1, data))
		{
			MyOutputDebugString("[2] %d\r\n", *(data + 0));
			printf("[2] %d\r\n", *(data + 0));
		}

		Sleep(10);
	}

	return 0;
}
#endif	/* defined(_TCP) */
