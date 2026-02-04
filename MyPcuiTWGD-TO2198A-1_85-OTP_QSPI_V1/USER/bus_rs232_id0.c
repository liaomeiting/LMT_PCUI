/********************************* SKYCODE *************************************
* Copyright (c) 2016-2018, 深圳思凯测试技术有限公司（广州天码电子技术有限公司）
* All rights reserved.
*
* 文件名   ：bus_rs232_ID0.c    
* 描述     ：Bus To RS232模块应用程序，模块ID=0
*
* 版    本 ：V1.0.1
* 作    者 ：
* 完成日期 ：2018-10-17
* 修改描述 ：1.S10专用 
*
* 版    本 ：V1.0
* 作    者 ：SKYCODE2013
* 完成日期 ：2017-10-30
* 修改描述 ：1. 使用 V1.1 版 BusRS232.c，增加可设置数据位，校验方式，停止位
*
* 版    本 ：V0.4
* 作    者 ：SKYCODE2013
* 完成日期 ：2017-01-17
* 修改描述 ：1. 更名bus_rs232_idx
*					                          	
*******************************************************************************/

#include "bus_rs232_ID0.h"
#include "BusRS232.h"
#include "Bus.h"
#include "systick.h"


//////////////////////////////// DeBug define //////////////////////////////////
//程序中使用 _DEBUG_RS232_ID0() 来调试代码

//#define __DEBUG_RS232_ID0__ 1  //注释这句 则代码不执_DEBUG_RS232_ID0()

#ifdef __DEBUG_RS232_ID0__
#include "debug_def.h"
#define _DEBUG_RS232_ID0 _DEBUG 
#else
#define _DEBUG_RS232_ID0(format,...)   
#endif

////////////////////////////////////////////////////////////////////////////////


/*接收静态全局變量*/
#define ID0_RX_BUFF_SIZE 32
static uint8_t sg_rx_buff[ID0_RX_BUFF_SIZE];  //USART接收數據包緩存
static unsigned int sg_rx_cnt = 0;
static unsigned int sg_rx_wr_index = 0;
static unsigned int sg_rx_rd_index = 0;


//声明接收中断
static void BusRS232_ID0_RxIrq(u8 rx_data_len, u8* rx_data_buff);

/********************************************************************************
*函數名：BusRS232_ID0_Init
*描述  ：初始化 ID=0的 Bus转RS232模块
*参数  ：无
*返回  ：无
********************************************************************************/
void BusRS232_ID0_Init(void)
{
	BusRS232_Init(BUSRS232_ID0,            //ID = 0
	               BUSRS232_BAUDRATE_38400, //波特率 请用BusRS232.h中宏定义
                   BUSRS232_DATABITS_7BIT,   //数据位 请用BusRS232.h中宏定义
                   BUSRS232_PARIYT_EVEN,     //校验方式 请用BusRS232.h中宏定义
                   BUSRS232_STOPBITS_2,      //停止位  请用BusRS232.h中宏定义
	               BusRS232_ID0_RxIrq);  //接收中断函数
}

/********************************************************************************
*函數名：BusRS232_ID0_SendData
*描述  ：發送RS232数据
*参数  ：1. tx_data_len    - 發送的数据长度
*        2. tx_data_buff   - 指向發送的数据的数组
*返回  ：BUSRS232_NG 發送失败
*        BUSRS232_OK 發送成功
********************************************************************************/
u8 BusRS232_ID0_SendData(u16 tx_data_len, u8* tx_data_buff)
{
    return BusRS232_SendData(BUSRS232_ID0, tx_data_len, tx_data_buff);
}

/********************************************************************************
*函數名：BusRS232_ID0_RdRxData
*描述  ：读取 RS232接收到的数据
*参数  ：1.rd_data_len - 读数据长度
*        2.rd_data_buff - 指向存读取数据的数组
*返回  ：实际读到的数据长度 只会小于等于rd_data_len
********************************************************************************/
u16 BusRS232_ID0_RdRxData(u16 rd_data_len, u8* rd_data_buff)
{
	u16 len_cnt=0;
	u16 i;
	
	Bus_DisableRxIT();
	
	if(sg_rx_cnt >= rd_data_len)
	{
		len_cnt = rd_data_len;
	}
	else
	{
		len_cnt = sg_rx_cnt;
	}
	
	for(i=0; i<len_cnt; i++)
	{
		rd_data_buff[i] = sg_rx_buff[sg_rx_rd_index];
		if(++sg_rx_rd_index >= ID0_RX_BUFF_SIZE){sg_rx_rd_index = 0;}
	}
	
	sg_rx_cnt -= len_cnt;
	
	Bus_EnableRxIT();
	
	return len_cnt;
}

/********************************************************************************
*函數名：BusRS232_ID0_OnLine
*描述  ：Bus转RS232 连线处理，需要不停的调用
*参数  ：无
*返回  ：无
********************************************************************************/
void BusRS232_ID0_OnLine(void)
{
    static unsigned int ts=0;
	unsigned int ret;
	
	if(GetMeasureTime_ms(ts) >= 2000) //每2秒钟检查一次与RS232模块的连接
	{
		ret = BusRS232_Check(BUSRS232_ID0);
		if(ret != BUSRS232_OK) 
		{
//			Delay_10us(4);
			if(BusRS232_Link(BUSRS232_ID0) != BUSRS232_OK)
			{
				_DEBUG_RS232_ID0("BusRS232_Link NG");
			}
			
			_DEBUG_RS232_ID0("BusRS232_Check erro code = 0x%04x", ret);
		}
		else
		{
			_DEBUG_RS232_ID0("BUSRS232_CHECK_OK");
		}
		ts = MeasureTimeStart_ms();
	}
	
}

/********************************************************************************
*函數名：BusRS232_ID0_RxIrq
*描述  ：RS232接收中断函数
*        注意，函数内的程序必须尽量短，不可再调用RS232的發送函数(BusRS232_ID0_SendData())
*参数  ：中断时系统自动带入的
*        1. rx_data_len - 接收到的数据长度
*        2. rx_data_buff - 暂存着接收到的数据，长度由rx_data_len决定
*返回  ：无       
********************************************************************************/
static void BusRS232_ID0_RxIrq(u8 rx_data_len, u8* rx_data_buff)
{
	u8 i=0;
	while(rx_data_len)
	{
		if(sg_rx_cnt < ID0_RX_BUFF_SIZE)
		{
			sg_rx_buff[sg_rx_wr_index] = rx_data_buff[i++];
			if(++sg_rx_wr_index >= ID0_RX_BUFF_SIZE){sg_rx_wr_index = 0;}
			++sg_rx_cnt;
		}
		else
		{
			break; //overflow//溢出
		}
		rx_data_len--;
	}
}

/********************************* SKYCODE ************************************/
