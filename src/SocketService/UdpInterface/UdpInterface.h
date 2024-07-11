#pragma once

#define DllExport __declspec( dllexport )

/*------------------------------*/
/*			定数定義			*/
/*------------------------------*/

// UDP送信ｻｰﾋﾞｽ ｲﾍﾞﾝﾄ定義
enum {
	eUdpSendServiceEvent_InitSocket = 0,
	eUdpSendServiceEvent_SendData,
	eUdpSendServiceEvent_Disconnect,
	eUdpSendServiceEvent_Exit,
	eUdpSendServiceEvent_Max
};

// UDP受信ｻｰﾋﾞｽ ｲﾍﾞﾝﾄ定義
enum {
	eUdpRcvServiceEvent_InitSocket = 0,
	eUdpRcvServiceEvent_Max
};

/*------------------------------*/
/*			構造体定義			*/
/*------------------------------*/

typedef struct
{
	char ipAddr[16];						/* IPｱﾄﾞﾚｽ													*/
	int port;								/* ﾎﾟｰﾄ番号													*/
	int dataQueueSize;						/* ﾊﾞｯﾌｧｷｭｰｻｲｽﾞ												*/
	int dataQueueLen;						/* ﾊﾞｯﾌｧｷｭｰ長												*/
} UdpSendParameter;

typedef struct
{
	int port;								/* ﾎﾟｰﾄ番号													*/
	int dataQueueSize;						/* ﾊﾞｯﾌｧｷｭｰｻｲｽﾞ												*/
	int dataQueueLen;						/* ﾊﾞｯﾌｧｷｭｰ長												*/
} UdpRcvParameter;

/*------------------------------*/
/*		プロトタイプ宣言		*/
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
