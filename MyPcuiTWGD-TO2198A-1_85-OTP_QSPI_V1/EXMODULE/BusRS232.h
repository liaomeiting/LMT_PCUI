/********************************* SKYCODE *************************************
* Copyright (c) 2015-2017, 深圳思凯测试技术有限公司（广州天码电子技术有限公司）
* All rights reserved.
*
* 文件名   ：BusRS232.h    
* 描述     ：BUS 1.0 协议 转 RS232模块通讯驱动程序
* 函数列表 ：
*             unsigned int BusRS232_Init(u8 bus_rs232_idx, 
                  u8 baud_rate, u8 data_bits, u8 parity, u8 stop_bits,
				  void (*user_rs232_rx_irq_handler)(u8 rx_data_len, u8* rx_data_buff));
			  unsigned int BusRS232_SendData(u8 bus_rs232_idx, u16 tx_data_len, u8* tx_data_buff);
			  unsigned int BusRS232_Check(u8 bus_rs232_idx);
			  unsigned int BusRS232_OnLine(u8 bus_rs232_idx);
*
* 版    本 ：V1.1.1
* 作    者 ：
* 完成日期 ：2018-10-17
* 修改描述 ：1.S10专用
*
* 版    本 ：V1.1
* 作    者 ：SKYCODE2013
* 完成日期 ：2017-10-30
* 修改描述 ：1.增加可设置RS232的 数据位，校验方式，停止位
              RS232可配置的数据格式 
               位数  奇偶检验    停止位
                7     奇或偶	    0.5、1、1.5、2
                8     无检验     0.5、1、1.5、2
                8	  奇或偶       1           
*
* 版    本 ：V1.0
* 作    者 ：SKYCODE2013
* 完成日期 ：2016-12-15
* 修改描述 ：1. BUS 1.0 协议 转 RS232
*            2. 增加发送忙判断，发送256个字节就读忙一次
*            3. 波特率最小只到 9600
*
* 版    本 ：V0.2
* 作    者 ：SKYCODE2013
* 完成日期 ：2016-04-27
* 修改描述 ：修改各函数返回值宏定义
*
* 版    本 ：V0.1
* 作    者 ：SKYCODE2013
* 完成日期 ：2016-04-02
* 修改描述 ：
*					                          	
*******************************************************************************/

#ifndef _BUS_RS232_
#define _BUS_RS232_

#include "sys.h"
#include "systick.h"

//Bus RS232 ID 定义
#define BUSRS232_ID0 0
#define BUSRS232_ID1 1
#define BUSRS232_ID2 2
#define BUSRS232_ID3 3
#define BUSRS232_ID4 4

//函数返回值
#define BUSRS232_OK 0X00
#define BUSRS232_NG 0X01 

//忙碌宏定義
#define BUSRS232_BUSY 0x02

//Bus RS232 波特率宏定义
#define BUSRS232_BAUDRATE_9600 0X02
#define BUSRS232_BAUDRATE_14400 0X03
#define BUSRS232_BAUDRATE_19200 0X04
#define BUSRS232_BAUDRATE_38400 0X05
#define BUSRS232_BAUDRATE_56000 0X06
#define BUSRS232_BAUDRATE_57600 0X07
#define BUSRS232_BAUDRATE_115200 0X08

//数据位宏定义
#define BUSRS232_DATABITS_7BIT  0X00
#define BUSRS232_DATABITS_8BIT  0X01

//校验方式宏定义
#define BUSRS232_PARITY_NONE    ((uint16_t)0x00)
#define BUSRS232_PARIYT_EVEN    ((uint16_t)0x04) //偶校验
#define BUSRS232_PARIYT_ODD     ((uint16_t)0x06) //奇校验

//停止位宏定义
#define BUSRS232_STOPBITS_0_5   ((uint16_t)0x01) //0.5位停止位
#define BUSRS232_STOPBITS_1     ((uint16_t)0x00) //1
#define BUSRS232_STOPBITS_1_5   ((uint16_t)0x03) //1.5
#define BUSRS232_STOPBITS_2     ((uint16_t)0x02) //2

/*
*函數名：BusRS232_Init
*描述  ：初始化Bus转RS232模块
*        RS232可配置的数据格式 
           位数  奇偶检验    停止位
            7     奇或偶	    0.5、1、1.5、2
            8     无检验     0.5、1、1.5、2
            8	  奇或偶       1
*参数  ：1. bus_rs232_idx - Bus转RS232模块的ID
*        2. baud_rate - 波特率,请用上面的宏定义，如BUSRS232_BAUDRATE_9600
*        3. data_bits - 数据位，7bit 或 8bit, 请用上面的宏定义
*        4. parity    - 校验方式, 请用上面的宏定义
*        5. stop_bits - 停止位，请用上面的宏定义
*        6. *user_rs232_rx_irq_handler 接收中断函数
*返回  ：BUSRS232_NG - 初始化失败
*        BUSRS232_OK - 初始化成功
*/
unsigned int BusRS232_Init(u8 bus_rs232_idx, 
                  u8 baud_rate, u8 data_bits, u8 parity, u8 stop_bits,
				  void (*user_rs232_rx_irq_handler)(u8 rx_data_len, u8* rx_data_buff));

/*
*函數名：BusRS232_SendData
*描述  ：Bus RS232模块 發送数据
*参数  ：1. bus_rs232_idx - Bus转RS232模块的ID
*        2. tx_data_len    - 發送的数据长度
*        3. tx_data_buff   - 指向發送的数据的数组
*返回  ：BUSRS232_NG 發送失败
*        BUSRS232_OK 發送成功
*/
u8 BusRS232_SendData(u8 bus_rs232_idx, u16 tx_data_len, u8* tx_data_buff);


/*
*函數名：BusRS232_Check
*描述  ：获取状态
*参数  ：bus_rs232_idx - Bus转RS232模块的ID
*返回  ：BUSRS232_OK - 模块状态OK
*        非0值为错误代码
*/
unsigned int BusRS232_Check(u8 bus_rs232_idx);

/*
*函數名：BusRS232_Busy
*描述  ：讀忙
*参数  ：bus_rs232_idx - Bus转RS232模块的ID
*返回  ：BUSRS232_OK - 模块状态OK
*        BUSRS232_NG - 通訊NG
*        BUSRS232_BUSY - 忙碌       
*/
u8 BusRS232_Busy(u8 bus_rs232_idx);

/*
*函數名：BusRS232_OnLine
*描述  ：与Bus转RS232模块连接
*参数  ：bus_rs232_idx - Bus转RS232模块的ID
*返回  ：BUSRS232_NG - 连接失败
*        BUSRS232_OK - 连接完成 并不指一定成功 咔咔
*/
unsigned int BusRS232_Link(u8 bus_rs232_idx);

#endif

/********************************* SKYCODE ************************************/
