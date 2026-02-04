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
* 版    本 ：V1.0.0
* 作    者 ：SKYCODE2013
* 完成日期 ：2019-5-30
* 修改描述 ：1. 由 S10 V2.3.0版本修改而来
*            2. 支持8lane
*            3. Dcs Short 0XBF封装方式
*
*            						                          	
*******************************************************************************/

#include "ssd2828.h"
#include "debug_def.h"
#include <stdarg.h>

//////////////////////////////// DeBug define /////////////////////////////////
/*程序中使用 _DEBUG_SSD28() 来调试代码*/

#define __DEBUG_SSD28__ 1   //注释掉这句 则代码不执行_DEBUG_SSD28()

#ifdef __DEBUG_SSD28__
#include "debug_def.h"
#define _DEBUG_SSD28 _DEBUG 
#else
#define _DEBUG_SSD28(format,...)   
#endif

///////////////////////////////////////////////////////////////////////////////


/*---------------------------模擬SPI接口引腳宏定義----------------------------*/
#define SPI_SDI_1()     GPIO_SetBits(GPIOE, GPIO_Pin_1)
#define SPI_SDI_0()     GPIO_ResetBits(GPIOE, GPIO_Pin_1)

#define SPI_SCLK_1()    GPIO_SetBits(GPIOE, GPIO_Pin_2)
#define SPI_SCLK_0()    GPIO_ResetBits(GPIOE, GPIO_Pin_2)

#define SPI_SDO()		GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_0)  //输入


#define SSD_RESET_1()   GPIO_SetBits(GPIOC, GPIO_Pin_0)
#define SSD_RESET_0()   GPIO_ResetBits(GPIOC, GPIO_Pin_0)

#define SSD_SHUT_1()    GPIO_SetBits(GPIOE, GPIO_Pin_4)
#define SSD_SHUT_0()    GPIO_ResetBits(GPIOE, GPIO_Pin_4) 

/*----------------------------LP or HS or Video------------------------------*/
#define LP 0
#define HS 1
#define VD 2
unsigned int sg_speedmode;  //

/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/*0xB6寄存器设置的一些宏*/
#define VS_P_LOW (0<<7)
#define VS_P_HIGH (1<<7)

#define HS_P_LOW (0<<6)
#define HS_P_HIGH (1<<6)

#define PCLK_P_FALLING (0<<5) //下降沿
#define PCLK_P_RISING (1<<5)

#define CBM_0 (0<<0)
#define CBM_1 (1<<0) //no blanking packet

#define NVB_0 (0<<7)
#define NVB_1 (1<<7) //burst模式下 非视频信号在垂直消隐期间传送

#define NVD_HS (0<<6) //非视频数据将使用HS模式传输
#define NVD_LP (1<<6) //非视频数据将使用LP模式传输


/*---------------------------------------------------------------------------*/
static unsigned char sg_cs;

static unsigned int sg_par_size=0;  //MIPI写数据长度 
static unsigned int sg_par_cnt=0;
static unsigned int sg_dcs_swr=0;

u16 gHS_Mbps; //MIPI 高速模式的数据率（COAMMAND模式的数据率设置）


extern u16 SSD2828_baH_MsNs(u16 hs_mbps, u8 *ms, u8 *ns);

/*----------------------------------------------------------------------------*/

void SSD2828_GPIOConfig(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE|RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC|
                           RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOF|RCC_AHB1Periph_GPIOD, ENABLE );
        

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;//SCLK
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    
    //,SHUT,SDI
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_1; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    
    //SDO
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;   //SDO
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    //RST
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
        
    //PS4
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    GPIO_SetBits(GPIOE, GPIO_Pin_3);
    
    //IF
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

//    if(PD_CommandMode())
//    {
//        GPIO_SetBits(GPIOE, GPIO_Pin_6);
//    }
//    else
//    {
//        GPIO_ResetBits(GPIOE, GPIO_Pin_6);
//    }
        
    //2828 POWER
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;	
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;   
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; 
    GPIO_Init(GPIOA, &GPIO_InitStructure);	
    SSD2828_POWER_OFF();
}


/*----------------------------SSD28 SPI 底层时序-----------------------------*/

static void SPI_WriteCmd(u8 cmd)
{   
    unsigned char i;
    
    SPI_SDI_0(); Delay_us(1);                     
    SPI_SCLK_1();Delay_us(1);  
    SPI_SCLK_0();
    
    for(i=0;i<8;i++)
    {
        Delay_us(1);
        if((cmd&0x80)==0x80){SPI_SDI_1();}
        else  {SPI_SDI_0();}

        Delay_us(1);
        SPI_SCLK_1();
        Delay_us(1);
        SPI_SCLK_0();
        
        cmd=cmd<<1;
    }
  
    Delay_us(1); 
}

static void SPI_WriteData(u8 dat)
{   
    unsigned char i;

    SPI_SDI_1();
    Delay_us(1);
     
    SPI_SCLK_1();
    Delay_us(1);
    
    SPI_SCLK_0();
    
    for(i=0;i<8;i++)
    {
        Delay_us(1);
        if((dat&0x80)==0x80)
        {   
            SPI_SDI_1();
        }
        else
        {
            SPI_SDI_0();
        }

        Delay_us(1);
        SPI_SCLK_1();
        Delay_us(1); 
        SPI_SCLK_0();
        
        dat=dat<<1;
    }
   
    Delay_us(1);   
}

//ssd2829讀操作
static u8 SPI_ReadData(void)
{
    unsigned int i;
    uint8_t tmp = 0;
 
    for(i = 0; i < 8; i++)
    {
        Delay_us(2);
        tmp <<= 1;
        if(SPI_SDO()) tmp |= 0x01;
        
        SPI_SCLK_1();
        Delay_us(1);
        SPI_SCLK_0();
    }

    Delay_us(1);
    
    return tmp;
}


/*---------------------------------------------------------------------------*/

void SSD2828_CS_0(void)
{
	u8 tem = sg_cs;
	if (tem & 0x01)
		GPU_SSD28_CS_A(1);
	else
		GPU_SSD28_CS_A(0);
	tem = tem >> 1;
	
	if (tem & 0x01)
		GPU_SSD28_CS_B(1);
	else
		GPU_SSD28_CS_B(0);

	
}

void SSD2828_CS_1(void)
{
	GPU_SSD28_CS_A(0);
	GPU_SSD28_CS_B(0);
	
//    switch(CS_2828N)
//	{
//	case 0:
//		GPU_SSD28_CS_A(0);
//		break;
//	case 1:
//		GPU_SSD28_CS_B(0);
//		break;
//	case 2:
//		GPU_SSD28_CS_C(0);
//		break;
//	case 3:
//		GPU_SSD28_CS_D(0);
//		break;
//	}
}

/*用于写初始化数据, 写一个字节*/
void SSD2828_WriteData(u8 data)
{
    static u32 par=0; 
    
    SSD2828_CS_0();

    if(sg_dcs_swr==1)
    {
        SSD2828_WriteCmd(data);
        sg_dcs_swr = 0;
        return;
    }
    
    
    if(PD_CommandMode())
    {
    
        if(sg_par_size == 1)
        {
            par = data;
            sg_par_cnt = 0;
            sg_par_size = 0;
            GPU_SSD28_WrData(par&0X0000FF);
            par = 0;
        }
        else if(sg_par_size == 2)
        {
            par |= ((u16)(data) << 8);
            if(++sg_par_cnt >= 2)
            {
                sg_par_cnt = 0;
                sg_par_size -= 2;
                GPU_SSD28_WrData(par&0X00FFFF);
                par = 0;
            }
            par >>= 8;        
        }
        else if(sg_par_size >= 3)
        {
            par |= ((u32)(data) << 16);
            if(++sg_par_cnt >= 3)
            {
                sg_par_cnt = 0;
                sg_par_size -= 3;
                GPU_SSD28_WrData(par&0XFFFFFF);
                par = 0;
            }
            par >>= 8;
        }
    }
    else
    {
        SPI_WriteData(data);
    }   
    
    SSD2828_CS_1();
}

void SSD2828_WriteCmd(u8 cmd)
{
    if(PD_CommandMode())
    {
        GPU_SSD28_WrCmd(cmd);
    }
    else
    {
        SPI_WriteCmd(cmd);
    }
}


//写2828寄存器 带两个8bit参数
void SSD2828_WriteReg(u8 cmd, u8 dat_h, u8 dat_l)
{       
    SSD2828_CS_0();
    
    if(PD_CommandMode())
    {
        GPU_SSD28_WrCmd(cmd);
        GPU_SSD28_WrData(((u16)(dat_h)<<8) | (dat_l));
    }
    else
    {
        SPI_WriteCmd(cmd);
        SPI_WriteData(dat_l);
        SPI_WriteData(dat_h);
    }
    
    SSD2828_CS_1();
    
}

//讀2828寄存器 16bit的
u16 SSD2828_ReadReg(u8 reg)
{
    u16 rd_data;
    
    if(PD_CommandMode())
    {  
        SSD2828_CS_0();
        GPU_SSD28_WrCmd(reg);
        Delay_us(10);
        rd_data = GPU_SSD28_RdData()&0xffff;
    }
    else
    {
        SSD2828_WriteReg(0xD4,0x00, 0xFA); //Set read command "FA" for 3 wire 8bit
        SSD2828_CS_0();
        
        SPI_WriteCmd(reg);    //被读的寄存器地址
        SPI_WriteCmd(0xFA);    //讀命令
        rd_data = SPI_ReadData();
        rd_data |= ((u32)(SPI_ReadData()) << 8);
    }
    
    SSD2828_CS_1();
    
    return rd_data;
    
}


//----------------------------------------------------------------------------------

void SSD2828_Init(void)
{
	SSD2828_GPIOConfig();
    SPI_SCLK_0();
	SSD_SHUT_0();
    SSD2828_POWER_ON();
    SSD_RESET_0();
}

void SSD2828_Config(void)
{
	unsigned int video_pll_clk;
    u8 pclk_p;
	u8 r_ba_FR,r_ba_MS=1,r_ba_NS;
	u8 r_bb_LPD;
    u8 lane_num;
	
    if(PD_CommandMode())
    {
        GPIO_SetBits(GPIOE, GPIO_Pin_6);
    }
    else
    {
        GPIO_ResetBits(GPIOE, GPIO_Pin_6);
    }
    
    SSD_RESET_0();
	Delay_ms(10);
	SSD_RESET_1();
	Delay_ms(50);
    
    /*检测2828*/

    SSD2828_A_CS_0();
    SSD2828_B_CS_1();

    if(SSD2828_ReadReg(0xB0) != 0x2828)
    {
      _DEBUG_SSD28("SSD2828_A NG\n\r"); 
    }
    
    SSD2828_A_CS_1();
    SSD2828_B_CS_0();
    if(SSD2828_ReadReg(0xB0) != 0x2828)
    {

      _DEBUG_SSD28("SSD28282_B NG\n\r"); 
    }
    
    SSD2828_A_CS_0();
    SSD2828_B_CS_0();
        
    if(PD_CommandMode())
    {
        _DEBUG_SSD28("MIPI Command Mode\n\r");
    }
    else
    {
        _DEBUG_SSD28("MIPI Video Mode\n\r");
    }

    /*PLL 时钟计算*/    
    if(PD_CommandMode())
    {
        video_pll_clk = SSD2828_baH_MsNs(gHS_Mbps, &r_ba_MS, &r_ba_NS);
        if(gLCD_IFMODE == MIPI_COMMAND_1LANE)
        {
            lane_num = 0;
        }
        else if(gLCD_IFMODE == MIPI_COMMAND_2LANE)
        {
            lane_num = 1;
        }
        else if(gLCD_IFMODE == MIPI_COMMAND_2LANE)
        {
            lane_num = 2;
        }
        else //if((gLCD_IFMODE == MIPI_COMMAND_4LANE) /*|| (gLCD_IFMODE == MIPI_COMMAND_8LANE)*/)
        {
            lane_num = 3;
        }
    }
    else
    {
        if(gLCD_IFMODE == MIPI_VIDEO_1LANE)
        {
            video_pll_clk = gLCD_PCLK * 24;
            lane_num = 0;
        }
        else if(gLCD_IFMODE == MIPI_VIDEO_2LANE)
        {
            video_pll_clk = gLCD_PCLK * 12;
            lane_num = 1;
        }
        else if(gLCD_IFMODE == MIPI_VIDEO_3LANE)
        {
            video_pll_clk = gLCD_PCLK * 8;
            lane_num = 2;
        }
        else //if((gLCD_IFMODE == MIPI_VIDEO_4LANE) /*|| (gLCD_IFMODE == MIPI_VIDEO_8LANE)*/)
        {
            video_pll_clk = gLCD_PCLK * 6;
            lane_num = 3;
        }
        
        //NS
        if(video_pll_clk%24)
        {
            r_ba_NS = video_pll_clk/24+1;
        }
        else
        {
            r_ba_NS = video_pll_clk/24+1;
        }
    }
        
    _DEBUG_SSD28("MIPI %dMbps/lane\n\r", 24*r_ba_NS/r_ba_MS);

	//pll fOUT 范围
	if(video_pll_clk <= 125)
	{
		r_ba_FR = 0x00;
	}
	else if(video_pll_clk <= 250)
	{
		r_ba_FR = 0x40;
	}
	else if(video_pll_clk <= 500)
	{
		r_ba_FR = 0x80;
	}
	else
	{
		r_ba_FR = 0xc0;
	}    
    

	SSD2828_WriteReg(0x00b9,0x00,0x00); //关pLL

	SSD2828_WriteReg(0x00b1, gLCD_VSPW, gLCD_HSPW);
	
	if((SSD2828_VIDEO_MODE==0x01) || (SSD2828_VIDEO_MODE==0x02))
	{
		SSD2828_WriteReg(0x00b2, gLCD_VBPD+gLCD_VSPW, gLCD_HBPD+gLCD_HSPW);
	}
	else
	{
		SSD2828_WriteReg(0x00b2, gLCD_VBPD, gLCD_HBPD);
	}
	
	SSD2828_WriteReg(0x00b3,gLCD_VFPD, gLCD_HFPD);
	
 
    if((gLCD_IFMODE!=MIPI_VIDEO_8LANE) && (gLCD_IFMODE!=MIPI_COMMAND_8LANE))
	{ 
		SSD2828_WriteReg(0xb4,(gLCD_XSIZE >> 8) & 0xff, gLCD_XSIZE & 0xff);
	}
	else
	{
		SSD2828_WriteReg(0xb4,((gLCD_XSIZE/2) >> 8) & 0xff, (gLCD_XSIZE/2) & 0xff);
	}


	SSD2828_WriteReg(0xb5,(gLCD_YSIZE >> 8) & 0xff, gLCD_YSIZE & 0xff);

    if(gLCD_PCLK_EDGE == FALLING_EDGE){pclk_p=PCLK_P_FALLING;}
    else {pclk_p=PCLK_P_RISING;}

//	SSD2828_WriteReg(0x00b6,0x00,0xCB);	
	SSD2828_WriteReg(0x00b6,pclk_p, (0x03)|(SSD2828_VIDEO_MODE<<2)|NVB_0); 
//    SSD2828_WriteReg(0x00b6,pclk_p, (0x03)|(SSD2828_VIDEO_MODE<<2)|NVB_1); //
	
	SSD2828_WriteReg(0x00ba, r_ba_FR|r_ba_MS, r_ba_NS); // PLL= fIN X NS = xMHz	

	//LP 模式的时钟
	r_bb_LPD = (unsigned int)24*r_ba_NS/(r_ba_MS * (8*10));
//    r_bb_LPD = (unsigned int)(r_ba_NS)*24 / 90 /2;
	if(r_bb_LPD > 0X3F)
	{
		r_bb_LPD = 0X3F;
	}
	
	SSD2828_WriteReg(0x00bb, 0x00, r_bb_LPD);  // LP Mode CLK =PLL /(LPD+1)/8 = x MHz
	
	SSD2828_WriteReg(0x00b8,0x00,0x00);
	
	SSD2828_WriteReg(0x00c9,0x23,0x02);
	SSD2828_WriteReg(0x00ca,0x23,0x01);
	SSD2828_WriteReg(0x00cb,0x05,0x10);
	SSD2828_WriteReg(0x00cc,0x10,0x05);
	
   //SSD2828_WriteReg(0x00d0,0x00,0x00);
	
	SSD2828_WriteReg(0x00de,0x00,lane_num);
//	SSD2828_WriteReg(0x00d6,0x00,0x04); ////设置RGB顺序?? 在视频模式下消隐期间发送的数据包的数量
	SSD2828_WriteReg(0x00d6,0x00,0x05);//设置RGB顺序?? 在视频模式下消隐期间发送的数据包的数量
	SSD2828_WriteReg(0x00c4,0x00,0x01); //Automatically perform BTA after the next write packe
	//SSD2828_WriteReg(0x00e9,0xff2f); //
	SSD2828_WriteReg(0x00eb,0x80,0x00);

//    _DEBUG_SSD28("REG = 0x%X\r\n",SSD2828_ReadReg(0xCC));

	Delay_ms(10);

	SSD2828_WriteReg(0x00b9,0x00,0x01); //开PLL
	Delay_ms(120);
}


/*
*函数名 : SSD2828_DcsLongWrite
*描述   : DT = 0x05, 0x15的写操作设置
*参数   : n 写的字节数
*返回   : 无
*/
void SSD2828_DcsShortWrite(u8 len)
{
    
    sg_par_size = len;
    sg_par_cnt = 0;
   
    if(sg_speedmode == LP)
	{	
		SSD2828_WriteReg(0x00b7,0x02,0x50);	  //EOT Packet Enable,ECC CRC Check Enable, DCS, Short packer, LP
	}
	else if(sg_speedmode == HS)
	{
		SSD2828_WriteReg(0x00b7, 0x02, (0x50&0XEF)|0X03); //EOT Packet Enable,ECC CRC Check Enable, DCS, Short packer, HS
	} 
	else if(sg_speedmode == VD)
	{
		SSD2828_WriteReg(0x00b7, 0x02|0x01, (0x50&0XEF)|0X0B); //EOT Packet Enable,ECC CRC Check Disable, DCS, Short packer, HS Video
	}
	Delay_us(100);
	SSD2828_WriteReg(0xbc,0x00,len);
	SSD2828_WriteReg(0xbd,0x00,0x00);
	SSD2828_WriteReg(0xbe,0x00,len);
    
    SSD2828_CS_0();
    
//    sg_dcs_swr = 1;
    SSD2828_WriteCmd(0XBF);
        
//    SSD2828_CS_1();
    
}


/*
*函数名 : SSD2828_DcsLongWrite
*描述   : DT = 0X39的写操作设置
*参数   : n 写的字节数
*返回   : 无
*/
void SSD2828_DcsLongWrite(int len)
{
    
    sg_par_size = len;
    sg_par_cnt = 0;
    
    if(sg_speedmode == LP)
	{
		SSD2828_WriteReg(0x00b7,0x06,0x50);  //EOT Packet Enable,ECC CRC Check Enable, DCS Long Write, LP
	}
	else if(sg_speedmode == HS)
	{
		SSD2828_WriteReg(0x00b7,0x06, (0x50&0XEF)|0X03);  //EOT Packet Enable,ECC CRC Check Enable, DCS Long Write, HS
	}
	else if(sg_speedmode == VD)
	{
		SSD2828_WriteReg(0x00b7, 0x06|0x01, (0x50&0XEF)|0X0B);  //EOT Packet Enable,ECC CRC Check Disable, DCS Long Write, HS Video
	}

	Delay_us(100);
	SSD2828_WriteReg(0xbc,(len)>>8, (len));
	SSD2828_WriteReg(0xbd,(len)>>24, (len)>>16);
	SSD2828_WriteReg(0xbe,0x0f,0xff);
    
    SSD2828_CS_0();
    
    SSD2828_WriteCmd(0xbf);
        
//    SSD2828_CS_1();
}


/*
*函数名 : SSD2828_GenericShortWrite
*描述   : DT = 0x03, 0x13, 0x23的写操作设置
*参数   : n 写的字节数
*返回   : 无
*/
void SSD2828_GenericShortWrite(int len)
{        
    sg_par_size = len;
    sg_par_cnt = 0;

	if(sg_speedmode == LP)
	{
		SSD2828_WriteReg(0x00b7,0x02,0x10); //EOT Packet Enable,ECC CRC Check Enable, Generic Short Write, LP
	}
	else if(sg_speedmode == HS)
	{
		SSD2828_WriteReg(0x00b7,0x02,(0x10&0XEF)|0X03);  //EOT Packet Enable,ECC CRC Check Enable, Generic Short Write, HS
	}
	else if(sg_speedmode == VD)
	{
		SSD2828_WriteReg(0x00b7,0x02|0x01,(0x10&0XEF)|0X0B);  //EOT Packet Enable,ECC CRC Check Disable, Generic Short Write, HS Video
	}

	Delay_us(100);
	SSD2828_WriteReg(0xbc,0x00,len);
	SSD2828_WriteReg(0xbd,0x00,0x00);
	SSD2828_WriteReg(0xbe,0x00,len);
    
    SSD2828_CS_0();
    
    SSD2828_WriteCmd(0xbf);
    
//    SSD2828_CS_1();
    
}


/*
*函数名 : SSD2828_GenericLongWrite
*描述   : DT = 0x29的写操作设置
*参数   : n - 写的字节数
*返回   : 无
*/
void SSD2828_GenericLongWrite(int len)
{
    
    sg_par_size = len;
    sg_par_cnt = 0;
    
    if(sg_speedmode == LP)
	{	
		SSD2828_WriteReg(0x00b7,0x06,0x10); //EOT Packet Enable,ECC CRC Check Enable, Generic Long Write, LP; 
	}
	else if(sg_speedmode == HS)
	{
		SSD2828_WriteReg(0x00b7,0x06,(0x10&0XEF)|0X03); //EOT Packet Enable,ECC CRC Check Enable, Generic Long Write, HS;
	}
	else if(sg_speedmode == VD)
	{
		SSD2828_WriteReg(0x00b7,0x06|0X01,(0x10&0XEF)|0X0B);//EOT Packet Enable,ECC CRC Check Disable, Generic Long Write, HS Video
	}
	
	Delay_us(100);
	SSD2828_WriteReg(0xbc,(len)>>8, (len));
	SSD2828_WriteReg(0xbd,(len)>>24,(len)>>16);
	SSD2828_WriteReg(0xbe,0x0f,0xff);
    
    SSD2828_CS_0();
    SSD2828_WriteCmd(0xbf);
        
//    SSD2828_CS_1();
}


/*
*函数名 : SSD2828_MIPIPackWrite
*描述   : ！！此函数不能直接使用 只能调用宏定义的 MIPI_WR()
*返回   : 无
*/
u8 pack_data[260];
void SSD2828_MIPIPackWrite(u8 DT, int data, ...)
{
    va_list param_list;
    int in_data;
    unsigned int len=0,i;
    
    va_start(param_list, data);
    
    pack_data[len++] = data;
    
    while(1)
    {
        in_data = va_arg(param_list, int);
        if(in_data == -1)break;
        pack_data[len++] = in_data;
    }
    
//    _DEBUG_SSD28("DT=0x%x, len=%d", DT, len);
//    for(i=0; i<len; i++)
//    {
//        _DEBUG_SSD28(", %x",pack_data[i]);
//    }
//    _DEBUG_SSD28("\n\r");
    
    if(len>256)return; 
    
    switch(DT)
    {
        case 0x39:
            SSD2828_DcsLongWrite(len);
            break;
        
        case 0x29:
            SSD2828_GenericLongWrite(len);
            break;
        
        case 0x05:
        case 0x15:
            SSD2828_DcsShortWrite(len);
            break;
        
        case 0x03:
        case 0x13:
        case 0x23:
            SSD2828_GenericShortWrite(len);
            break;
    }
    
    for(i=0; i<len; i++)
    {
        SSD2828_WriteData(pack_data[i]);
    }
    
    va_end(param_list);
}

/*
*函数名 : MIPI_WrArray
*描述   : MIPI 写 一个数组的数据
*         整合了 DT=0X39 0X29 0X05 0X15 0X03 0X13 0X23 的MIPI写操作 
*参数   : 1. DT - data type
*         2. len - 数组的长度
          3. data_array - 要MIPI传送的数组
*返回   : 无
*/
void MIPI_WrArray(u8 DT, int len, u8 *data_array)
{
    int i;
    switch(DT)
    {
        case 0x39:
            SSD2828_DcsLongWrite(len);
            break;
        
        case 0x29:
            SSD2828_GenericLongWrite(len);
            break;
        
        case 0x05:
        case 0x15:
            SSD2828_DcsShortWrite(len);
            break;
        
        case 0x03:
        case 0x13:
        case 0x23:
            SSD2828_GenericShortWrite(len);
            break;
    }
    
    for(i=0; i<len; i++)
    {
        SSD2828_WriteData(data_array[i]);
    }
}

/*******************************************************************************
*函数名 : SSD2828_DcsReadDT06
*描述   : DCS READ, no parameters DT=0X06
*参数   : reg - LCM 驱动IC寄存器地址
          len - 讀的长度 单位Byte
		  p   - 保存读出的数据的数组
*返回   : 读状态 成功或失敗
          MIPI_READ_FAIL 读失敗
		  MIPI_READ_SUCCEED 读成功
*******************************************************************************/
unsigned int SSD2828_DcsReadDT06(u8 reg, u16 len, u8 *p)
{
	uint16_t state;
	unsigned int i;
	unsigned int timeout_cnt=0;
    u32 tmp;
    
	do
	{
		if(sg_speedmode == LP)
		{
			SSD2828_WriteReg(0x00b7,0x03,0xc2);		//LP DCS read
		}
		else if(sg_speedmode == HS)
		{
			SSD2828_WriteReg(0x00b7,0x03,(0xc2&0XEF)|0X03);
		}
		else if(sg_speedmode == VD)
		{
			SSD2828_WriteReg(0x00b7,0x03,(0xc2&0XEF)|0X0B);
		}

//		SSD2828_WriteReg(0x00bb,0x00,8);			//PL clock
		SSD2828_WriteReg(0x00c1,len>>8,len);		//Maximum Return Size
		SSD2828_WriteReg(0x00c0,0x00,0x01);		//取消SSD2828_的操作？？
		SSD2828_WriteReg(0x00BC,0x00,0x01);
		SSD2828_WriteReg(0x00BF,0x00,reg);	   //把要讀的地址發送出去
		Delay_ms(10);					   //讀需要一点点时间
		state = SSD2828_ReadReg(0xc6);		//读 ssd2828 的状态寄存器
		
		if(state & 0x01){break;}     //讀成功 跳出循環
		else if(++timeout_cnt > 10){return MIPI_READ_FAIL;}		//超時 讀失敗
	}while(1);	

	SSD2828_CS_0();
    SSD2828_WriteCmd(0xff);
    
    if(PD_CommandMode()) 
    {       
        for(i = 0; i < len;)
        {
            tmp = GPU_SSD28_RdData();
            *p++ = tmp;
            if(++i >= len){break;}
            *p++ = (tmp>>8);
            if(++i >= len){break;}
            *p++ = (tmp>>16);
            ++i;
        }
    }
    else
    {
        for(i = 0; i < len;)
        {
            SPI_WriteCmd(0xfa); //讀命令
            *p++ = SPI_ReadData();
            if(++i >= len){SPI_ReadData(); break;}
            *p++ = SPI_ReadData();
            ++i;
        }
    }
        
    SSD2828_CS_1();

	return MIPI_READ_SUCCEED;
}
 

/*******************************************************************************
*函数名 : SSD2828_GenericReadDT14
*描述   : Generic READ, 1 parameter; DT=0X14
*参数   : reg - LCM 驱动IC寄存器地址
          len - 讀的长度 单位Byte
		  p   - 保存读出的数据的数组
*返回   : 读状态 成功或失敗
          MIPI_READ_FAIL 读失敗
		  MIPI_READ_SUCCEED 读成功
*******************************************************************************/
unsigned int SSD2828_GenericReadDT14(u8 reg, u16 len, u8 *p)
{
	uint16_t state;
	unsigned int i;
	unsigned int timeout_cnt=0;
    u32 tmp;
    
	do
	{
		if(sg_speedmode == LP)
		{
			SSD2828_WriteReg(0x00b7,0x03,0x82);		//LP generic read
		}
		else if(sg_speedmode == HS)
		{
			SSD2828_WriteReg(0x00b7,0x03,(0x82&0XEF)|0X03);
		}
		else if(sg_speedmode == VD)
		{
			SSD2828_WriteReg(0x00b7,0x03,(0x82&0XEF)|0X0B);
		}

//		SSD2828_WriteReg(0x00bb,0x00,8);			//PL clock
		SSD2828_WriteReg(0x00c1,len>>8,len);		//Maximum Return Size
		SSD2828_WriteReg(0x00c0,0x00,0x01);		//取消SSD2828_的操作？？
		SSD2828_WriteReg(0x00BC,0x00,1);		 // *参数个数 
		SSD2828_WriteReg(0x00BF,0x00,reg);	   //把要讀的地址發送出去
		Delay_ms(10);					   //讀需要一点点时间
		state = SSD2828_ReadReg(0xc6);		//读 ssd2828 的状态寄存器

		if(state & 0x01){break;}     //讀成功 跳出循環
		else if(++timeout_cnt > 10){return MIPI_READ_FAIL;}		//超時 讀失敗
	}while(1);	//ssd2828的状态不对 就全部重来

	SSD2828_CS_0();
    SSD2828_WriteCmd(0xff);
    
    if(PD_CommandMode()) 
    {       
        for(i = 0; i < len;)
        {
            tmp = GPU_SSD28_RdData();
            *p++ = tmp;
            if(++i >= len){break;}
            *p++ = (tmp>>8);
            if(++i >= len){break;}
            *p++ = (tmp>>16);
            ++i;
        }
    }
    else
    {
        for(i = 0; i < len;)
        {
            SPI_WriteCmd(0xfa); //讀命令
            *p++ = SPI_ReadData();
            if(++i >= len){SPI_ReadData(); break;}
            *p++ = SPI_ReadData();
            ++i;
        }
    }
    
    SSD2828_CS_1();

	return MIPI_READ_SUCCEED;
}

/*LP 模式*/
void SSD2828_LP()
{
	sg_speedmode = LP;
}

/*VIDEO 模式*/
void SSD2828_Video()
{
	sg_speedmode = VD;
	SSD2828_WriteReg(0x00b7,0x03,0x0B);//使能 video，24b,Short  TCLK做参考时钟	
//	SSD2828_WriteReg(0x00b7,0x07,0x0B);//使能 video，24b,long  TCLK做参考时钟	
}

/*HS 模式 command*/
void SSD2828_HS(void)
{
	sg_speedmode = HS;
}


//command 模式设置或打开一个显示方框
void LCD_DispArea(unsigned int xs, unsigned int ys, 
                  unsigned int xe, unsigned int ye)
{        
    unsigned int size1;
    unsigned int pst;
    
    size1 = (long)(xe-xs+1)*(ye-ys+1)*3;
    
    xs=xs+0x16;//根据实际情况设置
    xe=xe+0x16; 
    
    SSD2828_CS_0();
 
    SSD2828_DcsLongWrite(5);
    SSD2828_WriteData(0x2a);
    SSD2828_WriteData(xs>>8);//high 8bit
    SSD2828_WriteData(xs);//Horizontal_start
    SSD2828_WriteData(xe>>8);//Horizontal_end
    SSD2828_WriteData(xe);//low 8 bit*/	

    SSD2828_DcsLongWrite(5);
    SSD2828_WriteData(0x2b);
    SSD2828_WriteData(ys>>8);//VertiMTP_start
    SSD2828_WriteData(ys);
    SSD2828_WriteData(ye>>8);
    SSD2828_WriteData(ye);
    
    //-------------------------
    SSD2828_WriteReg(0xbc,size1>>8,size1);
    SSD2828_WriteReg(0xbd,size1>>24,size1>>16);

    pst = (xe-xs+1)*3;
    if(pst%12)
    {
        pst += (12- (pst%12));
    }
    SSD2828_WriteReg(0xbe, pst>>8,pst);//SSD2828_WriteReg(0xbe,((xe-xs+1)*3 )>> 8,(xe-xs+1)*3 );
    
    //-------------------------
    SSD2828_CS_0();
	SSD2828_WriteCmd(0x2c);

}


void SSD2828_POWER_ON(void) 
{
    GPIO_SetBits(GPIOA, GPIO_Pin_8);
    Delay_ms(100);
}

void SSD2828_POWER_OFF(void) 
{
    GPIO_ResetBits(GPIOA, GPIO_Pin_8);
}

void SSD2828_RST_H(void)
{
	SSD_RESET_1();
}

void SSD2828_RST_L(void)
{
	SSD_RESET_0();
}

//片選2828 IC1
void SSD2828_A_CS_1(void)
{
    sg_cs &= (u8)(~0x01);     
}	  

void SSD2828_A_CS_0(void)
{
    sg_cs |= 0x01; 
}

//片選2828 IC1
void SSD2828_B_CS_1(void)
{
    sg_cs &= (u8)(~0x02);     
}	  

void SSD2828_B_CS_0(void)
{
    sg_cs |= 0x02; 
}

/********************************* SKYCODE *************************************/
