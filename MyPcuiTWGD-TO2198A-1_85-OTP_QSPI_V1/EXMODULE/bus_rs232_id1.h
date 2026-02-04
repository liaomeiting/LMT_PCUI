/********************************* SKYCODE *************************************
* Copyright (c) 2016-2022, 深圳思凯测试技术有限公司（广州天码电子技术有限公司）
* All rights reserved.
*
* 文件名   ：bus_rs232_id4.h   
* 描述     ：Bus To RS232模块应用程序，实现RS232通讯 模块ID=0
* 函数列表 ：
*             void BusRS232_ID1_Init(void);
*			  u8 BusRS232_ID1_SendData(u16 tx_data_len, u8* tx_data_buff);
*			  u16 BusRS232_ID1_RdRxData(u16 rd_data_len, u8* rd_data_buff);
*			  void BusRS232_ID1_OnLine(void);
*
* 版    本 ：
* 作    者 ：
* 完成日期 ：2022-7-7
* 修改描述 ：1.DCPHY,用于转接310 410的通讯
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

#ifndef _USER_RS232_ID1_H_
#define _USER_RS232_ID1_H_

#include "sys.h"
#include "commonIOInterface.h"

sky_comDriver* CA310_BusRS232_ID1_Init(void);

/*
*函數名：BusRS232_ID1_Init
*描述  ：初始化 ID=0的 mBus转RS232模块
*参数  ：无
*返回  ：无
*/
void BusRS232_ID1_Init(void);

/*
*函數名：BusRS232_ID1_SendData
*描述  ：發送RS232数据
*参数  ：1. tx_data_len    - 發送的数据长度
*        2. tx_data_buff   - 指向發送的数据的数组
*返回  ：无
*/
u8 BusRS232_ID1_SendData(u16 tx_data_len, u8* tx_data_buff);

/*
*函數名：BusRS232_ID1_RdRxData
*描述  ：读取 RS232接收到的数据
*参数  ：1.rd_data_len - 读数据长度
*        2.rd_data_buff - 指向存读取数据的数组
*返回  ：实际读到的数据长度 只会小于等于rd_data_len
*/
u16 BusRS232_ID1_RdRxData(u16 rd_data_len, u8* rd_data_buff);

/*
*函數名：BusRS232_ID1_OnLine
*描述  ：mBus转RS232 连线处理，需要不停的调用
*参数  ：无
*返回  ：无
*/
void BusRS232_ID1_OnLine(void);


#endif
