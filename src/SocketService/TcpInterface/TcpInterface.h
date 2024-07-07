#pragma once

#define DllExport __declspec( dllexport )

/*------------------------------*/
/*			�萔��`			*/
/*------------------------------*/

// TCP/IP���M���޽ ����Ē�`
enum {
	eTcpSendServiceEvent_InitSocket = 0,
	eTcpSendServiceEvent_Connect,
	eTcpSendServiceEvent_SendData,
	eTcpSendServiceEvent_Disconnect,
	eTcpSendServiceEvent_Exit,
	eTcpSendServiceEvent_Max
};

// TCP/IP��M���޽ ����Ē�`
enum {
	eTcpRcvServiceEvent_InitSocket = 0,
	eTcpRcvServiceEvent_Max
};

/*------------------------------*/
/*			�\���̒�`			*/
/*------------------------------*/

typedef struct
{
	char ipAddr[16];						/* IP���ڽ													*/
	int port;								/* �߰Ĕԍ�													*/
	int dataQueueSize;						/* �ޯ̧�������												*/
	int dataQueueLen;						/* �ޯ̧�����												*/
} TcpSendParameter;

typedef struct
{
	int port;								/* �߰Ĕԍ�													*/
	int dataQueueSize;						/* �ޯ̧�������												*/
	int dataQueueLen;						/* �ޯ̧�����												*/
} TcpRcvParameter;


/*------------------------------*/
/*		�v���g�^�C�v�錾		*/
/*------------------------------*/
extern "C"
{
// TCP Service
	DllExport void TcpInterface_TcpInitDllFunction(int sendNum, int rcvNum);
	DllExport void TcpInterface_TcpSetSendParameter(int socketNo, TcpSendParameter *parameter);
	DllExport void TcpInterface_TcpSetRcvParameter(int socketNo, TcpRcvParameter *parameter);
	DllExport void TcpInterface_InitTcpSendService(int socketNo);
	DllExport void TcpInterface_InitTcpRcvService(int socketNo);
	DllExport void TcpInterface_PushTcpSendData(int socketNo, char *data, int len);
	DllExport void TcpInterface_RequestTcpSendService(int socketNo, unsigned char event);
	DllExport void TcpInterface_RequestTcpRcvService(int socketNo, unsigned char event);
	DllExport BOOL TcpInterface_PullTcpRcvData(int socketNo, char *data);
}
