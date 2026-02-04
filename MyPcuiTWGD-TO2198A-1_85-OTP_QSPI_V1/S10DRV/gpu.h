/********************************* SKYCODE *************************************
* Copyright (c) 2018-2019, 深圳思凯测试技术有限公司
*
* 文件名   ：gpu.c   
* 描述     ：
             1.h10_8lane专用
*            2.底层图像处理
*            3.RGB信号产生
*            4.像素写入，图像加载缓存,图像输出
*            5.LCD接口定义
*
* 版    本 ：V1.1.0
* 作    者 ：SKYCODE2013=mxt
* 完成日期 ：2021-11-19
* 修改描述 ：1. 增加设置在COMMAND模式下写2828的速度。具体看函数GPU_SetCommandWrSpeed()。需要配合FPGA V1.1.0
*
* 版    本 ：V1.0.0
* 作    者 ：SKYCODE2013
* 完成日期 ：2019-8-26
* 修改描述 ：1. 继承AG7D的程序，支持8LANE 2颗2828
**            
*******************************************************************************/

#ifndef _GPU_H_
#define _GPU_H_

#include "sys.h"


/*-------------MIPI 接口方式宏定義--------------*/ 

#define MIPI_VIDEO_1LANE 0X15   //1LANE的MIPI VIDEO接口
#define MIPI_VIDEO_2LANE 0X16   //2LANE的MIPI VIDEO接口
#define MIPI_VIDEO_3LANE 0X17   //3LANE的MIPI VIDEO接口
#define MIPI_VIDEO_4LANE 0X02   //4LANE的MIPI VIDEO接口
#define MIPI_VIDEO_8LANE 0X03  //8LANE MIPI A、BPORT对应左右屏 C、DPORT对应左右屏

#define MIPI_COMMAND_1LANE 0x10 //1LANE的MIPI COMMAND接口
#define MIPI_COMMAND_2LANE 0x11
#define MIPI_COMMAND_3LANE 0x12
#define MIPI_COMMAND_4LANE 0x13
#define MIPI_COMMAND_8LANE 0x14


/*------------------GRB时钟边沿和极性选择宏定义----------------*/
typedef enum
{
    FALLING_EDGE = 0x00, //下降沿锁存
    RISING_EDGE = 0x01,	 //上升沿锁存
}RGBEdge_TypeDef;

typedef enum
{
    ACTIVE_LOW = 0X00,  //低电平有效
    ACTIVE_HIGH = 0X01, //高电平有效
}RGBPolarity_TypeDef;


/*-----------------------接口模式、RGB时序参数全局变量---------------------------------*/
//注意：在调用初始化函數 GPU_Config() 之前必须先给这些变量赋值

extern u8 gLCD_IFMODE;         //各种接口模式 

extern float        gLCD_PCLK;  //单位MHz 精度0.1MHz，例LCD_PCLK = 145.3
extern unsigned int gLCD_XSIZE;
extern unsigned int gLCD_YSIZE;
extern unsigned int gLCD_HBPD;
extern unsigned int gLCD_HFPD;
extern unsigned int gLCD_HSPW;
extern unsigned int gLCD_VBPD;
extern unsigned int gLCD_VFPD;
extern unsigned int gLCD_VSPW;
extern RGBEdge_TypeDef     gLCD_PCLK_EDGE;	  //lcd锁存数据时的PCLK边沿	 
extern RGBPolarity_TypeDef gLCD_HS_POLARITY;  //HSYNC脉冲极性			  
extern RGBPolarity_TypeDef gLCD_VS_POLARITY;  //VSYNC脉冲极性			  
extern RGBPolarity_TypeDef gLCD_DE_POLARITY;  //DE信号有效电平		


/*--------------------各个提供给外部程序的函数-------------------------------*/

/*
*函数名 : GPU_Init
*描述   : 初始化，并检查硬件
*参数   : 无
*返回   : 1. 0x00 初始化成功
          2. 小于0 初始化失败，返回值为错误码，其绝对值含义：
               0x01 GPU PLL NG
               0x02 GPU MCU NG
               0x1n GPU SDRAM NG
                 0x11 SDRAM 1 NG
                 0x12 SDRAM 2 NG
                 0x14 SDRAM 3 NG
*/
extern int GPU_Init(void);	

/*
*函数名 : GPU_Config(void)
*描述   : 配置GPU，调用此函数前必须先给 接口模式，RGB时序参数 等等全局变量 赋值 
*参数   : 无
*返回   : 无
*调用   : 外部调用
*/
extern void GPU_Config(void);

/*
*函数名 : GPU_WriteData
*描述   : 向GPU写数据的最底层操作         
*参数   : data - 要写的数据
*返回   : 无
*/
extern void GPU_WrData(u32 data);


/*
*函数名 : GPU_WrRGB
*描述   : 向GPU写一个像素颜色        
*参数   : data -写像素数据时 color[23:16]为红数据，color[15:8]为绿数据，color[7:0]蓝数据
*               如纯红色 color=0xff0000
*返回   : 无
*/
extern void GPU_WrRGB(u32 color);
 
 
/*
*函数名 : GPU_LoadFrame
*描述   : 开始加载画面，函数参数决定加载为第几个画面
*参数   : frame - 画面号，从0开始
*返回   : 无
*/
extern void GPU_LoadFrame(unsigned int frame);


/*
*函数名 : GPU_DisplayFrame
*描述   : 显示某幅画面
*参数   : frame - 要显示的画面号，从0开始
*返回   : 无
*/
extern void GPU_DisplayFrame(unsigned int frame);


/*
*函数名 : GPU_GotoXy
*描述   : 跳转lcd面板显示坐标
*参数   : x - 水平方向坐標，从0开始
          y - 垂直方向坐標，从0开始
*返回   : 无
*/
extern void GPU_GotoXy(unsigned int x, unsigned int y);

/*
*函数名 : GPU_GotoLoadFrameXy
*描述   : 跳转加载画面缓存区像素坐标，由GPU_LoadFrame()函数设定的画面
*参数   : x - 水平方向坐標，从0开始
          y - 垂直方向坐標，从0开始
*返回   : 无
*/
extern void GPU_GotoLoadFrameXy(unsigned int x, unsigned int y);


/*
*函数名 : GPU_RGB_ON
*描述   : 开启RGB时序
*参数   : 无
*返回   : 无
*/
extern void GPU_RGB_ON(void);


/*
*函数名 : GPU_RGB_OFF
*描述   : 关闭RGB时序
*参数   : 无
*返回   : 无
*/
extern void GPU_RGB_OFF(void);

/*
*函数名 : GPU_GetVersion
*描述   : 获取GPU 固件版本
*参数   : ver[3] : 三个字节的固件版本号
*返回   : 无
*/
extern void GPU_GetVersion(u8 firm_ver[3]);


/*
*函数名 : PD_CommandMode(void)
*描述   : 判断是否运行在COMMAND模式
*参数   : 无
*返回   : =1 是； =0 否
*/
extern int PD_CommandMode(void);

/*
*函数名 : PD_VideoMode(void)
*描述   : 判断是否运行在Video模式
*参数   : 无
*返回   : =1 是； =0 否
*/
extern int PD_VideoMode(void);


/**/
extern void GPU_SSD28_CS_A(u8 cs);
extern void GPU_SSD28_CS_B(u8 cs);
extern void GPU_SSD28_CS_C(u8 cs);
extern void GPU_SSD28_CS_D(u8 cs);
/*COMMAND MODE 专用*/
extern void GPU_RefDisp(void);
extern void GPU_SSD28_WrCmd(u8 cmd);
extern void GPU_SSD28_WrData(u32 data);
extern u32 GPU_SSD28_RdData(void);

/*
*函数名 : GPU_SetCommandWrSpeed()
*描述   : 设置Command模式2828的写速度。
*         注意：1.此函数要在调用GPU_Config()函数之后使用，GPU_Config()函数有复位功能，会把设置值复位了。
*               2.不调用此函数时，默认speed=3.
*               3. 需要配合FPGA V1.1.0
*参数   : speed - 速度参数。建议不小于3。不可以等于0
                  最终速度 = 145MHz/(5 + speed)
*返回   : 无
*/
void GPU_SetCommandWrSpeed(uint8_t speed);


#endif 

/********************************* SKYCODE ************************************/
