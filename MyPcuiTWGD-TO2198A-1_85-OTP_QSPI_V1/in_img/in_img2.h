
#ifndef _IN_IMG2_H_
#define _IN_IMG2_H_

#include "sys.h"

/*上下文结构体*/
typedef struct
{
    unsigned int x_size;
    unsigned int y_size;
    void (*wr_rgb)(unsigned int rgb);  //写像素函数
    void (*draw_block)(unsigned int x_start, unsigned int y_start, unsigned int x_stop,unsigned int y_stop); //写坐标函数
}InImg2_Adapter;

/*
*函数名: InImg2_Init
*描述  : 软件产生画面程序的初始化。
*参数  : 1. img2_adapter - 上下文结构体（句柄），
             之后调用画面函数，都要用到这个变量。
         2. x_size - 屏分辨率
         3. y_size - 屏分辨率
         4. (*pf_wr_rgb)(unsigned int rgb) - 写像素的函数，参数格式要和这个一样
         5. (*pf_draw_block)(unsigned int x_start, unsigned int y_start, unsigned int x_stop,unsigned int y_stop)) - 
             跳坐标 或 声明显示范围的函数，点MCU接口和COMMAND MIPI屏经常用到的2A 2B写坐标的最后2C的那个

*返回  : 无
*/
void InImg2_Init(InImg2_Adapter *img2_adapter, 
                 unsigned int x_size, unsigned int y_size, //显示屏分辨率
                 void (*pf_wr_rgb)(unsigned int rgb), 
                 void (*pf_draw_block)(unsigned int x_start, unsigned int y_start, unsigned int x_stop,unsigned int y_stop));

void InImg2_CT(InImg2_Adapter *img2_adapter);
void InImg2_Full(InImg2_Adapter *img2_adapter,unsigned char r, unsigned char g, unsigned char b);
void InImg2_Chcker58(InImg2_Adapter *img2_adapter);
void InImg2_Box(InImg2_Adapter *img2_adapter);
void InImg2_Gray256_V(InImg2_Adapter *img2_adapter);
void InImg2_Gray256_H(InImg2_Adapter *img2_adapter);
void InImg2_RED256_H(InImg2_Adapter *img2_adapter);
void InImg2_GREEN256_H(InImg2_Adapter *img2_adapter);
void InImg2_BLUE256_H(InImg2_Adapter *img2_adapter);
void InImg2_RED256_V(InImg2_Adapter *img2_adapter);
void InImg2_GREEN256_V(InImg2_Adapter *img2_adapter);
void InImg2_BLUE256_V(InImg2_Adapter *img2_adapter);
void InImg2_ColorBar(InImg2_Adapter *img2_adapter); //彩条
void InImg2_ColorBarV(InImg2_Adapter *img2_adapter);
void InImg2_Flicker(InImg2_Adapter *img2_adapter);


#endif



