/********************************* SKYCODE *************************************
* Copyright (c) 2013-2019 深圳思凯测试技术有限公司
* All rights reserved.
*
* 文件名   ：ssd2828.c    
* 描述     ：提供一些SSD2828的操作函数
*            1. VIDEO COMMAND 一体
*            2. MIPI_WR() 函数 可不定长写
*            3. WIPI_WrArray() MIPI写一个数组的数据
*            4. Hi8专用
*
* 函数列表 ：
              void SSD2828_RST_H(void);
              void SSD2828_RST_L(void);
			  void SSD2828_CS_0(void);
			  void SSD2828_CS_1(void);
			  void SSD2828_Init(void);
              void SSD2828_Video(void); 
			  void SSD2828_LP(void);
              void SSD2828_HS(void);
			  void SSD2828_GenericLongWrite(unsigned long n);
			  void SSD2828_GenericShortWrite(unsigned char n);
			  void SSD2828_DcsLongWrite(unsigned long n);
			  void SSD2828_DcsShortWrite(unsigned char n);
			  unsigned int SSD2828_DcsReadDT06(uint8_t adr, uint16_t l, uint8_t *p);
			  unsigned int SSD2828_GenericReadDT14(uint8_t adr, uint16_t l, uint8_t *p);
              MIPI_WR(DT,data,...); //不定长写数据
              void MIPI_WrArray(u8 DT, int len, u8 *data_array);
			  void SSD2828_WriteData(unsigned char dat);
			  void SSD2828_WriteCmd(unsigned char cmd);
			  void SSD2828_WriteReg(unsigned char cmd,unsigned char dat1,unsigned char dat2);
*
* 版    本 ：V1.0.0
* 作    者 ：SKYCODE2013
* 完成日期 ：2019-5-30
* 修改描述 ：1. 由 S10 V2.3.0版本修改而来
*            2. 支持8lane
*            3. Dcs Short 0XBF封装方式
*
*
*            						                          	
*******************************************************************************/


#ifndef _SSD2828_H_
#define _SSD2828_H_

#include "sys.h"
#include "SysTick.h"
#include "gpu.h"

/*---------------------------------------------------------------------------*/

/*
*选择Video Mode的 Burst 模式
*选择其中一种模式（不注释）
*/
//#define SSD2828_VIDEO_MODE 0X00 //Non burst mode with sync pulses
//#define SSD2828_VIDEO_MODE 0X01 //Non burst mode with sync events
#define SSD2828_VIDEO_MODE 0X02 //Burst mode


/*
*MIPI 高速模式的数据率（COAMMAND模式的数据率设置）, 只COMMAND模式有效
*/

extern u16 gHS_Mbps;



///////////////////////////////////////////////////////////////////////////////
#define MIPI_READ_FAIL 0X01	          //讀失敗 是通信失敗
#define MIPI_READ_SUCCEED 0X00		  //讀成功 是通信成功，並不能說明讀出來的值就是對的值
							  
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////

void SSD2828_POWER_ON(void);
void SSD2828_POWER_OFF(void);
void SSD2828_RST_H(void);
void SSD2828_RST_L(void);

//void SSD2828_CS_0(void);
//void SSD2828_CS_1(void);
void SSD2828_A_CS_0(void);
void SSD2828_A_CS_1(void);
void SSD2828_B_CS_0(void);
void SSD2828_B_CS_1(void);

void SSD2828_Init(void);
void SSD2828_Config(void);
void SSD2828_LP(void);
void SSD2828_HS(void);
void SSD2828_Video(void);

/*
*函数名 : SSD2828_DcsLongWrite
*描述   : DT = 0x05, 0x15的写操作设置
*参数   : n 写的字节数
*返回   : 无
*/
void SSD2828_DcsShortWrite(u8 len);


/*
*函数名 : SSD2828_DcsLongWrite
*描述   : DT = 0X39的写操作设置
*参数   : n 写的字节数
*返回   : 无
*/
void SSD2828_DcsLongWrite(int len);


/*
*函数名 : SSD2828_GenericShortWrite
*描述   : DT = 0x03, 0x13, 0x23的写操作设置
*参数   : n 写的字节数
*返回   : 无
*/
void SSD2828_GenericShortWrite(int len);


/*
*函数名 : SSD2828_GenericLongWrite
*描述   : DT = 0x29的写操作设置
*参数   : n - 写的字节数
*返回   : 无
*/
void SSD2828_GenericLongWrite(int len);


/*
*函数名 : SSD2828_DcsReadDT06
*描述   : DCS READ, no parameters DT=0X06
*参数   : reg - LCM 驱动IC寄存器地址
          len - 讀的长度 单位Byte
		  p   - 保存读出的数据的数组
*返回   : 读状态 成功或失敗
          MIPI_READ_FAIL 读失敗
		  MIPI_READ_SUCCEED 读成功
*/
unsigned int SSD2828_DcsReadDT06(u8 reg, u16 len, u8 *p);


/*
*函数名 : SSD2828_GenericReadDT14
*描述   : Generic READ, 1 parameter; DT=0X14
*参数   : reg - LCM 驱动IC寄存器地址
          len - 讀的长度 单位Byte
		  p   - 保存读出的数据的数组
*返回   : 读状态 成功或失敗
          MIPI_READ_FAIL 读失敗
		  MIPI_READ_SUCCEED 读成功
*/
unsigned int SSD2828_GenericReadDT14(u8 reg, u16 len, u8 *p);


/*
*函数名 : MIPI_WR
*描述   : 整合了 DT=0X39 0X29 0X05 0X15 0X03 0X13 0X23 的MIPI写操作
*参数   : 1. DT - data type
          2. data,... 不定长的数据，每个数据用逗号间隔 目前最长256个
*返回   : 无
*/
#define MIPI_WR(DT,data,...)  SSD2828_MIPIPackWrite(DT,data,##__VA_ARGS__,-1)


/*
*函数名 : MIPI_WrArray
*描述   : MIPI 写 一个数组的数据
*         整合了 DT=0X39 0X29 0X05 0X15 0X03 0X13 0X23 的MIPI写操作 
*参数   : 1. DT - data type
*         2. len - 数组的长度
          3. data_array - 要MIPI传送的数组
*返回   : 无
*/
void MIPI_WrArray(u8 DT, int len, u8 *data_array);

/*2828底层写操作*/
void SSD2828_WriteData(unsigned char dat);
void SSD2828_WriteCmd(unsigned char cmd);
void SSD2828_WriteReg(unsigned char cmd,unsigned char dat_h,unsigned char dat_l);

/*不是能使用，只能用宏 MIPI_WR()*/
void SSD2828_MIPIPackWrite(u8 DT, int data, ...);

#endif

/********************************* SKYCODE *************************************/
