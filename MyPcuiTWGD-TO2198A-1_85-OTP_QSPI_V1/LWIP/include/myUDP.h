#ifndef __MYUDP_H
#define __MYUDP_H
#include "sys.h"
#include "lwip_comm.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "myFIFO.h"


typedef struct
{
	struct udp_pcb *pcb;    //指向当前的pcb
	MyFIFO_t myFIFO;
}UDP_P;  

int UDPInit(u8 remoteip[4],u16_t remotePort,u16_t localPort);
int UDPSend(char *buf,int len);
int UDPRecive(char *buf,int len);


#endif

