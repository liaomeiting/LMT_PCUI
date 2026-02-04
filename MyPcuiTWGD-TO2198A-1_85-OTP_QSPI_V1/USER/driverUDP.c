#include "driverUDP.h"

static struct udp_pcb *udp_pcb;
static sky_comDriver UDP_Driver;

static int udp_write(struct _Tsky_comDriver *self, uint8_t *data, int len);                                          //UDP写数据
static void udp_broadcast_recv(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, uint16_t port);//UDP接收中断函数

/*******************************************************************************
 * 函数名 : driverUDP_Init
 * 描  述 : UDP驱动初始化
 * 参  数 : remoteIp[4]:远端ip
 * 参  数 : remotePort:远端端口
 * 参  数 : local_port:本地端口
 * 返  回 : sky_comDriver结构体指针句柄
            0:初始化失败
*******************************************************************************/
sky_comDriver *driverUDP_Init(uint8_t remoteIp[4], uint16_t remotePort, uint16_t localPort)
{
	err_t status;
	struct ip_addr remote_ip;
	
	memset(&UDP_Driver, 0, sizeof(sky_comDriver));
	
	UDP_Driver.write = udp_write;//注册写函数
	
	udp_pcb = udp_new();
	if (udp_pcb)
	{
		IP4_ADDR(&remote_ip, remoteIp[0], remoteIp[1], remoteIp[2], remoteIp[3]);
		
		status = udp_connect(udp_pcb, &remote_ip, remotePort);
		if (status != ERR_OK)
			return 0;
		
		status = udp_bind(udp_pcb, IP_ADDR_ANY, localPort);
		if (status != ERR_OK)
			return 0;
		
		udp_recv(udp_pcb, udp_broadcast_recv, NULL);
	}
	else
		return 0;
	
	return &UDP_Driver;
}

/*******************************************************************************
 * 函数名 : udp_write
 * 描  述 : UDP写数据
 * 参  数 : *self:_Tsky_comDriver结构体指针
 * 参  数 : *data:要写的数据
 * 参  数 : len:数据长度
 * 返  回 : 0
*******************************************************************************/
static int udp_write(struct _Tsky_comDriver *self, uint8_t *data, int len)
{
	struct pbuf *ptr;
	
	if ((data == 0) || (len == NULL))
		return 0;
	
	ptr = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_POOL);
	
	if (ptr)
	{
		pbuf_take(ptr, data, len);
		udp_send(udp_pcb, ptr);
		pbuf_free(ptr);
	}
	
	return 0;
}

/*******************************************************************************
 * 函数名 : udp_broadcast_recv
 * 描  述 : UDP接收中断函数
 * 参  数 : *arg:未用
 * 参  数 : udp_pcb *upcb:未用
 * 参  数 : pbuf *p:缓存指针p
 * 参  数 : ip_addr *addr:ID地址指针addr
 * 参  数 : port:端口号
 * 返  回 : 无
*******************************************************************************/
static void udp_broadcast_recv(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, uint16_t port)
{
	struct pbuf *q = NULL;
	sky_comDriver *s = &UDP_Driver;
    
	if (p != NULL)
	{
		for (q = p; q != NULL; q = q->next)
		{
            Write2dev(s, q->payload, q->len);
		}
		pbuf_free(p);
	}
	else
		udp_disconnect(udp_pcb);
}
