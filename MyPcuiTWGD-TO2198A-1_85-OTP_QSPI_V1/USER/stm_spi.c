/********************************* SKYCODE *************************************
* Copyright (c) 2014-2016, 深圳思凯测试技术有限公司（广州天码电子技术有限公司）
* All rights reserved.
*
* 文件名   ：stm_spi.h    
* 描述     ： 此程序是虚拟spi的
*           
*
* 當前版本 ：V1.0
* 作    者 ：SKYCODE2013
* 完成日期 ：2016-09-24
* 修改描述 ：
*						                          	
*******************************************************************************/

#include "stm_spi.h"
#include <stdarg.h>

#define _Time    10         //延时时间 us


#define _CPOL     0
#define _CPHA     0



void SPI_Send_Dat(unsigned char dat);
unsigned char SPI_Receiver_Dat(void);


void SPI_delay(void)
{
   uint16_t i=0;
//    while(i) 
//    { 
//      i--; 
//    }  
}

/************************************************
        端口方向配置  与输出初始化
************************************************/
void Spi_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOB, ENABLE); 
    
    //CSX  使能片选
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;	
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;   
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; 
    GPIO_Init(GPIOA, &GPIO_InitStructure);	 
    
    //DCX  写指令 或者 写数据
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;	
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;   
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; 
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    //SCL 时钟
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;	
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;   
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; 
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    //SDI 数据
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;   
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    //SDO 数据
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    
    SPI_SDI=0;
    SPI_CSX=1;
    SPI_DCX=0;
    #if _CPOL==0
    SPI_SCK=0;
    #else
    SPI_SCK=1;
    #endif
}


void SPI_Write1(unsigned char cmd, unsigned char data)
{
    SPI_CSX=0;
    
    SPI_DCX=0;
    SPI_delay();
    SPI_Send_Dat(cmd);
    
    SPI_DCX=1;
    SPI_delay();
    SPI_Send_Dat(data);
    
    SPI_CSX=1;
    SPI_delay();
//    SPI_delay();
}

unsigned char SPI_Read(unsigned char cmd)
{
    unsigned char data;
    
    SPI_DCX=0;
    SPI_CSX=0;
    

	SPI_delay();
    SPI_Send_Dat(0xf1);	
    SPI_Send_Dat(cmd);	
    SPI_CSX=0;
    
    SPI_delay();

	SPI_DCX=1;
	
	SPI_SCK=0;
	SPI_delay();
	SPI_SCK=1;
	SPI_delay();
    data = SPI_Receiver_Dat();
    // data = SPI_Receiver_Dat();
	SPI_delay();
    
    SPI_CSX=1;
    SPI_delay();
    
    
    return data;
}
void SPI_Read_Buffer(unsigned char cmd, uint16_t *buffer, unsigned int length)
{
    unsigned int i;
    
    // 发送命令阶段
    SPI_DCX = 0;    // 命令模式
    SPI_CSX = 0;    // 片选使能
    
    SPI_delay();
    SPI_Send_Dat(0xf1);	
    SPI_Send_Dat(cmd);  // 发送命令字节
    
    // 准备读取数据
    SPI_CSX = 0;    // 保持片选有效
    SPI_DCX = 1;    // 切换到数据模式

	SPI_SCK=0;
	SPI_delay();
	SPI_SCK=1;
	SPI_delay();
    // SPI_Receiver_Dat();//8bit dummy
    // 循环读取指定长度的数据
    for(i = 0; i < length; i++)
    {
        // 如果需要时钟同步，可取消下面的注释
        // SPI_SCK = 0;
        // SPI_delay();
        // SPI_SCK = 1;
        // SPI_delay();
        
        buffer[i] = SPI_Receiver_Dat();
        SPI_delay();
    }
    
    // 结束传输
    SPI_CSX = 1;    // 片选禁用
    SPI_delay();
}
void SPI_Write_4data(unsigned char cmd, unsigned char data1, unsigned char data2, unsigned char data3, unsigned char data4)
{
    SPI_CSX=0;
    
    SPI_DCX=0;
    SPI_delay();
    SPI_Send_Dat(cmd);
    
    SPI_DCX=1;
    SPI_delay();
    SPI_Send_Dat(data1);
    
    SPI_Send_Dat(data2);
    
    SPI_Send_Dat(data3);
    
    SPI_Send_Dat(data4);
    
    SPI_CSX=1;
    SPI_delay();
//    SPI_delay();
    
}
static uint8_t spi_pack_data[260];
void SPI_SendX(unsigned char cmd, unsigned char data,...)
{
    va_list param_list;
    int in_data;
    unsigned int len=0,i;
    
    va_start(param_list, data);
    
    spi_pack_data[len++] = data;
    
    while(1)
    {
        in_data = va_arg(param_list, int);
        if(in_data == -1)break;
        spi_pack_data[len++] = in_data;
    }
 
    if(len>256)return; 
	
    SPI_CSX=0;
    
    SPI_DCX=0;
    SPI_delay();		
    SPI_Send_Dat(cmd);	
    SPI_CSX=0;
   
    SPI_DCX=1;
    SPI_delay();
    for(i=0; i<len; i++)
    {
        SPI_Send_Dat(spi_pack_data[i]);  
    }
    SPI_CSX=1;
    // SPI_DCX=0;
   SPI_delay(); 
    

    va_end(param_list);	

}

void SPI_SendArr(unsigned char cmd, const unsigned char *data_array, unsigned int len)
{
    unsigned int i;
    
    // 参数检查
    if (data_array == NULL || len == 0 || len > 256) {
        return;
    }
    
    SPI_CSX = 0;
    
    // 发送命令
    SPI_DCX = 0;
    SPI_delay();		
    SPI_Send_Dat(cmd);	
    SPI_CSX = 0; 
    
    // 发送数据
    SPI_DCX = 1;
    SPI_delay();
    for (i = 0; i < len; i++) {
        SPI_Send_Dat(data_array[i]);  
    }
    
    // 结束传输
    SPI_CSX = 1;
    SPI_DCX = 0;
    SPI_delay(); 
}

void SPI_Write_cmd(unsigned char cmd)
{
    SPI_CSX=0;
    
    SPI_DCX=0;
    SPI_delay();
    SPI_Send_Dat(cmd);
    
    SPI_CSX=1;
    SPI_DCX=1;
    SPI_delay();
//    SPI_delay();
    
}


/**********************************************
模式零           写数据
***********************************************/
#if _CPOL==0&&_CPHA==0          //MODE   0  0   
void SPI_Send_Dat(unsigned char dat)
{
    unsigned char n;
    for(n=0;n<8;n++)
    {
        SPI_SCK=0;
        SPI_delay();
        if(dat&0x80)
            SPI_SDI=1;
        else 
            SPI_SDI=0;
        SPI_delay();
        dat<<=1;
        SPI_SCK=1;
        SPI_delay();
    }
    // SPI_SCK=0;
}
void SPI_Send_Dat1(unsigned char dat)
{
    unsigned char n;
	
   for(n=0;n<8;n++)
    {
        SPI_SCK=0;
        if(dat&0x80)
			SPI_SDI=1;
        else 
			SPI_SDI=0;
        SPI_delay();
        dat<<=1;
        SPI_SCK=1;
        SPI_delay();
    }
	SPI_SDI=0;
    SPI_SCK=1;
	
}
/*********************************************
模式零         读数据
*********************************************/
unsigned char SPI_Receiver_Dat(void)
{
    unsigned char n ,dat,bit_t;

    for(n=0;n<8;n++)
    {
        SPI_SCK=0;

        SPI_delay();
        dat<<=1;
        if(SPI_RSDI)
            dat|=0x01;
        else 
            dat&=0xfe;        
        SPI_delay();
        SPI_SCK=1;
    }
    // SPI_SCK=0;
    return dat;
}
#endif


/**********************************************
模式二           写数据
***********************************************/
#if _CPOL==1&&_CPHA==0           //MODE   1  0
void SPI_Send_Dat(unsigned char dat)
{
 unsigned char n;
 for(n=0;n<8;n++)
 {
  SPI_SCK=1;
  if(dat&0x80)SPI_SDI=1;
  else SPI_SDI=0;
  SPI_delay();
  dat<<=1;
  SPI_SCK=0;
  SPI_delay();
 }
  SPI_SCK=1;
}
/*********************************************
模式二          读数据
*********************************************/
//unsigned char SPI_Receiver_Dat(void)
//{
// unsigned char n ,dat,bit_t;
// for(n=0;n<8;n++)
// {
//  SPI_SCK=1;
//  dat<<=1;
//  if(MISO_I())dat|=0x01;
//  else dat&=0xfe;
//  SPI_delay();
//  SPI_SCK=0;
//  SPI_delay();
// }
//  SPI_SCK=1;
//  return dat;
//}

#endif


/*********************************************
模式一        写数据
*********************************************/
#if _CPOL==0&&_CPHA==1           //MODE  0  1
void SPI_Send_Dat(unsigned char dat)
{
 unsigned char n;
 SPI_SCK=0;
 for(n=0;n<8;n++)
 {
  SPI_SCK=1;
  if(dat&0x80)SPI_SDI=1;
  else SPI_SDI=0;
  SPI_delay();
  dat<<=1;
  SPI_SCK=0;
  SPI_delay();
 }
}
/*********************************************
模式一       读数据
*********************************************/
//unsigned char SPI_Receiver_Dat(void)
//{
// unsigned char n ,dat,bit_t;
// for(n=0;n<8;n++)
// {
//  SPI_SCK=1;
//   dat<<=1;
//  if(MISO_I())dat|=0x01;
//  else dat&=0xfe;
//  SPI_delay();
//  SPI_SCK=0;
//  SPI_delay();
// }
//  SPI_SCK=0;
//  return dat;
//}
#endif


/*********************************************
模式三        写数据
*********************************************/
#if _CPOL==1&&_CPHA==1            //MODE  1  1
void SPI_Send_Dat(unsigned char dat)
{
    unsigned char n;

    SPI_SCK=1;
    for(n=0;n<8;n++)
    {
        SPI_SCK=0;
        if(dat&0x80)
            SPI_SDI=1;
        else 
            SPI_SDI=0;
        SPI_delay();
        dat<<=1;
        SPI_SCK=1;
        SPI_delay();
    }
}
/************************************
模式三          读数据
************************************/
//unsigned char SPI_Receiver_Dat(void)
//{
// unsigned char n ,dat,bit_t;
// SPI_SCK=0;
// for(n=0;n<8;n++)
// { 
//    SPI_SCK=0;
//    dat<<=1;
//    if(MISO_I())
//        dat|=0x01;
//    else 
//      dat&=0xfe;
//    SPI_delay();
//    SPI_SCK=1;
//    SPI_delay();
// }
//    SPI_SCK=1;
//    return dat;
//}


#endif


//command 模式设置或打开一个显示方框
void SPI_DispArea(unsigned int xs, unsigned int ys, 
                  unsigned int xe, unsigned int ye)
{        
    unsigned int size1;
//    
//    xs+=0x04;
//    xe+=0x04;
    
   SPI_Write_4data(0X2A,xs>>8,xs,xe>>8,xe);
   SPI_Write_4data(0X2b,ys>>8,ys,ye>>8,ye);
   
    SPI_CSX=0;
    SPI_DCX=0;
    SPI_delay();
    SPI_Send_Dat(0x2c);
    SPI_delay();
    SPI_DCX=1;
}


void SPI_RGB(u32 rgb)
{
    SPI_Send_Dat(rgb>>16);
//    SPI_delay();
    SPI_Send_Dat(rgb>>8);
//    SPI_delay();
    SPI_Send_Dat(rgb>>0);
}
/********************************* SKYCODE ************************************/
