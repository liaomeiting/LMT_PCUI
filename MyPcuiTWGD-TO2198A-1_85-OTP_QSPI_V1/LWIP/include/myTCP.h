#ifndef _MYTCP_H
#define _MYTCP_H

#include "sys.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/tcp.h"
#include "lwip/memp.h"
#include "lwip/mem.h"
#include "lwip_comm.h"
#include "myFIFO.h"

//LWIP回调函数使用的结构体
typedef struct 
{
	u8 mytcpFlag;	
	u8 remoteip[4];
	u16_t remotePort;
	u16_t localPort;
	struct tcp_pcb *pcb;    //指向当前的pcb
	struct pbuf *p;         //指向接收/或传输的pbuf
	MyFIFO_t myFIFO;
	
}TCP_P;  

int TCPInit(u8 remoteip[4],u16_t remotePort,u16_t localPort);
int TCPRecive(char *buf,int len);
int TCPSend(char *data,int len);

#endif

