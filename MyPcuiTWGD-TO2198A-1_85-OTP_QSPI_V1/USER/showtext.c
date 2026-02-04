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


#include "showtext.h" 
#include "ascii.h"
#include "gpu.h"
#include "systick.h"

#if SPI_IF==1
static void (*sg_pf_lcd_wr_rgb)(unsigned int rgb);
static void (*sg_pf_lcd_draw_block)(unsigned int x_start, unsigned int y_start, unsigned int x_stop,unsigned int y_stop);
#endif

/*************************************************************
显示一个字符
*************************************************************/
static void ShowChar(unsigned int x, unsigned int y, unsigned int font, uint32_t color, uint32_t Bcolor, char a)
{
    unsigned int i,j;
	unsigned int wc, hc, xp;
	const unsigned char *p;

	p=ascii_char[(unsigned char)(a)];

#if SPI_IF==1
    (*sg_pf_lcd_draw_block)(x,y, x+8*font-1, y+16*font-1);
    
    for(i=0; i<16; i++)
    {       
        for(hc=0; hc<font; hc++)
        {
            a = *p;
            for(j=0; j<8; j++)
            {
                for(wc=0; wc<font; wc++)
                {
                    if(a & 0x80)
                    {
                        (*sg_pf_lcd_wr_rgb)(color);
                    }
                    else
                    {
                        (*sg_pf_lcd_wr_rgb)(Bcolor);
                    }
                }
                a<<=1;
            }
            ++y;
        }

        ++p;
    }

#else
    
    for(i = 0; i < 16; i++)
	{       
		for(hc = 0; hc < font; hc++)
		{   
            GPU_GotoXy(x, y);
			xp = x;
			a = *p;
		    for(j = 0; j < 8; j++)
			{
				for(wc = 0; wc < font; wc++)
				{
					++xp;
					if(a & 0x80)
					{
						if(color <= 0xffffff){GPU_WrRGB(color);}
						else {GPU_GotoXy(xp, y);}
					}
					else
					{
						if(Bcolor <= 0xffffff){GPU_WrRGB(Bcolor);}
						else {GPU_GotoXy(xp, y);}
					}
				}
				a<<=1;
			}
			++y;
		}

		++p;
	}
    
#endif

}

/*
*函数名 : ShowString
*描述   : 显示一串字符，（注意 不会自动换行）
*参数   : x - 起始坐标水平方向值
		  y - 起始坐标垂直方向值
		  font - 字体尺寸参数 详见宏定义
		  color - 字符颜色, 如：color=0xFF0000为纯红色, color=0x00ff00为纯绿色，无色赋值为color=NOCOLOR
		  Bcolor - 字符背景颜色，如：color=0xFF0000为纯红色, color=0x00ff00为纯绿色，无色赋值为color=NOCOLOR
		  *p - 指向字符串的指针
*返回   : 无
*/
void ShowString(unsigned int x, unsigned int y, unsigned int font, uint32_t color, uint32_t Bcolor, char* p)
{
	char a;
//	Delay_ms(5);
	while((a=*p++)!=0){ShowChar(x, y, font, color, Bcolor, a); x += 8*font;}

#if SPI_IF==0
    GPU_RefDisp();
#endif
}


/*************************************************************
加载一个字符
*************************************************************/
static void LoadChar(unsigned int x, unsigned int y, unsigned int font, uint32_t color, uint32_t Bcolor, char a)
{
    unsigned int i,j;
	unsigned int wc, hc, xp;
	const unsigned char *p;

#if SPI_IF==1  //SPI 显示接口模式不支持这个函数
    return ;
#endif
    
	p=ascii_char[(unsigned char)(a)];

	for(i = 0; i < 16; i++)
	{       
		for(hc = 0; hc < font; hc++)
		{
			GPU_GotoLoadFrameXy(x, y);
			xp = x;
			a = *p;
		    for(j = 0; j < 8; j++)
			{
				for(wc = 0; wc < font; wc++)
				{
					++xp;
					if(a & 0x80)
					{
						if(color <= 0xffffff){GPU_WrRGB(color);}
						else {GPU_GotoLoadFrameXy(xp, y);}
					}
					else
					{
						if(Bcolor <= 0xffffff){GPU_WrRGB(Bcolor);}
						else {GPU_GotoLoadFrameXy(xp, y);}
					}
				}
				a<<=1;
			}
			++y;
		}

		++p;
	}
}

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
void LoadString(unsigned int x, unsigned int y, unsigned int font, uint32_t color, uint32_t Bcolor, char* p)
{
	char a;

	while((a=*p++)!=0){LoadChar(x, y, font, color, Bcolor, a); x += 8*font;}

}

#if SPI_IF==1
void ShowString_2_Config(void (*pf_lcd_wr_rgb)(unsigned int rgb), 
                         void (*pf_lcd_draw_block)(unsigned int x_start, unsigned int y_start, unsigned int x_stop,unsigned int y_stop))

{
    sg_pf_lcd_wr_rgb = pf_lcd_wr_rgb;
    sg_pf_lcd_draw_block = pf_lcd_draw_block;
}
#endif

/******************************* 广州天码电子 *********************************/
