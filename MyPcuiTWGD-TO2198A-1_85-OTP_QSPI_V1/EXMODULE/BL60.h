/********************************* SKYCODE ************************************
* Copyright (c) 2019, 深圳思凯测试技术有限公司
* All rights reserved.
*
* 文件名   ：bl60.h    
* 描述     ：背光板驱动程序
*
* 函数列表 ：
*           void BUSBL60_Init(void);
            int BUSBL60_SetV(int group, int Ce, float Vol);
            int BUSBL60_SetI(int group, int Ce, int Ch, float Current);
            int BUSBL60_LED_ON(int group,int Ce,u32 chs);
            int BUSBL60_LED_OFF(int group,int Ce,int Off);
            int BUSBL60_StartAlarm(int group,int Ce);
            int BUSBL60_StopAlarm(int group,int Ce);
            int BUSBL60_VMinMax(int group,int Ce,int Ch,float Max,float Min);
            int BUSBL60_IMinMax(int group,int Ce,int Ch,float Max,float Min);
            int BUSBL60Set_PWM(int group,int Ce,int OnOff);
            int BUSBL60Get_BlIV(int group, int Ce, ReLEDPowerTypeDef *IVLED);
            int BUSBL60_ReLedOnOff(int group,int Ce,BlOnOffDef *psLEDONOFF);
            int BUSBL60_GetAlarmState(int group, int Ce, ErrorFlagDef *Error);
            
* 版本     ：V1.0.1
* 作者     ：
* 完成日期 ：2022-4-1
* 修改描述 ：1.添加 int BUSBL60Set_SYNCEN1_ON(int group); 
             2.添加 int BUSBL60Set_SYNCEN2_ON(int group);
             3.添加 int BUSBL60Set_SYNCEN1_OFF(int group);
             4.添加 int BUSBL60Set_SYNCEN2_OFF(int group);
*
* 版本     ：V1.0.0
* 作者     ：
* 完成日期 ：2020-1-19
* 修改描述 ：1.增加函数
*           
*			 				            							        
******************************************************************************/
#ifndef __BL60_H
#define	__BL60_H

#include "stm32f4xx.h"
#include "sys.h" 
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "Public_SoftCan.h"


#define BL60_ID0     1  
#define BL60_ID1     2
#define BL60_ID2     4
#define BL60_ID3     8
#define BL60_ID4     16
#define BL60_ID5     32
#define BL60_ID6     64
#define BL60_ID7     128 //ID号指的是背光机的拨码数


#define BL60_CN1     1 
#define BL60_CN2     2 
#define BL60_CN4     4 //对应的是背光机的电源板ID
/*************BL20********************/

typedef struct
{
	u8 flag;    //等待数据采集为1  采集完成为0

}BL20TypeDef;

typedef struct
{
	u8 userId;    //找出背光机的ID

}BL60Id;

typedef struct
{
	
    float LEDV1[20];    
    float LEDI1[20];     
    u8 errorFlag[20];  //低两位电压 0正常 1偏大 2偏小 ，第2、3位电流
    
}ReLEDPowerTypeDef;

typedef struct
{
    u8 errorFlag[20];  //0-19 背光 低两位电压 0正常 1偏大 2偏小 ，第2、3位电流
	u32 ErrorTime[20];
	u32 oldReadTime;
}ErrorFlagDef;


typedef struct
{
	u8 ledFlag;       //背光打开时的错误
	u8 ledOnOff[2];   //背光打开或关闭命令
    u8 errorFlag[2];  //详细错误
}BlOnOffDef;         //需要设置并回读

/*
*函数名: BUSBl60_Init
*描述  : 初始化Bus通讯
*        
*参数  : 无  
*返回  ; 无
*/
void BUSBL60_Init(void);

/*
*函数名: BUSBl20_SetV
*描述  : 设置背光板的正电压
*        
*参数  : 1:group     BL60_ID0,BL60_ID1,BL60_ID2...选择对应背光机拨码器上的ID号
          2:Ce BL60_CN1   BL60_CN2 BL60_CN3//对应的是背光机的电源板ID
            3:Vol 背光板的正电压
*返回  : =0 成功，<0 失败       
*/
int BUSBL60_SetV(int group, int Ce, float Vol);

/*
*函数名: BUSBL60_SetI
*描述  : 设置各通道背光电流
*参数  :1: group 0x01 
        2: Ce 0x01 0X02 0X04
         3:Ch-通道(1-20)
          4:Current-需要设置的电压 
*返回  : =0 成功，<0 失败      
*/
int BUSBL60_SetI(int group, int Ce, int Ch, float Current);

/*
*函数名: BUSBL60_LED_ON
*描述  : 开启LED背光，背光通道设置电流後要调用此函数才会通电
*        同时会开启报警功能
*参数  :   1:group 0x01 
            2:0x01 LED1, 0X02 LED2, 0X04 LED3, …… 可或
*返回  : =0 成功，<0 失败， -49断路 -17阳极短地 -33阳极短高 -65阴极短地
*/
int BUSBL60_LED_ON(int group,int Ce,u32 chs);

/*
*函数名: LED_OFF
*描述  : 关灭LED背光，也会关闭LED报警
*参数  : Off = 0 关掉背光
*返回  : =0 成功，<0 失败 
*/
int BUSBL60_LED_OFF(int group,int Ce,int Off);

/*
*函数名: BUSBL60_StopAlarm
*描述  : 开启报警功能。设置通道限定值後，需调用此函数才能开启报警功能
*参数  : 
*返回  : =0 成功，<0 失败 
*/
int BUSBL60_StartAlarm(int group,int Ce);


/*
*函数名: BUSBL60_StopAlarm
*描述  : 关闭报警功能。设置通道限定值後，需调用此函数才能开启报警功能
*参数  : 
*返回  : =0 成功，<0 失败 
*/
int BUSBL60_StopAlarm(int group,int Ce);


/*
*函数名: BUSBL60_VMinMax
*描述  : 设置电压通道的最大最小值
*参数  : 
        1:group 0x01 
         2:Ce 0x01 0X02 0X04
          3:Ch通道号(1-20)
           4:Max最大值
            5:Min最小值
*返回  : =0 成功，<0 失败      
*/

int BUSBL60_VMinMax(int group,int Ce,int Ch,float Max,float Min);

/*
*函数名: BUSBL60_IMinMax
*描述  : 设置通道的最大最小值
*参数  : 
        1:group 0x01 
         2:Ce 0x01 0X02 0X04
          3:Ch通道号(1-20)
           4:Max最大值
            5:Min最小值
*返回  : =0 成功，<0 失败      
*/
int BUSBL60_IMinMax(int group,int Ce,int Ch,float Max,float Min);

/*
*函数名: BUSBL60Set_PWM
*描述  : 是否使用外部pwm调节背光
*参数  : OnOff- 0不使用  1使用	 
*返回  : =0 成功，<0 失败      
*/

int BUSBL60Set_PWM(int group,int Ce,int OnOff);

/*
*函数名: BUSBL60Get_BlIV
*描述:获取回读的电压电流值
*参数  : 
         1: group 1 
          2: Ce 1 3 7               
           3:回读更新的电压电流值
*             MesLEDPowerTypeDef 具体看结构体
*返回  : =0 成功，<0 失败      
*/
int BUSBL60Get_BlIV(int group, int Ce, ReLEDPowerTypeDef *IVLED);

/*
*函数名: Power_setLedOnOff
*描述  : 回读LEDONOFF的值，状态
*参数  : 
        1: group 1
         2: Ce 1 3 7  
          3 LEDOnOffDef 结构体
*返回  : =0 成功，<0 失败 
*/
int BUSBL60_ReLedOnOff(int group,int Ce,BlOnOffDef *psLEDONOFF);

/*
*函数名: BUSBL60_GetAlarmState
*描述  : 获取报警状态和时间
*参数  : 1,group 1 
          2,Ce 1 3 7 
           3,ErrorFlagDef 具体看结构体
*返回  : =0 成功，<0 失败      
*/
int BUSBL60_GetAlarmState(int group, int Ce, ErrorFlagDef *Error);

/*
*函数名: BUSBL60Set_SYNCEN1_ON 
*描述  : 使能SYNC模式 EN1 
*参数  : 1,group 1 2 4
*返回  : =0 成功，<0 失败      
*/
int BUSBL60Set_SYNCEN1_ON(int group);


/*
*函数名: BUSBL60Set_SYNCEN2_ON 
*描述  : 使能SYNC模式  EN2
*参数  : 1,group 1 2 4
*返回  : =0 成功，<0 失败      
*/
int BUSBL60Set_SYNCEN2_ON(int group);

/*
*函数名: BUSBL60Set_SYNCEN1_OFF
*描述  : 失能SYNC模式 EN1 
*参数  : 1,group 1 2 4
*返回  : =0 成功，<0 失败      
*/
int BUSBL60Set_SYNCEN1_OFF(int group);


/*
*函数名: BUSBL60Set_SYNCEN2_OFF
*描述  : 失能SYNC模式 EN2 
*参数  : 1,group 1 2 4
*返回  : =0 成功，<0 失败      
*/
int BUSBL60Set_SYNCEN2_OFF(int group);

#endif 


