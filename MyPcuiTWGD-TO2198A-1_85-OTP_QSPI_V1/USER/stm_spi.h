/********************************* SKYCODE *************************************
* Copyright (c) 2014-2016, 深圳思凯测试技术有限公司（广州天码电子技术有限公司）
* All rights reserved.
*
* 文件名   ：stm_spi.h    
* 描述     ： 此程序是虚拟SPI的
*             
*
* 當前版本 ：V1.0
* 作    者 ：SKYCODE2013
* 完成日期 ：2016-09-24
* 修改描述 ：
*            					                          	
*******************************************************************************/
#ifndef _STM_SPI_H_
#define _STM_SPI_H_

#include "sys.h"
#include "SysTick.h"
#include "sysio.h"
#include "S10.h"

#define SPI_SCK PBout(3)	//SCL  时钟
#define SPI_SDI PBout(5)	//SDI  写数据   
#define SPI_CSX PAout(4)	//CSX  使能片选
#define SPI_DCX PBout(6)	//DCX  写数据/写指令
#define SPI_RSDI PBin(4)	//SDO  读数据
#define SPI_Write(DT,data,...)  SPI_SendX(DT,data,##__VA_ARGS__,-1)

void SPI_SendX(unsigned char cmd, unsigned char data,...);
void SPI_SendArr(unsigned char cmd, const unsigned char *data_array, unsigned int len);
void Spi_Init(void);
void SPI_Read_Buffer(unsigned char cmd, uint16_t *buffer, unsigned int length);
void SPI_Write1(unsigned char cmd, unsigned char data);

unsigned char SPI_Read(unsigned char cmd);

void SPI_Write_4data(unsigned char cmd, unsigned char data1, unsigned char data2, unsigned char data3, unsigned char data4);

void SPI_Write_cmd(unsigned char cmd);

void SPI_Send_Dat(unsigned char dat);
unsigned char SPI_Receiver_Dat(void);

void SPI_RGB(u32 rgb);

void SPI_DispArea(unsigned int xs, unsigned int ys, 
                  unsigned int xe, unsigned int ye);
void SPI_Send_Dat1(unsigned char dat);
#endif

/********************************* SKYCODE ************************************/
