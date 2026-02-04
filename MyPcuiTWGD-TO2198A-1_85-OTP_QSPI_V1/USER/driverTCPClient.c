#include "driverTCPClient.h"

static uint8_t remote_ip_buff[4];
static uint16_t remotePort_buff;
static uint16_t localPort_buff;

static uint8_t tcp_flag = 0;
static struct tcp_pcb *pcb;
static struct pbuf *p;
static sky_comDriver TCP_Driver;

static int tcp_write_data(struct _Tsky_comDriver *self, uint8_t *data, int len);           //TCP写数据
static err_t tcp_send(void *arg, struct tcp_pcb *tpcb, uint16_t len);                      //TCP发送函数
static void tcp_client_send(struct tcp_pcb *tpcb);                                         //TCP客户端发送函数
static err_t tcp_recv_callBack(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);//TCP接收回调函数
static err_t tcp_connect_callBack(void *arg, struct tcp_pcb *tpcb, err_t err);             //TCP连接回调函数
static void tcp_close_connect(struct tcp_pcb *tpcb);                                       //TCP关闭连接

/*******************************************************************************
 * 函数名 : driverTCPClient_Init
 * 描  述 : TCP客户端驱动初始化
 * 参  数 : remoteIp[4]:远端ip
 * 参  数 : remotePort:远端端口
 * 参  数 : localPort:本地端口
 * 返  回 : sky_comDriver结构体指针句柄
            0:初始化失败
*******************************************************************************/
sky_comDriver *driverTCPClient_Init(uint8_t remoteIp[4], uint16_t remotePort, uint16_t localPort)
{
	int t = 0;
	err_t err;
	struct ip_addr remote_ip;
	
	memset(&TCP_Driver, 0, sizeof(sky_comDriver));
	
	TCP_Driver.write = tcp_write_data;//注册写函数
	
	remote_ip_buff[0] = remoteIp[0];
	remote_ip_buff[1] = remoteIp[1];
	remote_ip_buff[2] = remoteIp[2];
	remote_ip_buff[3] = remoteIp[3];
	
	remotePort_buff = remotePort;
	localPort_buff = localPort;
	
	tcp_flag = 0;
	
	if (pcb)
	{
		tcp_close_connect(pcb);
		
		pcb = 0;
	}
	
	pcb = tcp_new();
	if (pcb)
	{
		IP4_ADDR(&remote_ip, remoteIp[0], remoteIp[1], remoteIp[2], remoteIp[3]);
		
//		err = tcp_bind(pcb, IP_ADDR_ANY, localPort);
//		if(err != ERR_OK)
//		{
//			tcp_close_connect(pcb);
//			
//			pcb = 0;
//			
//			return 0;
//		}
		
		err = tcp_connect(pcb, &remote_ip, remotePort, tcp_connect_callBack);
		if (err != ERR_OK)
		{
			tcp_close_connect(pcb);
			
			pcb = 0;
			
			return 0;
		}
	}
	else
	{
		tcp_close_connect(pcb);
		
		pcb = 0;
		
		return 0;
	}
	
	t = 0;
	while (1)
	{
		if (tcp_flag & 0x01)
			break;
		lwip_periodic_handle();
	
		Delay_ms(1);
		t++;
		if (t >= 3000)
		{
			tcp_close_connect(pcb);
			
			pcb = 0;
			
			return &TCP_Driver;
		}
	}
	
	return &TCP_Driver;
}

/*******************************************************************************
 * 函数名 : tcp_write_data
 * 描  述 : TCP写数据
 * 参  数 : *self:sky_comDriver结构体
 * 参  数 : *data:要写的数据
 * 参  数 : len:数据长度
 * 返  回 : 写的数据长度
            -1:超时或初始化失败
*******************************************************************************/
static int tcp_write_data(struct _Tsky_comDriver *self, uint8_t *data, int len)
{
	int t;
    struct pbuf *ptr;
    
	if ((tcp_flag & 0x01) == 0)
	{
		if (driverTCPClient_Init(remote_ip_buff, remotePort_buff, localPort_buff) < 0)
			return -1;
	}
	
	p = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_POOL);
	pbuf_take(p, (char *)data, len);
    
    ptr = p;
	tcp_client_send(pcb);
    
    if (ptr)
    {
        pbuf_free(ptr);
        Delay_us(1);
    }
    
	t = 0;
	while (1)
	{
		if (tcp_flag & 0x04)
			break;
		
		lwip_periodic_handle();
		
		Delay_ms(3);
		t++;
		if (t >= 200)
			return -1;
	}
	
	return len;
}

/*******************************************************************************
 * 函数名 : tcp_send
 * 描  述 : TCP发送函数
 * 参  数 : *arg:未用
 * 参  数 : *tpcb:tcp的pcb
 * 参  数 : len:要发送的数据长度
 * 返  回 : 无
*******************************************************************************/
static err_t tcp_send(void *arg, struct tcp_pcb *tpcb, uint16_t len)
{
	if (p)
		tcp_client_send(tpcb);
	else
	 	tcp_flag |= 0x04;
	
	return ERR_OK;
}

/*******************************************************************************
 * 函数名 : tcp_client_send
 * 描  述 : TCP客户端发送函数
 * 参  数 : *arg:未用
 * 参  数 : *tpcb:tcp的pcb
 * 参  数 : len:要发送的数据长度
 * 返  回 : 无
*******************************************************************************/
static void tcp_client_send(struct tcp_pcb *tpcb)
{
	struct pbuf *ptr;
 	err_t wr_err = ERR_OK;
	
	while ((wr_err == ERR_OK) && (p) && (p->len <= tcp_sndbuf(tpcb)))
	{
		ptr = p;
		wr_err = tcp_write(tpcb, ptr->payload, ptr->len, 1);
		if (wr_err == ERR_OK)
		{
			p = ptr->next;
//			if (p)
//				pbuf_ref(p);
//			pbuf_free(ptr);
		}
		else if (wr_err == ERR_MEM)
		{
			p = ptr;
		}
		
		tcp_output(tpcb);
	}
}

/*******************************************************************************
 * 函数名 : tcp_recv_callBack
 * 描  述 : TCP接收回调函数
 * 参  数 : *arg:未用
 * 参  数 : *tpcb:tcp的pcb
 * 参  数 : *p:pbuf指针
 * 参  数 : err:错误标志
 * 返  回 : 无
*******************************************************************************/
static err_t tcp_recv_callBack(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
	struct pbuf *q;
	err_t ret_err;
	sky_comDriver *s = &TCP_Driver;
    
	if (p == NULL)
	{
		tcp_flag &= ~0x01;
		ret_err = ERR_OK;
	}
	else if (err != ERR_OK)
	{
		tcp_flag &= ~0x01;
		if (p)
			pbuf_free(p);
		ret_err = err;
	}
	else 
	{
		if (p != NULL)
		{
			for (q = p; q != NULL; q = q->next)
			{
                Write2dev(s, q->payload, q->len);
			}
 			tcp_recved(tpcb, p->tot_len);
			pbuf_free(p);
			ret_err = ERR_OK;
		}
	}
	
	return ret_err;
}

/*******************************************************************************
 * 函数名 : tcp_connect_callBack
 * 描  述 : TCP连接回调函数
 * 参  数 : *arg:未用
 * 参  数 : *tpcb:tcp的pcb
 * 参  数 : err:错误标志
 * 返  回 : 无
*******************************************************************************/
static err_t tcp_connect_callBack(void *arg, struct tcp_pcb *tpcb, err_t err)
{
	if (err == ERR_OK)
	{
		tcp_flag |= 0x01;
		tcp_arg(pcb,NULL);
		tcp_recv(pcb, tcp_recv_callBack);
		tcp_err(pcb, NULL);
		tcp_sent(pcb, tcp_send);
		tcp_poll(pcb, NULL, NULL);
	}
	else
		tcp_flag &= ~0x01;
	
	return err;
}

/*******************************************************************************
 * 函数名 : tcp_close_connect
 * 描  述 : TCP连接关闭
 * 参  数 : *tpcb:tcp的pcb
 * 返  回 : 无
*******************************************************************************/
static void tcp_close_connect(struct tcp_pcb *tpcb)
{
	tcp_arg(tpcb, NULL);
	tcp_recv(tpcb, NULL);
	tcp_sent(tpcb, NULL);
	tcp_err(tpcb, NULL);
	tcp_poll(tpcb, NULL, NULL);
	tcp_abort(tpcb);
}

/*******************************************************************************
 * 函数名 : tcp_client_detection
 * 描  述 : TCP客户端检测TCP服务器是否有监听并连接上TCP服务器
 * 参  数 : remoteIp[4]:远端ip
 * 参  数 : remotePort:远端端口
 * 参  数 : localPort:本地端口
 * 返  回 : TCPState:TCP客户端连接状态
*******************************************************************************/
int tcp_client_detection(uint8_t remoteIp[4], uint16_t remotePort, uint16_t localPort)
{
	err_t err = -1;
	int t = 0;
	static int TCPState = -1;
	struct ip_addr remote_addr;
	
	lwip_periodic_handle();
	
	if (pcb != 0)
		TCPState = pcb->state;
	else
		TCPState = CLOSED;
	if (TCPState == CLOSED)//没有连接
	{
		if (pcb)
		{
			tcp_close_connect(pcb);
			
			pcb = 0;
		}
		
		/* 重新尝试连接TCP服务器 */
		pcb = tcp_new();
		if (pcb)			//创建成功
		{
			IP4_ADDR(&remote_addr, remoteIp[0], remoteIp[1], remoteIp[2], remoteIp[3]); 
			
//			err = tcp_bind(pcb, IP_ADDR_ANY, localPort);
//			if (err != ERR_OK)
//			{
//				tcp_close_connect(pcb);
//				
//				pcb = 0;
//			}
			
			err = tcp_connect(pcb, &remote_addr, remotePort, tcp_connect_callBack);
			if (err != ERR_OK)
			{
				tcp_close_connect(pcb);
				
				pcb = 0;
				
				TCPState = CLOSED;
				
				return -1;
			}
		}
		else
		{
			tcp_close_connect(pcb);
			
			pcb = 0;
			
			TCPState = CLOSED;
			
			return -1;
		}
		
		t = 0;
		while (1)
		{
			if (tcp_flag & 0x01)
				break;
			lwip_periodic_handle();
			Delay_ms(10);
			t++;
			if (t >= 20)
			{
				tcp_close_connect(pcb);
				
				pcb = 0;
				
				TCPState = CLOSED;
				
				return -1;
			}
		}
	}
	else if (TCPState == LISTEN)//服务器进入侦听态,等待客户端的连接请求
	{
		
	}
	else if (TCPState == SYN_SENT)//连接请求已发送,等待确认
	{
		
	}
	else if (TCPState == SYN_RCVD)//已收到对方的连接请求
	{
		
	}
	else if (TCPState == ESTABLISHED)//连接已建立
	{
		
	}
	else if (TCPState == FIN_WAIT_1)//程序已关闭该连接
	{
		
	}
	else if (TCPState == FIN_WAIT_2)//另一端已接受关闭该连接
	{
		
	}
	else if (TCPState == CLOSE_WAIT)//等待程序关闭连接
	{
		/* 关闭TCP连接 */
		tcp_close_connect(pcb);
		
		pcb = 0;
		
		TCPState = CLOSED;
	}
	else if (TCPState == CLOSING)//两端同时收到对方的关闭请求
	{
		
	}
	else if (TCPState == LAST_ACK)//服务器等待对方接受关闭操作
	{
		
	}
	else if (TCPState == TIME_WAIT)//关闭成功,等待网络中可能出现的剩余数据
	{
		
	}
	
	return TCPState;
}
