/********************************* SKYCODE ************************************
* Copyright (c) 2018, 深圳思凯测试技术有限公司
* All rights reserved.
*
* 文件名   ：cfl.c    
* 描述     ：s10 镜头驱动程序
*
* 当前版本 ：
* 作者     ：
* 完成日期 ：2025-5-28
* 修改描述 ：完善读60个DB的函数
*
* 当前版本 ：V1.3.0
* 作者     ：cws
* 完成日期 ：2019-10-21
* 修改描述 ：
*
*							                          	
******************************************************************************/
#ifndef __CFL_H
#define	__CFL_H
#include "stm32f4xx.h"
#include <stdio.h>


#define CFL_ID0 0x01
#define CFL_ID1 0x02
#define CFL_ID2 0x04
#define CFL_ID3 0x08
#define CFL_ID4 0x10
#define CFL_ID5 0x20
#define CFL_ID6 0x40
#define CFL_ID7 0x80

#define CFL_ID8 	0x0100
#define CFL_ID9 	0x0200
#define CFL_ID10 	0x0400
#define CFL_ID11 	0x0800
#define CFL_ID12 	0x1000
#define CFL_ID13 	0x2000
#define CFL_ID14 	0x4000
#define CFL_ID15 	0x8000

#define Replace_CA310 	1
#define Replace_CA410 	0


/*************1.0.2********************/

typedef struct
{
	float T;
	float duv;
	float Lv;
}TdeltauvLvTypeDef;

typedef struct
{
	float u_;
	float v_;
	float Lv;
}u_v_LvTypeDef;

/*************CFL********************/
typedef struct
{
	float X;
	float Y;
	float Z;
	
}XYZTypeDef;

typedef struct
{
	float X;
	float Y;
	float Lv;
	
}XYLvTypeDef;

typedef struct
{
	u8 b1;
	u8 b2;
	u8 b3;
	
}VersionTypeDef;

typedef struct
{  
    
	float Replace_CA410_HZ;
    
}PatchTypeDef_1_0_8;

typedef struct
{  
    
	u8 zero_flag ;
    
}PatchTypeDef_1_0_9;

typedef struct
{  
    
	u16 Cycle_num;
    
}PatchTypeDef_1_0_10;

/*
*函数名: CFL_Init
*描述  : 初始电源板驱动程序
*参数  : 无
*返回  : =0 初始化成功，<0 失败      
*/
int CFL_Init(void);

/*
*函数名: CFL_GetCalXYLvMeasure
*描述  : 回读颜色校准值
*参数  : 
	1. PN:探头id，见最上方宏定义。第0位表示ID0  第七位表示ID7 
    2. color : 0 红 ，1 绿， 2 蓝
	3. xyLv  ： 读取的值 具体请看结构体定义
       注意:如果ID大于1，或拖多个探头，这个参数必须是数组(的地址)，如xyLv[0]是CFL IDO的值，xyLv[1]是CFL ID1的值...  xyLv[15]是CFL ID15的值
*返回  : =0 成功，<0 失败 
*/
int CFL_GetCalXYLv(u16 PN,u8 color,XYLvTypeDef *xyLv);

/*
*函数名: CFL_SetCalXYLv
*描述  : 设置颜色校准值
*参数  : 
	1. PN:探头id，见最上方宏定义。第0位表示ID0  第七位表示ID7 
    2. color : 0 红 ，1 绿， 2 蓝
	3. xyLv  ： 要设置的值 具体请看结构体定义
       注意:如果ID大于1，或拖多个探头，这个参数必须是数组(的地址)，如xyLv[0]是CFL IDO的值，xyLv[1]是CFL ID1的值...  xyLv[15]是CFL ID15的值
*返回  : =0 成功，<0 失败 
*/
int CFL_SetCalXYLv(u16 PN,u8 color,XYLvTypeDef *xyLv);



/*
*函数名: CFL_GetCalXYLvMeasure
*描述  : 回读测量颜色XYLv
*参数  : 
	1. PN:探头id，见最上方宏定义。 第0位表示ID0  第七位表示ID7 
	1. xyLv  ： 读取的值 具体请看结构体定义
       注意:如果ID大于1，或拖多个探头，这个参数必须是数组(的地址)，如xyLv[0]是CFL IDO的值，xyLv[1]是CFL ID1的值...  xyLv[15]是CFL ID15的值
*返回  : =0 成功，<0 失败 
*/
int CFL_GetMeasureXYLv(u16 PN,XYLvTypeDef *xyLv);

/*
*函数名: CFL_GetCalXYLvMeasure
*描述  : 回读测量的db值
*参数  : 
	1. PN:探头id，见最上方宏定义。 第0位表示ID0  第七位表示ID7 
    1. db : 返回的测量值
       注意:如果ID大于1，或拖多个探头，这个参数必须是数组(的地址)，如 db[0]是CFL IDO的值，db[1]是CFL ID1的值...  db[15]是CFL ID15的值
*返回  : =0 成功，<0 失败 
*/
int CFL_GetMeasureDB(u16 PN,float *db);

/*
*函数名: CFL_GetMeasureDBGroup
*描述  : 回读测量的db组  6HZ~65hZ
*参数  : 
	1. PN:探头id，见最上方宏定义。 第0位表示ID0  第七位表示ID7 
    2. dbGroup : 大小为60的数组 ，返回的测量值
       注意:如果ID大于1，或拖多个探头，这个参数必须是二维数组(的地址)，如 dbGroup[0][0~59]是CFL_ID0的值,dbGroup[1][0~59]是CFL_ID1的值,... dbGroup[15][0~59]是CFL_ID15的值   
        例子 :   CFL_GetMeasureDBGroup(CFL_ID0,&dbGroup[0][0]);    //无论传入哪个ID，都是二维数组的首地址
                 CFL_GetMeasureDBGroup(CFL_ID1,&dbGroup[0][0]);
*返回  : =0 成功，<0 失败 
*/
int CFL_GetMeasureDBGroup(u16 PN,float *dbGroup);


/*
*函数名: CFL_GetVersionNum
*描述  : 回读软件版本号
*参数  : 
    1. PN:探头id，见最上方宏定义。 第0位表示ID0  第七位表示ID7 
    2. Version : 返回的版本号 
				注意:如果ID大于1，或拖多个探头，这个参数必须是数组(的地址)，如 Version[0]是CFL IDO的值，Version[1]是CFL ID1的值...  Version[15]是CFL ID15的值
*返回  : =0 成功，<0 失败 
*/
int CFL_GetVersionNum(u16 PN,VersionTypeDef *Version);

/*
*函数名: CFL_GetCalXYLvMeasure
*描述  : 回读测量的flick
*参数  : 
	1. PN:探头id，见最上方宏定义。 第0位表示ID0  第七位表示ID7 
    2. flick : 返回的测量值
             注意:如果ID大于1，或拖多个探头，这个参数必须是数组(的地址)，如 flick[0]是CFL IDO的值，flick[1]是CFL ID1的值...  flick[15]是CFL ID15的值
    
*返回  : =0 成功，<0 失败 
*/
int CFL_GetMeasureFLICK(u16 PN,float *flick);


/*
*函数名: CFL_GetMeasureTdeltauvLv
*描述  : 回读测量TdeltauvLv
*参数  : 
    1. PN:探头id，见最上方宏定义。 第0位表示ID0  第七位表示ID7 
    2. TdeltauvLv : 返回的测量值
       注意:如果ID大于1，或拖多个探头，这个参数必须是数组(的地址)，如 TdeltauvLv[0]是CFL IDO的值，TdeltauvLv[1]是CFL ID1的值...  TdeltauvLv[15]是CFL ID15的值
*返回  : =0 成功，<0 失败 
*/
int CFL_GetMeasureTdeltauvLv(u16 PN,TdeltauvLvTypeDef *TdeltauvLv);


/*
*函数名: CFL_GetMeasureu_v_Lv
*描述  : 回读测量u_v_Lv
*参数  : 
    1. PN:探头id，见最上方宏定义。 第0位表示ID0  第七位表示ID7 
    2. u_v_Lv : 返回的测量值
       注意:如果ID大于1，或拖多个探头，这个参数必须是数组(的地址)，如 u_v_Lv[0]是CFL IDO的值，u_v_Lv[1]是CFL ID1的值...  u_v_Lv[15]是CFL ID15的值
*返回  : =0 成功，<0 失败 
*/
int CFL_GetMeasureu_v_Lv(u16 PN,u_v_LvTypeDef *u_v_Lv);



/*
*函数名: CFL_SetLED_SW
*描述  : 设置显示屏开关
*参数  : 
    1. PN:探头id，见最上方宏定义。 第0位表示ID0  第七位表示ID7 
    2. *ledFlag 1打开0关闭
        注意:如果ID大于1，或拖多个探头，这个参数必须是数组(的地址)，如 ledFlag[0]是CFL_IDO的值，ledFlag[1]是CFL_ID1的值...  ledFlag[15]是CFL_ID15的值
*返回  : =0 成功，<0 失败 
*/
int CFL_SetLED_SW(u16 PN,u8 *ledFlag);

/*
*函数名: CFL_SetShutter_SW
*描述  : 设置快门开关
*参数  : 
    1. PN:探头id，见最上方宏定义。 第0位表示ID0  第七位表示ID7 
    2. *ShutterFlag 1打开0关闭
    注意:如果ID大于1，或拖多个探头，这个参数必须是数组(的地址)，如 ShutterFlag[0]是CFL_IDO的值，ShutterFlag[1]是CFL_ID1的值...  ShutterFlag[15]是CFL_ID15的值
*返回  : =0 成功，<0 失败 
*/
int CFL_SetShutter_SW(u16 PN,u8 *ShutterFlag);


int readMatrix_Cal(u16 PN, float *cal);//回读校准矩阵
int readMatrix_Inv_Cal(u16 PN, float *cal);

/*
*函数名: CFL_GetMeasureXYZ
*描述  : 回读测量颜色XYZ
*参数  : 
    1. PN:探头id，见最上方宏定义。 第0位表示ID0  第七位表示ID7 
	2. XYZ  ： 读取的值 具体请看结构体定义
       注意:如果ID大于1，或拖多个探头，这个参数必须是数组(的地址)，如 XYZ[0]是CFL_IDO的值，XYZ[1]是CFL_ID1的值...  XYZ[15]是CFL_ID15的值
*返回  : =0 成功，<0 失败 
*/
int CFL_GetMeasureXYZ(u16 PN,XYZTypeDef *XYZ);


/*
*函数名: CFL_Set_CH
*描述  : 切换通道
*参数  : 
	1. PN:探头id，见最上方宏定义，
    2. ch范围0-99 不在范围切到0通道
       注意:如果ID大于1，或拖多个探头，这个参数必须是数组(的地址)
*返回  : =0 成功，<0 失败 
*/
int CFL_Set_CH(u16 PN,u8 *CH);

/*
*函数名: CFL_Read_Sole_ID
*描述  : 读唯一ID
*参数  : 
    1. PN:探头id，见最上方宏定义，
    2. *ID 
       注意:如果ID大于1，或拖多个探头，这个参数必须是数组(的地址)
*返回  : =0 成功，<0 失败 
*/
int CFL_Read_Sole_ID(u16 PN,u32 *ID);

/*
*函数名: CFL_read_CAL_Time
*描述  : 读校准的日期
*参数  : 
    1. PN:探头id，见最上方宏定义，
    2. *CAL_Time 
       注意:如果ID大于1，或拖多个探头，这个参数必须是数组(的地址)
*返回  : =0 成功，<0 失败 
*/
int CFL_read_CAL_Time(u16 PN,u32 *CAL_Time);



/*
*函数名: CFL_SET_CAL_Time
*描述  : 设置频率
*参数  : 
    1. PN:探头id，见最上方宏定义，
    2. *SET_HZ 
       注意:如果ID大于1，或拖多个探头，这个参数必须是数组(的地址)
*返回  : =0 成功，<0 失败 
*/
int CFL_SET_SYNC_HZ(u16 PN,u8 CA310_CA410,float *SET_HZ);


/*
*函数名: CFL_Zero_cal
*描述  : 校准零偏（清零操作）//等待时间较长，大概35秒。
*参数  : 
    1. PN:探头id，见最上方宏定义，
    2. *data 
       注意:如果ID大于1，或拖多个探头，这个参数必须是数组(的地址)
*返回  : =0 成功，<0 失败 
*/
int CFL_Zero_cal(u16 PN,u8 *data);


/*
*函数名: CFL_Set_Cycle_num
*描述  : 设置采样的周期数量，采样的时间与同步频率有关。举例：60hz频率，采样周期是5，采样时间=1000/60*5=83.333ms。
*参数  : 
    1. PN:探头id，见最上方宏定义，
    2. *data 
       注意:如果ID大于1，或拖多个探头，这个参数必须是数组(的地址) 
*返回  : =0 成功，<0 失败 
*/
int CFL_Set_Cycle_num(u16 PN,u16 *data)	;
	
	
#endif 


