/********************************* SKYCODE ************************************
* Copyright (c) 2018-2019, 深圳思凯测试技术有限公司
* All rights reserved.
*
* 文件名   ：uartKey.c
* 描述     : 1. 1to2 按键盒驱动程序
*            2. COM口驱动
*
* 日   期 ：2023-11-7
* 修 改     ：1.增加测量CA310色温功能
*
*
* 版    本 ：V1.3.0
* 作者     ：1. 增加通过串口读取镜头，注意:两个机子及其以上不能同一时间读取设置
*
* 版    本 ：V1.2.0
* 作者     ：1. 增加ca310各种功能。

* 版    本 ：V1.1.0
* 作者     ：1. 增加COM发送数据函数。
*            
* 完成日期 ：2019-09-03
*
* 版    本 ：V1.0.0
* 作者     ：
* 完成日期 ：2019-01-16
*							                          	
******************************************************************************/

#ifndef _UARTKEY_H_
#define _UARTKEY_H_

#include "commonIOInterface.h"
#include "commonCa310.h"
#include "cfl.h"

typedef struct
{
	Ca310Fun  ca310Fun;
	
	/**************使用到的硬件驱动***************/
	sky_comDriver *driver;
}uartKeyInterface;

uartKeyInterface* uartKeyBindDriver(sky_comDriver*  driver);

//键值宏定义
#define UARTKEY_NULL 0x00 //无按键
#define UARTKEY_ON   0x10
#define UARTKEY_OFF  0x11
#define UARTKEY_INC  0x12 //+
#define UARTKEY_DEC  0x13 //-
#define UARTKEY_DOWN 0x02 //下翻
#define UARTKEY_UP   0x04 //上翻
#define UARTKEY_ENTER 0x08 //确认
#define UARTKEY_BACKUP 0x14 //备用

/*
     按键位置定义
                |备用|
 |确认| | 加 |  | 减 |

 |输出| |下翻|  |上翻|
*/


/*
*函数名: uartKey_Init
*描述  : 初始化按键驱动程序
*参数  : 无
*返回  : =0 成功
*        =-1 失败
*/
int uartKey_Init(void);

/*
*函数名: uartKey_Read
*描述  : 回读按键值
*参数  : 无
*返回  : 回读到的键值。键值请用上面宏定义
*        -1无键值
*/
int uartKey_Read(void);

/*
*函数名: uartKey_ReadPN
*描述  : 回读板子编号（接通讯板UART1的编号=0，接UART2的编号=1...）
*参数  : 无
*返回  : 板子编号
*        -1无键值
*/
int uartKey_ReadPN(void);

/*
*函数名: COM_ReadData
*描述  : 回读COM数据
*参数  : *data - 保存回读到的数据
*        len  - 读的长度
*返回  : 实际数据长度
*/
int COM_ReadData(u8 *data, int len);

/*
*函数名: COM_SendData
*描述  : 从COM发送数据
*参数  : *data - 数据
*        len  - 长度
*返回  : >=0成功
*/
int COM_SendData(u8 *data, int len);

/*
*函数名: COM_Printf
*描述  : 从COM口格式化输出
*参数  : 同printf
*返回  : >=0成功
*/
int COM_Printf(const char *fmt, ...);

/*
*函数名: COM_SetMode
*描述  : 设置com口功能模式 
*参数  : 功能模式  0 数据传输模式 1 ca310通信模式
*返回  : >=0成功
*/
int COM_SetMode(u8 mode);

/********************************************************************************
*函數名：COM_CA310_Init
*描述  ：初始化 CA310,
*参数  ：ch   选择哪个内存通道 -1默认 0~99
*		 sync  同步模式  -1：默认  0：NTSC 1：PAL mode 2：EXT mode 3：UNIV mode
*返回  ：正确返回1  负数为错误码 
********************************************************************************/
int COM_CA310_Init(int ch,int sync);

/********************************************************************************
*函數名：COM_CA310_GetFMA
*描述  ：从CA310获取数据  
*参数  ：PN  :读取哪些通道  数据bit0~bit4  表示1~5通道。这个参数指的是CA310主机扩展探头的编号。
*		FMA的地址  返回到数据会保存在其中
*返回  ：正确返回1  负数为错误码  
*使用模板 if((mlen=CA310_GetFMA(1,&FMA))>0)
********************************************************************************/
int COM_CA310_GetFMA(u8 P,float* FMA);

/********************************************************************************
*函數名：COM_CA310_GetLvXY
*描述  ：从CA310获取数据  
*参数  ：PN  :读取哪些通道  数据bit0~bit4  表示1~5通道
*		 Lv， X， Y的地址  返回到数据会保存在其中
*返回  ：正确返回1  负数为错误码  
*使用模板 if((mlen=CA310_GetLvXY(1,&Lv,&X,&Y))>0)
********************************************************************************/
int COM_CA310_GetLvXY(u8 P,float* Lv,float* X,float* Y);

/********************************************************************************
*函數名：COM_CA310_GetTcpduvLv
*描述  ：从CA310获取数据  
*参数  ：PN  :读取哪些通道  数据bit0~bit4  表示1~5通道
*		 Tcp,duv,Lv的地址  返回到数据会保存在其中
*返回  ：正确返回1  负数为错误码  
********************************************************************************/
int COM_CA310_GetTcpduvLv(uint8_t P,float* Tcp,float* duv,float* Lv);


/**
  * 功能描述 : 查询探头唯一序列号
  * 输入参数 : 1. P -- 默认为1
  *              [CA-310]多探头输出时,P = (1..5)
  * 输出参数 : 1. id -- 镜头编号  值域 00000000 ~99999999
  * 返 回 值 : = 0 -- 没有此功能, = 1 -- 查询成功, < 0 -- 查询失败
  */
int COM_CA310_GetSerialNO(uint8_t P,int *id);





/*
*函数名: uartKey_GetnumberGroup
*描述  : 设置数据组  （用于内测）
*参数  : 
    1. dataGroup : 大小为256的数组
*返回  : =0 成功，<0 失败 
*/
int uartKey_SetnumberGroup(u16 addr,u8 *dataGroup,int len);



/*
*函数名: uartKey_GetnumberGroup
*描述  : 回读数据组  （用于内测）
*参数  : 
    1. dataGroup : 大小为256的数组 ，返回的数据值
*返回  : =0 成功，<0 失败 
*/
int uartKey_GetnumberGroup(u16 addr,u8 *dataGroup,int len);


/*函数名: uartKey_CFL_Setuartnumber
*描述  : 设置拖多少个串口（应该是指有多少个测试机接到了通讯板上，且和有没CFL无关）
*参数  : 
    1. uartnumber : 1 2 3 4 最大值为4
*返回  : =0 成功，<0 失败 
*/
int uartKey_CFL_Setuartnumber(u8 uartnumber);


#endif
