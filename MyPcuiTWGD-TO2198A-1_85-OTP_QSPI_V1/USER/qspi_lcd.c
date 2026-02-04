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

#include "sys.h"
#include "sysIO.h"
#include "sim_qspi.h"
#include <stdarg.h>
#include "qspi_lcd.h"
#include "gpu.h"

SIM_QSPI_Adapter sg_qspi_adapter;

static u8 sg_cs=0x01; //bit0=1 片选第一个接口； bit1=1 片选第二个接口

static void Delay(void)
{
    Delay_us(0);
}

void QSPI_CS_Set(uint8_t level)
{
    if(level == 0) //低 只控制该控制的
    {
        if(sg_cs&0x01) {PAout(4) = 0;}
    }
    else //高 所有都为高
    {
//        if((sg_cs&0x01)==0){PDout(12) = 1;}
//        if((sg_cs&0x02)==0){PBout(5) = 1;}
//        if((sg_cs&0x04)==0){PCout(3) = 1;}
//        if((sg_cs&0x08)==0){PCout(7) = 1;}
        PAout(4) = level;
    }
    
//        PDout(12) = level;
//        PBout(5) = level;
//        PCout(3) = level;
//        PCout(7) = level;
}

static void QSPI_SCK_Set(uint8_t level)
{
    PDout(13) = level;
}

/*=0 输入,=1输出*/
static void QSPI_IO_Change(uint8_t io)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    if(io)
    {
        /*SI0*/
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;// GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_Init(GPIOD, &GPIO_InitStructure);
		
		/*SI1*/
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;// GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_Init(GPIOD, &GPIO_InitStructure);
		
		/*SI2*/
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		
		/*SI3*/
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
    }
    else
    {
		/*SI0*/
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_OType =GPIO_OType_PP;//  GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_Init(GPIOD, &GPIO_InitStructure);
		
		/*SI1*/
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_OType =GPIO_OType_PP;//  GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_Init(GPIOD, &GPIO_InitStructure);
		
		/*SI2*/
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		
		/*SI3*/
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
    }
}

static void QSPI_SI0_Set(uint8_t level)
{
    PDout(3) = level;
}

static void QSPI_SI1_Set(uint8_t level)
{
    PDout(12) = level;
}

static void QSPI_SI2_Set(uint8_t level)
{
    PAout(0) = level;
}

static void QSPI_SI3_Set(uint8_t level)
{
    PAout(3) = level;
}

static uint8_t QSPI_SI0_Read(void)
{
    return PDin(3);
}

static uint8_t QSPI_SI1_Read(void)
{
    return PDin(12);
}

static uint8_t QSPI_SI2_Read(void)
{
    return PAin(0);
}

static uint8_t QSPI_SI3_Read(void)
{
    return PAin(3);
}



void QSPI_LCD_IO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_GPIOA, ENABLE );
       
	 /*CS*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /*SCL*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
	
    /*SI0*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;// GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    
    /*SI1*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;// GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    
    /*SI2*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /*SI3*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    sg_qspi_adapter.delay = Delay;
    sg_qspi_adapter.set_cs = QSPI_CS_Set;
    sg_qspi_adapter.set_sclk = QSPI_SCK_Set;
    sg_qspi_adapter.change_io = QSPI_IO_Change;
    sg_qspi_adapter.set_io0 = QSPI_SI0_Set;
    sg_qspi_adapter.set_io1 = QSPI_SI1_Set;
    sg_qspi_adapter.set_io2 = QSPI_SI2_Set;
    sg_qspi_adapter.set_io3 = QSPI_SI3_Set;
    sg_qspi_adapter.read_io0 = QSPI_SI0_Read;
    sg_qspi_adapter.read_io1 = QSPI_SI1_Read;
    sg_qspi_adapter.read_io2 = QSPI_SI2_Read;
    sg_qspi_adapter.read_io3 = QSPI_SI3_Read;
    
  QSPI_CS_Set(1);
}

static uint8_t pack_data[260];
void QSPI_LCD_PackWrite(uint8_t cmd, int data, ...)
{
    va_list param_list;
    int in_data;
    unsigned int len=0;
    
    va_start(param_list, data);
    
    pack_data[len++] = 0x02;
    pack_data[len++] = 0x00;
    pack_data[len++] = cmd;
    pack_data[len++] = 0x00;
    pack_data[len++] = data;
    
    while(1)
    {
        in_data = va_arg(param_list, int);
        if(in_data == -1)break;
        pack_data[len++] = in_data;
    }
        
    if(len>256)return; 
    
    QSPI_1W_ReadWriteData(&sg_qspi_adapter, len, pack_data,0,0); 
	

    va_end(param_list);
}


void QSPI_LCD_ReadData(uint8_t addr, unsigned int len, uint8_t *rd_data)
{
    uint8_t wr_data[4];
    
    wr_data[0] = 0x03;
    wr_data[1] = 0x00;
    wr_data[2] = addr;
    wr_data[3] = 0x00;
    
    QSPI_1W_ReadWriteData(&sg_qspi_adapter, 4, wr_data,len,rd_data); 
}

void QSPI_LCD_ReadDataFT(uint8_t addr, uint8_t addr2,unsigned int len, uint8_t *rd_data)
{
    uint8_t wr_data[4];
    
    wr_data[0] = 0x03;
    wr_data[1] = 0x00;
    wr_data[2] = addr;
    wr_data[3] = addr2;
    
    QSPI_1W_ReadWriteData(&sg_qspi_adapter, 4, wr_data,len,rd_data); 
}

/*
其实这此的屏不能跳坐标为，为了对应in_img2的程序才写成这样
*/
void QSPI_LCD_GotoXY(unsigned int xs, unsigned int ys, unsigned int xe, unsigned int ye)
{
    // QSPI_LCD_WriteCmd(0xFE, 0x00);
    // QSPI_LCD_WriteCmd(0x2A, 0x00, 0x00, 0x01, 0x3F);		
	// QSPI_LCD_WriteCmd(0x2B, 0x00, 0x00, 0x01, 0x67);		
	QSPI_1W_WriteByte(&sg_qspi_adapter,0x12);
	QSPI_4W_WriteByte(&sg_qspi_adapter,0x00);
    QSPI_4W_WriteByte(&sg_qspi_adapter,0x2C);
    QSPI_4W_WriteByte(&sg_qspi_adapter,0x00);
											 
											 
}

/*写像素的函数*/
void QSPI_LCD_WriteRGB(uint32_t rgb)
{
    QSPI_4W_WriteByte(&sg_qspi_adapter, rgb>>16);
    QSPI_4W_WriteByte(&sg_qspi_adapter, rgb>>8);
    QSPI_4W_WriteByte(&sg_qspi_adapter, rgb);
    // QSPI_4W_WriteByte(&sg_qspi_adapter, ((rgb>>16)&0xF8) | (((rgb>>8)&0xE0)>>5));
    // QSPI_4W_WriteByte(&sg_qspi_adapter, (((rgb>>8)&0x1C)<<3) | ((rgb&0xF8)>>3));
}

/*
*函数名 : QSPI_LCD_A_CS 等等
*描述   : 下面四个函数是片选设置，不会马上控制CS引脚，用于写指令 和读指令的应用，不能用于写图像时的片选.
*          指明在之后的写 读命令操作中哪个接口有效。
*参数   : 1. level - CS脚输出的电平
*返回   : 无
*/
void QSPI_LCD_A_CS(uint8_t level)
{
    if(level)
    {
        sg_cs &= (~0x01);
    }
    else
    {
        sg_cs |= (0x01);
    }
}

void QSPI_LCD_B_CS(uint8_t level)
{
    if(level)
    {
        sg_cs &= (~0x02);
    }
    else
    {
        sg_cs |= (0x02);
    }
}

//void QSPI_LCD_C_CS(uint8_t level)
//{
//    if(level)
//    {
//        sg_cs &= (~0x04);
//    }
//    else
//    {
//        sg_cs |= (0x04);
//    }
//}

//void QSPI_LCD_D_CS(uint8_t level)
//{
//    if(level)
//    {
//        sg_cs &= (~0x08);
//    }
//    else
//    {
//        sg_cs |= (0x08);
//    }
//}


/*
*函数名 : QSPI_LCD_CS_AllOut
*描述   : 所有片选一起控制马上输出.用于写图像用。读写命令不要用这个函数
*参数   : 1. cs_bit - 按位片选 用QSPI_LCD_A_CS_BIT, QSPI_LCD_B_CS_BIT, QSPI_LCD_C_CS_BIT, QSPI_LCD_D_CS_BIT宏定义，可按位或
*         2. level - CS脚输出的电平
*返回   :无
*/
void QSPI_LCD_CS_AllOut(uint8_t cs_bit, uint8_t level)
{
    if(cs_bit & QSPI_LCD_A_CS_BIT){PAout(4) = level;}
//    if(cs_bit & QSPI_LCD_B_CS_BIT){PBout(5)= level;}
//    if(cs_bit & QSPI_LCD_C_CS_BIT){PCout(3) = level;}
//    if(cs_bit & QSPI_LCD_D_CS_BIT){PCout(7) = level;}
}

/*
*函数名 : QSPI_LCD_WrByte
*描述   : 写一个字节的函数 SD卡程序用到
*参数   : 
*返回   :无
*/
void QSPI_LCD_WrByte(uint8_t dat)
{
    QSPI_4W_WriteByte(&sg_qspi_adapter, dat);
}


