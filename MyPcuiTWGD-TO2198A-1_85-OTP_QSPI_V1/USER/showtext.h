/********************************* SKYCODE *************************************
* Copyright (c) 2014-2020, 深圳思凯测试技术有限公司（广州天码电子技术有限公司）
* All rights reserved.
*
* 文件名   ：showtext.c 
* 描述     ：实现在模组显示字符串
*
* 版    本 ：V0.3
* 作    者 ：skycode2013
* 完成日期 ：2023-04-17 
* 修改描述 ：1. 修复 SPI_IF 为0时字符出错，漏了跳坐标，此问题在2021年AG7D老化机有发现并修复
*
* 版    本 ：V0.2
* 作    者 ：skycode2013
* 完成日期 ：2020-05-16 
* 修改描述 ：1. 本来是MIPI接口专用的显示字符的程序，增加支持SPI显示字符的功能
                支持SPI显示字符是需要下面 SPI_IF宏定义为 1 且要调用ShowString_2_Config()函数进行配置。
*
* 版    本 ：V0.1
* 作    者 ：skycode2013
* 完成日期 ：2014-05-26 
* 修改描述 ：
*            							                          	
*******************************************************************************/

#ifndef _SHOWTEXT_H_
#define _SHOWTEXT_H_

#include "sys.h"

//SPI等非MIPI无经过GPU的显示接口的选择
#define SPI_IF 1  //=1 选择SPI等非MIPI无经过GPU的显示接口 =0选择MIPI等有经过GPU的显示接口


//字体尺寸参数宏定义
#define FONT0816  0x01   //size: w=8, h=16
#define FONT1632  0X02   //size: w=16, h=32
#define FONT2448  0X03   //size: w=24, h=48
#define FONT3264  0X04   //size: w=32, h=64 
#define FONT4080  0X05   //size: w=40, h=80
#define FONT4896  0X06   //size: w=48, h=96

//色彩宏定义
#define NOCOLOR 0X1000000  //无色
#define RED    0XFF0000
#define GREEN  0X00FF00
#define BLUE   0X0000FF
#define WHITE  0XFFFFFF
#define BLACK  0X000000

/*
*函数名 : ShowString
*描述   : 显示一串字符，（注意 不会自动换行）
*参数   : x - 起始坐标水平方向值
		  y - 起始坐标垂直方向值
		  font - 字体尺寸参数 详见上面的字体尺寸参数宏定义
		  color - 字符颜色, 如：color=0xFF0000为纯红色, color=0x00ff00为纯绿色，无色赋值为color=NOCOLOR
		  Bcolor - 字符背景颜色，如：color=0xFF0000为纯红色, color=0x00ff00为纯绿色，无色赋值为color=NOCOLOR
		  *p - 指向字符串的指针
*返回   : 无
*/
void ShowString(unsigned int x, unsigned int y, unsigned int font, uint32_t color, uint32_t Bcolor, char* p);


/*
*函数名 : LoadString
*描述   : 加载一串字符，是在图片缓存区加载一串字符串,不是马上显示的（注意 不会自动换行）
*参数   : x - 起始坐标水平方向值
		  y - 起始坐标垂直方向值
		  font - 字体尺寸参数 详见宏定义
		  color - 字符颜色, 如：color=0xFF0000为纯红色, color=0x00ff00为纯绿色，无色赋值为color=NOCOLOR
		  Bcolor - 字符背景颜色，如：color=0xFF0000为纯红色, color=0x00ff00为纯绿色，无色赋值为color=NOCOLOR
		  *p - 指向字符串的指针
*返回   : 无
*/
void LoadString(unsigned int x, unsigned int y, unsigned int font, uint32_t color, uint32_t Bcolor, char* p);


void ShowString_2_Config(void (*pf_lcd_wr_rgb)(unsigned int rgb), 
                         void (*pf_lcd_draw_block)(unsigned int x_start, unsigned int y_start, unsigned int x_stop,unsigned int y_stop));
                             
#endif

/******************************* 广州天码电子 *********************************/
