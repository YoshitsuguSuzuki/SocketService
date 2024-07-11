#pragma once

#define DllExport __declspec( dllexport )

/*------------------------------*/
/*			�萔��`			*/
/*------------------------------*/

// UDP���M���޽ ����Ē�`
enum {
	eUdpSendServiceEvent_InitSocket = 0,
	eUdpSendServiceEvent_SendData,
	eUdpSendServiceEvent_Disconnect,
	eUdpSendServiceEvent_Exit,
	eUdpSendServiceEvent_Max
};

// UDP��M���޽ ����Ē�`
enum {
	eUdpRcvServiceEvent_InitSocket = 0,
	eUdpRcvServiceEvent_Max
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
} UdpSendParameter;

typedef struct
{
	int port;								/* �߰Ĕԍ�													*/
	int dataQueueSize;						/* �ޯ̧�������												*/
	int dataQueueLen;						/* �ޯ̧�����												*/
} UdpRcvParameter;

/*------------------------------*/
/*		�v���g�^�C�v�錾		*/
/*------------------------------*/
extern "C"
{
// UDP Service
	DllExport void UdpInterface_UdpInitDllFunction(int sendNum, int rcvNum);
	DllExport void UdpInterface_UdpSetSendParameter(int socketNo, UdpSendParameter *parameter);
	DllExport void UdpInterface_UdpSetRcvParameter(int socketNo, UdpRcvParameter *parameter);
	DllExport void UdpInterface_InitUdpSendService(int socketNo);
	DllExport void UdpInterface_InitUdpRcvService(int socketNo);
	DllExport void UdpInterface_PushUdpSendData(int socketNo, char *data, int len);
	DllExport void UdpInterface_RequestUdpSendService(int socketNo, unsigned char event);
	DllExport void UdpInterface_RequestUdpRcvService(int socketNo, unsigned char event);
	DllExport BOOL UdpInterface_PullUdpRcvData(int socketNo, char *data);
}
