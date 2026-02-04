/********************************* SKYCODE *************************************
* Copyright (c) 2020, 深圳思凯测试技术有限公司
*
* 文件名   ：qspi_lcd.c
* 描述     ：QSPI接口的显示屏的驱动程序         
*
* 版    本 ：V1.0.0
* 作    者 ：ma_xiaoteng
* 完成日期 ：2020-6-3
* 修改描述 ：          
*******************************************************************************/


#ifndef _QSPI_LCD_
#define _QSPI_LCD_

#include "sys.h"

/**/
#define QSPI_LCD_A_CS_BIT 0x01
#define QSPI_LCD_B_CS_BIT 0x02
#define QSPI_LCD_C_CS_BIT 0x04
#define QSPI_LCD_D_CS_BIT 0x08

/*
*函数名 : QSPI_LCD_IO_Init
*描述   : 初始化IO，很重要
*参数   : 
*返回   :
*/
void QSPI_LCD_IO_Init(void);


/*不要调用这个函数*/
void QSPI_LCD_PackWrite(uint8_t cmd, int data, ...);

/*
*函数名 : QSPI_LCD_WriteCmd
*描述   : 写命令用的，用于写初始化代码。
*参数   : 写初始化的指令和参数
*返回   :无
*/
#define QSPI_LCD_WriteCmd(cmd,data,...)  QSPI_LCD_PackWrite(cmd,data,##__VA_ARGS__,-1)

void QSPI_LCD_PackWrite_2(uint8_t cmd, uint16_t *data, uint32_t data_len);

/*
*函数名 : QSPI_LCD_ReadData
*描述   : 读
*参数   : 1. addr - 读地址
*         2. len  - 读长度
*         3. rd_data - 读的数据 存到数组
*返回   :无
*/
void QSPI_LCD_ReadData(uint8_t addr, unsigned int len, uint8_t *rd_data);

/*
*函数名 : QSPI_LCD_GotoXY
*描述   : 跳坐标函数，其实这此的屏不能跳坐标为，为了对应in_img2的程序才写成这样
*参数   : 1. xs 
*         2. ys - 
*         3. xe - 
*         4. ye - 
*返回   :无
*/
void QSPI_LCD_GotoXY(unsigned int xs, unsigned int ys, unsigned int xe, unsigned int ye);


/*写像素的函数*/
void QSPI_LCD_WriteRGB(uint32_t rgb);


/*
*函数名 : QSPI_LCD_A_CS 等等
*描述   : 下面四个函数是片选设置，不会马上控制CS引脚，用于写指令 和读指令的应用，不能用于写图像时的片选.
*          指明在之后的写 读命令操作中哪个接口有效。
*参数   : 1. level - CS脚输出的电平
*返回   : 无
*/
void QSPI_LCD_A_CS(uint8_t level);
void QSPI_LCD_B_CS(uint8_t level);
void QSPI_LCD_C_CS(uint8_t level);
void QSPI_LCD_D_CS(uint8_t level);

/*
*函数名 : QSPI_LCD_CS_AllOut
*描述   : 所有片选一起控制马上输出.用于写图像用。读写命令不要用这个函数
*参数   : 1. cs_bit - 按位片选 用QSPI_LCD_CS_A QSPI_LCD_CS_B QSPI_LCD_CS_C QSPI_LCD_CS_D宏定义，可按位或
*         2. level - CS脚输出的电平
*返回   :无
*/
void QSPI_LCD_CS_AllOut(uint8_t cs_bit, uint8_t level);

/*
*函数名 : QSPI_LCD_WrByte
*描述   : 写一个字节的函数 SD卡程序用到
*参数   : 
*返回   :无
*/
void QSPI_LCD_WrByte(uint8_t dat);

#endif

/********************************* SKYCODE ************************************/

