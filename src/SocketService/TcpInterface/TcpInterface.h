#pragma once

#define DllExport __declspec( dllexport )

/*------------------------------*/
/*			定数定義			*/
/*------------------------------*/

// TCP/IP送信ｻｰﾋﾞｽ ｲﾍﾞﾝﾄ定義
enum {
	eTcpSendServiceEvent_InitSocket = 0,
	eTcpSendServiceEvent_Connect,
	eTcpSendServiceEvent_SendData,
	eTcpSendServiceEvent_Disconnect,
	eTcpSendServiceEvent_Exit,
	eTcpSendServiceEvent_Max
};

// TCP/IP受信ｻｰﾋﾞｽ ｲﾍﾞﾝﾄ定義
enum {
	eTcpRcvServiceEvent_InitSocket = 0,
	eTcpRcvServiceEvent_Max
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
} TcpSendParameter;

typedef struct
{
	int port;								/* ﾎﾟｰﾄ番号													*/
	int dataQueueSize;						/* ﾊﾞｯﾌｧｷｭｰｻｲｽﾞ												*/
	int dataQueueLen;						/* ﾊﾞｯﾌｧｷｭｰ長												*/
} TcpRcvParameter;


/*------------------------------*/
/*		プロトタイプ宣言		*/
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
