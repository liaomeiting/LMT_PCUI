/********************************* SKYCODE ************************************
* Copyright (c) 2014-2019, 深圳思凯测试技术有限公司
* All rights reserved.
*
* 文件名   ：key.h
* 描述     : 按键扫描 和按键LED指示灯 驱动程序
*
* 版    本 ：V1.0.0
* 作者     ：SKYCODE2013
* 完成日期 ：2019-08-26
* 修改描述 ：1. 继承 S10 key.c V1.3.0
*            2. 适用于H10_8LANE，有两个key的IO与S10不一样
*							                          	
******************************************************************************/


#ifndef _KEY_H_
#define _KEY_H_

#include "stm32f4xx.h"


/*按键值宏定义*/
#define KEY_ONOFF   0x01   //显示开关
#define KEY_DOWN    0x02   //下翻
#define KEY_UP      0x04   //上翻
#define KEY_ENTER   0x08   //确认
#define KEY_OTP     0x08   //确认键做OTP
#define KEY_NULL 	0x00


/*KEY LED设置宏定义*/
#define KEYLED_OFF  0x00
#define KEYLED_ON   0x01


////////////////////////////////////////////////////////////////////////


/*
*函数名: KEY_Init
*描述  : 初始化按键驱动程序
*参数  : 无
*返回  : 无      
*/
extern void KEY_Init(void);


/*
*函数名: KEY_Read
*描述  : 按鍵掃描，每隔20ms調用一次
*参数  : 无
*返回  : 回读到的键值。键值请用上面宏定义     
*/
extern u8 KEY_Read(void);


/*
*函数名: KEY_SetLED
*描述  : 设置键盘上的指示灯
*参数  : key - 对应按键上的指示灯，请用上面按键值宏定义来赋值
         led_onoff - 设置指示灯状态，请用上面宏定义
                     =KEYLED_ON 点亮指示灯；=KEYLED_OFF关灭指示灯
*返回  : 无    
*/
extern void KEY_SetLED(u8 key, u8 led_onoff);

#endif

/********************************* SKYCODE ***********************************/
