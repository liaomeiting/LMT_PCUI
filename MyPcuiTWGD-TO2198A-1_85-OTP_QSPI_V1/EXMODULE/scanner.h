/********************************* SKYCODE ************************************
* Copyright (c) 2018, 深圳思凯测试技术有限公司
* All rights reserved.
*
* 文件名   ：scanner.c
* 描述     : 扫码枪驱动程序，需接SCANNER BUS转接盒
*
* 版    本 ：V1.0.2
* 作者     ：
* 完成日期 ：2018-07-26
* 修改描述 ：1.支持虚拟串口的扫码枪
*
* 版    本 ：V1.0.1
* 作者     ：
* 完成日期 ：2018-07-06
* 修改描述 ：扫描结果字符串最后补'\0'
*
* 版    本 ：V1.0.0
* 作者     ：
* 完成日期 ：2018-07-06
* 修改描述 ：
*							                          	
******************************************************************************/

#ifndef _SCANNER_H_
#define _SCANNER_H_

#include "sys.h"

/*
*函数名: SCANNER_Init
*描述  : 初始化驱动程序
*参数  : 无
*返回  : =0 成功
*        =-1 失败
*/
int SCANNER_Init(void);


/*
*函数名: SCANNER_GetStrData
*描述  : 获取扫描到的数据
*参数  : 1. * str - 扫描到的数据，字符串形式
*返回  : >0 扫描到的数据长度
         =0 没有扫描到        
*/
int SCANNER_GetStrData(char* str);





typedef union _CDC_LineCodingStructure
{
  uint8_t Array[7];
  struct
  {
    uint32_t             dwDTERate;     /*波特率*/
    uint8_t              bCharFormat;   /*停止位
    0 - 1 Stop bit
    1 - 1.5 Stop bits
    2 - 2 Stop bits*/
    uint8_t              bParityType;   /* 校验位
    0 - None
    1 - Odd
    2 - Even
    3 - Mark
    4 - Space*/
    uint8_t                bDataBits;     /* 数据位 (5, 6, 7, 8 or 16). */
  }b;
}CDC_LineCodingTypeDef;

/*
*函数名: SCANNER_SendData
*描述  : 发数据到USB串口上
*参数  : 1. *dat 数据指针 2. len数据长度
*返回  : <0 发送失败
         =0 发送成功     
*/
int SCANNER_SendData(char *dat,uint8_t len);

/*
*函数名: SCANNER_GetStatus
*描述  : 获取USB的连接状态
*参数  : 1. *status 返回的状态指针 0->未连接  1->连接
*返回  : <0 发送失败
         =0 发送成功     
*/
int SCANNER_GetStatus(uint8_t *status);

/*
*函数名: SCANNER_SetConfig
*描述  : 配置USB串口参数
*参数  : 1. *Config 配置指针
*返回  : <0 发送失败
         =0 发送成功    
*注意  ：默认是115200bps  1停止位 8数据位 无校验位        
*/
int SCANNER_SetConfig(CDC_LineCodingTypeDef *Config);

#endif

/********************************* SKYCODE ***********************************/
