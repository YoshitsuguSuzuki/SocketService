#ifndef COMMON_H
#define COMMON_H

#include <windows.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#if defined(_DEBUG)
#define MyOutputDebugString( str, ... ) \
      { \
        char c[256]; \
        sprintf( c, str, __VA_ARGS__ ); \
        OutputDebugStringA( c ); \
      }
#else
#define MyOutputDebugString( str, ... )
#endif	/* defined(_DEBUG) */

#if defined(_TCP)
#define TCP_IPADDR "127.0.0.1"
#define TCP_PORT1 65520
#define TCP_PORT2 65521
#define TCP_SEND_QUEUE_SIZE 16
#define TCP_SEND_QUEUE_LEN 128
#define TCP_RCV_QUEUE_SIZE 16
#define TCP_RCV_QUEUE_LEN 128
#endif	/* defined(_TCP) */

#define _TEST_ 1


#endif //COMMON_H
