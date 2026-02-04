/********************************* SKYCODE ************************************
* Copyright (c) 2016-2017, 深圳思凯测试技术有限公司（广州天码电子技术有限公司）
* All rights reserved.
*
* 文件名   ：BusCCD.c    
* 描述     ：1.基于 BUS 1.0 总线的镜頭通讯
*            2.本程序是主机端程序
*
* 版    本 ：V0.1_F407
* 作    者 ：SKYCODE2013
* 完成日期 ：2017-12-5
* 修改描述 ：1.适用于F407平台
*
* 版    本 ：V0.1
* 作    者 ：SKYCODE2013
* 完成日期 ：2017-3-28
*
*					                          	
******************************************************************************/


#ifndef _CCD_BUS_H_
#define _CCD_BUS_H_

#include "stm32f4xx.h"

//BUSCCD 接收数据结构
typedef struct
{
    bool rx_ok;
    u8 id;
    u8 cmd;
    u8 data_len;
    u8 data[8];
}BusCCDRxStruct;


extern BusCCDRxStruct g_ccd_rx_array[8];

//
#define BUSCCD_OK 0x00
#define BUSCCD_NG 0x01

/*
*函数名：BusCCD_Init
*描述  ：BUSCCD 發送指令
*参数  ：1. 镜頭ID
*返回  ：BUS接收过滤器编号 0 ~ (BUS_MAX_FILTER-1)
*       =BUS_ADDRXMLD_ERROR  错误 没有要过滤的
*       =BUS_ADDRXMLD_REPEAT 重复添加    
*		=BUS_ADDRXMLD_NOFILT 滤波器用完，增加失败  
*/
extern u8 BusCCD_Init(u8 ccd_id);

/*
*函数名：BusCCD_Send
*描述  ：BUSCCD 發送指令
*参数  ：1. 镜头ID
*        2. cmd - 指令
*        3. data_len - 参数长度
*        4. data_buff - 指向要发送的数据
*返回  ：BUSCCD_OK 發送OK; BUSCCD_NG 發送失败
*/
extern u8 BusCCD_Send(u8 ccd_id, u16 cmd, u8 data_len, u8* data_buff);

#endif

/********************************* SKYCODE ***********************************/
