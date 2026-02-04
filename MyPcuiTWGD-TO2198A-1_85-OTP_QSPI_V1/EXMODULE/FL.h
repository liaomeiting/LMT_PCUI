/********************************* SKYCODE ************************************
* Copyright (c) 2017, 深圳思凯测试技术有限公司
* All rights reserved.
*
* 文件名   ：FL.h    
* 描述     ：1. FL 镜头驱动程序。
*            2. 可同时驱动8个镜頭，以ID 区分各个镜頭, ID号范围0~7           
*
* 函数列表 ：
*           void FL_Init(u8 id);
*           unsigned int FL_RdFlicCont(u8 id, float *fc);
*           unsigned int FL_RdFlicJEITA(u8 id, float *fjeita);     
*           unsigned int FL_RdJEITA665(u8 id, float *fjeita_6_65hz); 
*           unsigned int FL_AdjLV(u8 id, float adj);   
*           unsigned int FL_0Cal(u8 id);
*           unsigned int FL_0CalStatus(u8 id);
*           unsigned int FL_RdSerNum(u8 id, u32* ser_num);
*
* 版    本 ：V0.4_S20
* 作    者 ：skycode2013
* 完成日期 ：2018-3-15
* 修改描述 ：1. 适用于S20 F427平台 
*
* 版    本 ：V0.4
* 作    者 ：skycode2013
* 完成日期 ：2017-07-3
* 修改描述 ：1. 增加读产品序列
*
* 版    本 ：V0.3
* 作    者 ：skycode2013
* 完成日期 ：2017-06-27
* 修改描述 ：1. 读FLCKER 亮度等 增加调零状态反馈
*            2. 增加调零函数
*            3. 增加调零状态查询
*
* 版    本 ：V0.2
* 作    者 ：skycode2013
* 完成日期 ：2017-06-26
* 修改描述 ：增加读 6HZ~65HZ的JEITA
*
* 版    本 ：V0.1
* 作    者 ：skycode2013
* 完成日期 ：2017-04-07
* 修改描述 ：
*                                                                      
******************************************************************************/

#ifndef _FL_H_
#define _FL_H_

#include "stm32f4xx.h"
#include "systick.h"

#define FV_SUC 0X01 //操作完成
#define FV_FAI 0XFF //操作失败，可能是通讯失败

#define FV_OLD      0x00    //獲取的值是舊值，上次已獲取過
#define FV_NEW      0X01     //獲取的值是新值
#define FV_ERROR    0xFF    //獲取錯誤，可能是通訊錯誤

#define FV_0CAL_NOT 0X02  //未调0
#define FV_0CAL_OK  0X01  //已调0



/*
*函數名：FL_Init
*描述  ：初始化 某ID 的镜头 
*参数  ：1. di - 镜頭 ID 值域 0~7
*返回  ：无
*/
extern void FL_Init(u8 id);


/*
*函數名：FL_RdFlicCont
*描述  ：获取Flicker 对比法测量的 百分比值。     
*参数  ：1. di - 镜頭 ID 值域 0~7
*        2. *fc - 保存flicker 百分比值的变量地址.
*           *fc值的單位是%。
*
*返回  ：状态值。
*        =FV_OLD：获取的值是旧值，上次已获取过
*        =FV_NEW：获取的值是新值 
*        =FV_0CAL_NOT：镜头未调零，无法获得正确测量值
*         =FV_ERROR：获取錯誤，参数有误 或 通讯失败
*
*例子  ：
        float fc;
        if(FV_NEW == FL_RdFlicCont(0, &fc)) //以返回值判断flicker值是否是新值
        {                             
            ...... //用户程序
        }
*/
extern unsigned int FL_RdFlicCont(u8 id, float *fc);


/*
*函數名：FL_RdFlicJEITA
*描述  ：获取Flicker JEITA dB值。    
*
*参数  ：1. di - 镜頭 ID 值域 0~7
*         2. *fjeita - 保存flicker 單位為dB的值的变量地址。
*            *fjeita 是个负数,單位是dB。
*    
*返回  ：状态值。
*        =FV_OLD：获取的值是旧值，上次已获取过
*         =FV_NEW：获取的值是新值 
*        =FV_0CAL_NOT：镜头未调零，无法获得正确测量值
*         =FV_ERROR：获取錯誤，参数有误 或 通讯失败
*
*例子  ：
        float fjeita;   
        if(FV_NEW == FL_RdFlicJEITA(0, &fjeita)) //以返回值判断flicker值是否是新值
        {                                                           
            ...... //用户程序
        }
*/
extern unsigned int FL_RdFlicJEITA(u8 id, float *fjeita);

/*
*函数名：FL_RdJEITA665
*描述  ：获取亮度值
*
*参数  ：1. di - 镜頭 ID 值域 0~7
*        2. *fjeita_6_65hz 保存6HZ~65HZ JEITA值的数组
*    
*返回  ：状态值。
*         =FV_SUC 成功 
*        =FV_0CAL_NOT 镜头未调零，无法获得正确测量值
*         =FV_FAI 失败
*
*/
extern unsigned int FL_RdJEITA665(u8 id, float *fjeita_6_65hz);

/*
*函數名：FL_RdLV
*描述  ：获取亮度值
*
*参数  ：1. di - 镜頭 ID 值域 0~7
*         2. *lv - 保存 亮度值的变量地址。
*            *lv ,單位cd/m2。
*    
*返回  ：状态值。
*        =FV_OLD：获取的值是旧值，上次已获取过
*         =FV_NEW：获取的值是新值 
*        =FV_0CAL_NOT：镜头未调零，无法获得正确测量值
*         =FV_ERROR：获取錯誤，参数有误 或 通讯失败
*
*例子  ：
        float lv;   
        if(FV_NEW == FL_RdLV(0, &lv)) //以返回值判断是否是新值
        {                                                           
            ...... //用户程序
        }
*/
extern unsigned int FL_RdLV(u8 id, float *lv);

/*
*函數名：FL_AdjLV
*描述  ：调整亮度值
*
*参数  ：1. di - 镜頭 ID 值域 0~7
*         2. adj - 调整参数， adj = 标准亮度/FL1亮度, adj不能大于3
*            
*返回  ：状态值。
*        = FV_SUC 操作完成
*        = FV_FAI 操作失败，参数有误 或 通讯失败
*/
extern unsigned int FL_AdjLV(u8 id, float adj);

/*
*函数名：FL_0Cal
*描述  ：调零
*
*参数  ：1. di - 镜頭 ID 值域 0~7
*    
*返回  ：状态值。
*         =FV_SUC：仅表示是通信成功，调零耗时长久，需在返回FV_SUC後13秒，用
                   FL_0CalStatus() 函数查询调0状态。
*         =FV_FAI：失败，ID错误或通信失败。
*
*/
extern unsigned int FL_0Cal(u8 id);

/*
*函数名：FL_0CalStatus()
*描述  ：获取调零状态
*
*参数  ：1. di - 镜頭 ID 值域 0~7
*    
*返回  ：状态值。
*         =FV_0CAL_NOT：未调零
*        =FV_0CAL_OK： 已成功调零
*         =FV_FAI：失败，ID错误、通信失败。
*
*/
extern unsigned int FL_0CalStatus(u8 id);


/*
*函数名：FL_RdSerNum
*描述  ：读取FL1 产品序列号，序列号是一个4Byte的数字
*
*参数  ：1. di - 镜頭 ID 值域 0~7
*         2. *ser_num - 指向保存序列号的 u32变量
*    
*返回  ：状态值。
*         =FV_SUC：成功
*         =FV_FAI：失败，ID错误或通信失败。
*/
extern unsigned int FL_RdSerNum(u8 id, u32* ser_num);

#endif

/********************************* SKYCODE ***********************************/
