#include "driverTCPServer.h"

typedef struct
{
	uint8_t tcp_connect_flag;
	struct tcp_pcb *new_pcb;
	struct tcp_pcb *former_pcb;
}tcp_server_adapter;

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
	
	if (tcp_server_net.new_pcb)
	{
		tcp_server_close(tcp_server_net.new_pcb);
		
		tcp_server_net.new_pcb = 0;
	}
	
	/* 创建新的PCB */
	tcp_server_net.new_pcb = tcp_new();
	if (tcp_server_net.new_pcb != NULL)
	{
		/* 绑定新的PCB和本地端口号 */
		err = tcp_bind(tcp_server_net.new_pcb, IP_ADDR_ANY, localPort);
		if (err != ERR_OK)
		{
			tcp_server_close(tcp_server_net.new_pcb);
			
			tcp_server_net.new_pcb = 0;
			
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
		
		tcp_server_net.new_pcb = 0;
		
		return 0;
	}
	
//	t = 0;
//	while (1)
//	{
//		if (tcp_server_net.tcp_connect_flag == 1)
//			break;
//		lwip_periodic_handle();

//		Delay_ms(1);
//		t++;
//		if (t >= 300000)
//		{
//			tcp_server_close(tcp_server_net.new_pcb);
//			
//			tcp_server_net.new_pcb = 0;
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
	
	if (tcp_server_net.tcp_connect_flag == 1)
	{
		pbuff = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_POOL);
		if (pbuff)
		{
			pbuf_take(pbuff, data, len);
			
			tcp_server_send_data(tcp_server_net.former_pcb, pbuff);
		}
		else
		{
			tcp_server_net.tcp_connect_flag = 0;
			
			if (tcp_server_net.former_pcb != NULL)
			{
				tcp_server_close(tcp_server_net.former_pcb);
				tcp_server_net.former_pcb = 0;
			}
		}
	}
	else
	{
		tcp_server_net.tcp_connect_flag = 0;
		
		if (tcp_server_net.former_pcb != NULL)
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
    
	while ((err == ERR_OK) && (p) && (p->len <= tcp_sndbuf(tpcb)))
	{
		pbuff = p;
		
		err = tcp_write(tpcb, pbuff->payload, pbuff->len, 1);
		if (err == ERR_OK)
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
	sky_comDriver *s = &tcp_server_driver;
    
	if (p == NULL)//远端客户端断开连接
	{
		tcp_server_net.tcp_connect_flag = 0;
		
		res_err = ERR_OK;
		
		/* 关闭连接 */
		tcp_close(tcp_server_pcb);
		tcp_arg(tcp_server_pcb, NULL);
		tcp_recv(tcp_server_pcb, NULL);
		tcp_sent(tcp_server_pcb, NULL);
		tcp_err(tcp_server_pcb, NULL);
		tcp_poll(tcp_server_pcb, NULL, NULL);
		
		/* 清空PCB */
		tcp_server_net.former_pcb = 0;
	}
	else if (err != ERR_OK)
	{
		tcp_server_net.tcp_connect_flag = 0;
		
		if (p)
			pbuf_free(p);
		
		res_err = err;
	}
	else if (p != NULL)
	{
		for (q = p; q != NULL; q = q->next)
		{
            Write2dev(s, q->payload, q->len);
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
	if (tcp_server_net.former_pcb == 0)
    {
		tcp_server_net.former_pcb = tcp_server_pcb;
    }
	else
	{
        if ((tcp_server_net.tcp_connect_flag == 0) || (tcp_server_mode == 0))
        {
            /* 关闭连接 */
            tcp_close(tcp_server_net.former_pcb);
            tcp_arg(tcp_server_net.former_pcb, NULL);
            tcp_recv(tcp_server_net.former_pcb, NULL);
            tcp_sent(tcp_server_net.former_pcb, NULL);
            tcp_err(tcp_server_net.former_pcb, NULL);
            tcp_poll(tcp_server_net.former_pcb, NULL, NULL);
            
            /* 记录当前新的PCB */
            tcp_server_net.former_pcb = tcp_server_pcb;
        }
	}
	
	if (err == ERR_OK)
	{
        if ((tcp_server_net.tcp_connect_flag == 0) || (tcp_server_mode == 0))
        {
            tcp_server_net.tcp_connect_flag = 1;
            
            tcp_arg(tcp_server_pcb, arg);
            tcp_poll(tcp_server_pcb, NULL, NULL);
            tcp_err(tcp_server_pcb, tcp_server_err_callback);
            
            tcp_recv(tcp_server_pcb, tcp_server_recv);
            tcp_sent(tcp_server_pcb, tcp_server_send);
        }
        else
            return ERR_ISCONN;
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
	
	if (tcp_server_net.former_pcb != NULL)
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
	tcp_arg(tcp_server_pcb, NULL);
	tcp_recv(tcp_server_pcb, NULL);
	tcp_sent(tcp_server_pcb, NULL);
	tcp_err(tcp_server_pcb, NULL);
	tcp_poll(tcp_server_pcb, NULL, NULL);
	tcp_abort(tcp_server_pcb);
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
