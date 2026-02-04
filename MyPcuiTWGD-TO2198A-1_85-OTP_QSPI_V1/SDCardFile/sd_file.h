/********************************* SKYCODE *************************************
* Copyright (c) 2013-2018, 深圳思凯测试技术有限公司（广州天码电子技术有限公司）
* All rights reserved.
*
* 文件名   ：sd_file.c 
* 描述     ：1.显示SD卡中 .BIN 和 .bmp 格式的图片
*              支持英文长文件名
*            2.在SD卡产生文本
*
* 函数列表 ：
*           void SDCard_Init(void);
*           void SDCard_BIN(char* file_name);
*           void SDCard_BMP(char* file_name);
*           int SDCard_Printf(const char *path,const char *fmt, ...);
*           int SDCard_PrintfHead(const char *path,const char *fmt, ...);
*			int SDCard_Save(const char *path,const void *data,int length);
*           int SDCard_getSave(char *path,void *data,int length);
*           int SDCard_Create(const char *path);
*
*
* 版    本 ：V2.8.0
* 作    者 ：SKYCODE2013
* 完成日期 ：2020-7-11
* 修改描述 ： 创建一个新文件，如果文件已存在，则它将被截断并覆盖
*
* 版    本 ：V2.7.0
* 作    者 ：SKYCODE2013
* 完成日期 ：2020-6-2
* 修改描述 ：1. 上次改支持SPI，mipi bmp改出BUG了，修正
*
* 版    本 ：V2.6.0
* 作    者 ：SKYCODE2013
* 完成日期 ：2020-5-16
* 修改描述 ：1. 增加支持SPI显示接口，增加SDCard_2_Confg()函数用于配置适用于SPI显示接口显示SD卡图片
*               配置为 SPI显示接口后，调用SDCard_BIN() 或 SDCard_BMP()图画直接显示不再加载
*
* 版    本 ：V2.5.0
* 作    者 ：SKYCODE2013
* 完成日期 ：2019-4-17
* 修改描述 ：1. 修正 sd_file.h 中SDCard_getSave 函数名写错
*
* 版    本 ：V2.0.4
* 作    者 ：SKYCODE2013
* 完成日期 ：2019-4-12
* 修改描述 ：1. 增加保存与读取数据
*
* 版    本 ：V2.0.3
* 作    者 ：SKYCODE2013
* 完成日期 ：2018-4-13
* 修改描述 ：1. 增加图片分辨率 和 .bmp格式判断。
*
* 版    本 ：V2.0.2
* 作    者 ：SKYCODE2013
* 完成日期 ：2018-2-9
* 修改描述 ：1. 增加 SDCard_Printf() SDCard_PrintfHead() 两个函数，实现在SD卡产生文本
*
* 版    本 ：V2.0.1
* 作    者 ：SKYCODE2013
* 完成日期 ：2017-11-15
* 修改描述 ：1. 修改显示.bmp漏洞
*            
* 版    本 ：V2.0.0
* 作    者 ：
* 完成日期 ：2016-12-10
* 修改描述 ：1. 提速加载SD卡图片
*            2. 增加显示BMP功能
*
*******************************************************************************/

#ifndef _SD_FILE_H_
#define _SD_FILE_H_

#include "sys.h"


/*
*函数名 : SDCard_Init
*描述   : 初始化SD卡
*参数   : 无
*返回   : 无
*/
extern void SDCard_Init(void);


/*
*函数名 : SDCard_BIN
*描述   : 加载SD卡 二进制(BIN)图像
*参数   : file_name - 图像文件名（可包含文件夹目录名），支持长英文名
*返回   : 无
*/
extern void SDCard_BIN(char* file_name);


/*
*函数名 : SDCard_BMP
*描述   : 加载SD卡 BMP格式图片, 必须是24位真彩色图片
*参数   : file_name - 图像文件名（可包含文件夹目录名），支持长英文名
*返回   : 无
*/
extern void SDCard_BMP(char* file_name);

/*
*函数名 : SDCard_Printf
*描述   : 向文件中写入字符串，若无文件则新建
*参数   : path 文件路径  fmt与printf相同
*返回   : 负数为错误代码  其他为写入文件的数据长度
*/
extern int SDCard_Printf(const char *path,const char *fmt, ...);

/*
*函数名 : SDCard_PrintfHead
*描述   : 新建一个文件并向文件中写入头部信息，若文件已存在则退出
*参数   : path 文件路径  fmt与printf相同
*返回   : 负数为错误代码 -1代表文件已存在  其他为写入文件的数据长度
*/
extern int SDCard_PrintfHead(const char *path,const char *fmt, ...);

/*
*函数名 : SDCard_getSave
*描述   : 向文件中写入二进制数据
*参数   : path 文件路径  fmt与printf相同
*返回   : 负数为错误代码  非负为正确
*/
extern int SDCard_Save(const char *path,const void *data,int length);

/*
*函数名 : SDCard_getSave
*描述   : 从sd卡中读取二进制数据，如果不存在会创建
*参数   : path 文件路径  data数据指针 length为长度
*返回   : 负数为错误代码 非负为正确
*/
extern int SDCard_getSave(const char *path,void *data,int length);

/*
*函数名 : SDCard_2_Confg
*描述   : SPI QSPI等显示图片，使用前要调用本函数初始化一遍，
*参数   : 1. spi_if - =1 选择SPI显示接口等，要为下面的指针赋正确的值,SD卡图片会直接显示出来 而不会缓存。
                      =0 选择MIPI接口，下面指针赋值无效
          2. pf_lcd_wr_byte_rgb - 函数指针,指向往LCD写一个字节画面数据的函数，此函数必须是连续的
          3. pf_lcd_draw_block - 函数指针，指向开显示窗函数
*返回   : 无
*/
void SDCard_2_Confg(unsigned int spi_if,
                    void (*pf_lcd_wr_byte_rgb)(u8 data), 
                    void (*pf_lcd_draw_block)(unsigned int x_start, unsigned int y_start, unsigned int x_stop,unsigned int y_stop));


 /*
*函数名 : SDCard_Create
*描述   : 创建一个新文件，如果文件已存在，则它将被截断并覆盖
*参数   : path 文件路径  
*返回   : -1为错误代码，0为正确
*/
extern int SDCard_Create(const char *path);                                

#endif

/********************************* SKYCODE ************************************/
