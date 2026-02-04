#include "driverTCPServer.h"

typedef struct
{
	uint8_t tcp_connect_flag;
	struct tcp_pcb *new_pcb;
	struct tcp_pcb *former_pcb;
}tcp_server_adapter;

static uint8_t tcp_server_connect_flag = 0;

static uint8_t tcp_server_mode = 0;

static tcp_server_adapter tcp_server_net;

static sky_comDriver tcp_server_driver;

static int tcp_server_write(struct _Tsky_comDriver *self, uint8_t *data, int len);                 //TCP服务器写
static void tcp_server_send_data(struct tcp_pcb *tpcb, struct pbuf *p);                            //TCP服务器发送数据
static err_t tcp_server_recv(void *arg, struct tcp_pcb *tcp_server_pcb, struct pbuf *p, err_t err);//TCP服务器接收回调函数
static err_t tcp_server_send(void *arg, struct tcp_pcb *tcp_server_pcb, u16_t len);                //TCP服务器发送回调函数
static err_t tcp_server_listen_callback(void *arg, struct tcp_pcb *tcp_server_pcb, err_t err);     //TCP服务器监听回调函数
static void tcp_server_err_callback(void *arg, err_t err);                                         //TCP服务器错误回调函数
static void tcp_server_close(struct tcp_pcb *tcp_server_pcb);                                      //TCP服务器关闭连接

extern uint8_t pc_remoteIP[4];//PC远端IP
extern uint16_t pc_remotePort;//PC远端端口

/*******************************************************************************
 * 函数名 : driverTCPServer_Init
 * 描  述 : TCP服务器驱动初始化
 * 参  数 : localPort:本地端口
 * 返  回 : sky_comDriver结构体指针句柄
            0:初始化失败
*******************************************************************************/
sky_comDriver *driverTCPServer_Init(uint16_t localPort)
{
	err_t err = -1;
//	int t = 0;
	
	memset(&tcp_server_net, 0, sizeof(tcp_server_adapter));
	memset(&tcp_server_driver, 0, sizeof(sky_comDriver));
	
	/* 注册写函数 */
	tcp_server_driver.write = tcp_server_write;
	
	/* 创建新的PCB */
	tcp_server_net.new_pcb = tcp_new();
	if(tcp_server_net.new_pcb != NULL)
	{
		/* 绑定新的PCB和本地端口号 */
		err = tcp_bind(tcp_server_net.new_pcb, IP_ADDR_ANY, localPort);
		if(err != ERR_OK)
		{
			_DEBUG("TCPServer Bind NG!\r\n");
			
			tcp_server_close(tcp_server_net.new_pcb);
			
			return 0;
		}
		
		/* 监听新的PCB上有没客户端接入 */
		tcp_server_net.new_pcb = tcp_listen(tcp_server_net.new_pcb);
		
		/* 注册服务器监听回调函数 */
		tcp_accept(tcp_server_net.new_pcb, tcp_server_listen_callback);
	}
	else
	{
		tcp_server_close(tcp_server_net.new_pcb);
		
		return 0;
	}
	
	while(tcp_server_net.tcp_connect_flag == 0)
	{
		_DEBUG("udp connect ...\r\n");
		
		Delay_ms(1000);
	}
	
	
	
	/* 获取到远端PC的IP */
	pc_remoteIP[0] = *((uint8_t *)&tcp_server_net.former_pcb->remote_ip.addr + 0);
	pc_remoteIP[1] = *((uint8_t *)&tcp_server_net.former_pcb->remote_ip.addr + 1);
	pc_remoteIP[2] = *((uint8_t *)&tcp_server_net.former_pcb->remote_ip.addr + 2);
	pc_remoteIP[3] = *((uint8_t *)&tcp_server_net.former_pcb->remote_ip.addr + 3);
	
	/* 获取到远端PC的端口号 */
	pc_remotePort = tcp_server_net.former_pcb->remote_port;
	
	/* 打印远端PC的IP和端口号 */
	_DEBUG("tcp_server connect OK!\r\n");
	_DEBUG("tcp_server remoteIp %d.%d.%d.%d\r\n", pc_remoteIP[0], pc_remoteIP[1], pc_remoteIP[2], pc_remoteIP[3]);
	_DEBUG("tcp_server remotePort %d\r\n", pc_remotePort);
	
	
	
//	t = 0;
//	while(1)
//	{
//		if(tcp_server_net.tcp_connect_flag == 1)
//			break;
//		lwip_periodic_handle();

//		Delay_ms(1);
//		t++;
//		if(t >= 300000)
//		{
//			tcp_server_close(tcp_server_net.new_pcb);
//			
//			return 0;
//		}
//	}
	
	return &tcp_server_driver;
}

/*******************************************************************************
 * 函数名 : tcp_server_write
 * 描  述 : TCP服务器写
 * 参  数 : *data:要写的数据
 * 参  数 : len:要写的数据长度
 * 返  回 : 无
*******************************************************************************/
static int tcp_server_write(struct _Tsky_comDriver *self, uint8_t *data, int len)
{
	struct pbuf *pbuff;
	
	if(tcp_server_net.tcp_connect_flag == 1)
	{
		pbuff = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_POOL);
		if(pbuff)
		{
			pbuf_take(pbuff, data, len);
			
			tcp_server_send_data(tcp_server_net.former_pcb, pbuff);
		}
		else
		{
			tcp_server_net.tcp_connect_flag = 0;
			
			tcp_server_close(tcp_server_net.former_pcb);
		}
	}
	else
	{
		tcp_server_net.tcp_connect_flag = 0;
		
		if(tcp_server_net.former_pcb != NULL)
		{
			tcp_server_close(tcp_server_net.former_pcb);
			tcp_server_net.former_pcb = 0;
		}
	}
	
	return len;
}

/*******************************************************************************
 * 函数名 : tcp_server_send_data
 * 描  述 : TCP服务器发送数据
 * 参  数 : *tcp_server_pcb:TCP服务器的PCB
 * 参  数 : *p:要发送的数据
 * 返  回 : 无
*******************************************************************************/
static void tcp_server_send_data(struct tcp_pcb *tpcb, struct pbuf *p)
{
	struct pbuf *pbuff;
 	err_t err = ERR_OK;
    
	while((err == ERR_OK) && (p) && (p->len <= tcp_sndbuf(tpcb)))
	{
		pbuff = p;
		
		err = tcp_write(tpcb, pbuff->payload, pbuff->len, 1);
		if(err == ERR_OK)
		{
			p = pbuff->next;
			if(p)
				pbuf_ref(p);
			pbuf_free(pbuff);
		}
		else if(err == ERR_MEM)
			p = pbuff;
		
		tcp_output(tpcb);
	}
}

extern void tcp_pcb_purge(struct tcp_pcb *pcb);
/*******************************************************************************
 * 函数名 : tcp_server_recv
 * 描  述 : TCP服务器接收回调函数
 * 参  数 : *arg:自定义参数传递
 * 参  数 : *tcp_server_pcb:TCP服务器的PCB
 * 参  数 : *p:接收到的数据
 * 参  数 : err:网络错误代码
 * 返  回 : 无
*******************************************************************************/
static err_t tcp_server_recv(void *arg, struct tcp_pcb *tcp_server_pcb, struct pbuf *p, err_t err)
{
	err_t res_err = -1;
	struct pbuf *q;
	
	if(p == NULL)
	{
		tcp_server_net.tcp_connect_flag = 0;
		
		res_err = ERR_OK;
		
//		_DEBUG("struct pbuf *p is NULL!\r\n");
		
		/* 关闭连接 */
		tcp_close(tcp_server_pcb);
		tcp_arg(tcp_server_pcb, NULL);
		tcp_recv(tcp_server_pcb, NULL);
		tcp_sent(tcp_server_pcb, NULL);
		tcp_err(tcp_server_pcb, NULL);
		tcp_poll(tcp_server_pcb, NULL, 0);
		
		/* 清空PCB */
		tcp_server_net.former_pcb = 0;
	}
	else if(err != ERR_OK)
	{
		tcp_server_net.tcp_connect_flag = 0;
		
		if(p)
			pbuf_free(p);
		
		res_err = err;
		
		_DEBUG("err is %d!\r\n", err);
	}
	else if(p != NULL)
	{
		for(q = p; q != NULL; q = q->next)
		{
			/* 若sky_comDriver结构体里的fifo不为空,则写fifo不为空 */
			if(tcp_server_driver.fifo != 0)
				myFIFOWrite(tcp_server_driver.fifo, q->payload, q->len);
			
			/* 若sky_comDriver结构体里的single不为空 */
			if(tcp_server_driver.single != 0)
				tcp_server_driver.single(&tcp_server_driver, q->payload, q->len);
		}
		
		tcp_recved(tcp_server_pcb, p->tot_len);
		pbuf_free(p);
		res_err = ERR_OK;
	}
	
	return res_err;
}

/*******************************************************************************
 * 函数名 : tcp_server_listen_callback
 * 描  述 : TCP服务器发送回调函数
 * 参  数 : *arg:自定义参数传递
 * 参  数 : *tcp_server_pcb:TCP服务器的PCB
 * 参  数 : len:发送的数据长度
 * 返  回 : 无
*******************************************************************************/
static err_t tcp_server_send(void *arg, struct tcp_pcb *tcp_server_pcb, u16_t len)
{
	
	
	return ERR_OK;
}

/*******************************************************************************
 * 函数名 : tcp_server_listen_callback
 * 描  述 : TCP服务器监听回调函数
 * 参  数 : *arg:自定义参数传递
 * 参  数 : *tcp_server_pcb:TCP服务器的PCB
 * 参  数 : err:网络错误代码
 * 返  回 : 无
*******************************************************************************/
static err_t tcp_server_listen_callback(void *arg, struct tcp_pcb *tcp_server_pcb, err_t err)
{
	if(tcp_server_net.former_pcb == 0)
		tcp_server_net.former_pcb = tcp_server_pcb;
	else
	{
		/* 关闭连接 */
		tcp_close(tcp_server_net.former_pcb);
		tcp_arg(tcp_server_net.former_pcb, NULL);
		tcp_recv(tcp_server_net.former_pcb, NULL);
		tcp_sent(tcp_server_net.former_pcb, NULL);
		tcp_err(tcp_server_net.former_pcb, NULL);
		tcp_poll(tcp_server_net.former_pcb, NULL, 0);
		
		/* 记录当前PCB */
		tcp_server_net.former_pcb = tcp_server_pcb;
	}
	
	if(err == ERR_OK)
	{
		tcp_server_net.tcp_connect_flag = 1;
		
		tcp_arg(tcp_server_pcb, arg);
		tcp_poll(tcp_server_pcb, NULL, NULL);
		tcp_err(tcp_server_pcb, tcp_server_err_callback);
		
		tcp_recv(tcp_server_pcb, tcp_server_recv);
		tcp_sent(tcp_server_pcb, tcp_server_send);
	}
	else
		tcp_server_net.tcp_connect_flag = 0;
	
	return err;
}

/*******************************************************************************
 * 函数名 : tcp_server_err_callback
 * 描  述 : TCP服务器错误回调函数
 * 参  数 : *arg:自定义参数传递
 * 参  数 : err:网络错误代码
 * 返  回 : 无
*******************************************************************************/
static void tcp_server_err_callback(void *arg, err_t err)
{
	tcp_server_net.tcp_connect_flag = 0;
	
	if(tcp_server_net.former_pcb != NULL)
	{
		tcp_server_close(tcp_server_net.former_pcb);
		tcp_server_net.former_pcb = 0;
	}
}

/*******************************************************************************
 * 函数名 : tcp_server_close
 * 描  述 : TCP服务器关闭连接
 * 参  数 : *tcp_server_pcb:TCP服务器的PCB
 * 返  回 : 无
*******************************************************************************/
static void tcp_server_close(struct tcp_pcb *tcp_server_pcb)
{
//	tcp_close(tcp_server_pcb);
//	tcp_abort(tcp_server_pcb);//
	tcp_arg(tcp_server_pcb, NULL);
	tcp_recv(tcp_server_pcb, NULL);
	tcp_sent(tcp_server_pcb, NULL);
	tcp_err(tcp_server_pcb, NULL);
	tcp_poll(tcp_server_pcb, NULL, 0);
	tcp_abort(tcp_server_pcb);//
}
/*******************************************************************************
 * 函数名 : tcp_server_mode_set
 * 描  述 : TCP服务器模式设置
 * 参  数 : mode:0(默认,服务器可以给其他客户端连接抢占),1(服务器不可以给其他客户端连接抢占)
 * 返  回 : 无
*******************************************************************************/
void tcp_server_mode_set(uint8_t mode)
{
    tcp_server_mode = mode;
}