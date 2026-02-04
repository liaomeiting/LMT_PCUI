/********************************* SKYCODE ************************************
* Copyright (c) 2018, 深圳思凯测试技术有限公司
* All rights reserved.
*
* 文件名   ：s10power.h    
* 描述     ：s10 电源板驱动程序
*
* 函数列表 ：
*           int POWER_Init(void);
            int POWER_GetVersion(u8 FIRM_VER[3], u8 HARD_VER[2]);
            int POWER_SetVDD(u8 ch, float v);
            int POWER_SetVDDLimit(u8 ch, float mA);
            int POWER_SetVxAlarmValue(u8 ch, float vMax, float vMin);
            int POWER_SetIxAlarmValueMA(u8 ch, float iMax, float iMin);
            int POWER_SetIxAlarmValueUA(u8 ch, float iMax, float iMin);
            int POWER_SetIxMeaRange(u8 ch, u8 range);
            int LED_SetI(u8 ledx, float mA);
			int LED_ExPWMOn();
			int LED_ExPWMOff();
			int LED_SetVoltage(float backVol);
            int LED_ON(void);
            int LED_OFF(void);
            int LED_SetVxAlarmValue(float vMax, float vMin);
            int LED_SetIxAlarmValue(float iMax, float iMin);
            int POWER_GetMeasure(PowerMeasureTypeDef* MeterMeasure);
			int POWER_GetAlarmFlag(PowerAlarmTypeDef* Alarm);
            int WAVE_SetIDRAlarmValue(float rMax, float rMin);
            int WAVE_SetIDVAlarmValue(float vMax, float vMin);
            int WAVE_SetDutyxAlarmValue(u8 ch,float dMax,float dMin);
            int WAVE_SetFreqxAlarmValue(u8 ch,float fMax,float fMin);	
            int WAVE_GetMeasure(WaveMeasureTypeDef* WaveMeasure);
            int S10_StartAlarm(void);
            int S10_StopAlarm(void);
            int MONITOR_Printf(const char *fmt, ...);
            int MONITOR_ClearPrintf(void);
            int MONITOR_ShowPage(u8 page);
            int MONITOR_SetProjectName(char *str);
            int MONITOR_SetName(u8 type, char *name);
            int MONITOR_GetIP(u8 ipAddr[4]);
            int MONITOR_SetIP(u8 ipAddr[4]);
            int MONITOR_GetPN(u8 *PN);
            int MONITOR_SetPN(u8 *PN);
            int POWER_SetVersionCheck(char *version);
*
* 版本     ：V1.14.0
* 作者     ：
* 完成日期 ：2023-04-04
* 修改描述 ：1.增加卡版本设置函数,对应版本才能正常使用电源板功能
*
*
*
* 版本     ：V1.13.0
* 作者     ：
* 完成日期 ：2019-10-17
* 修改描述 ：1.增加ui工程名部分 多行
*
*
* 版本     ：V1.12.0
* 作者     ：
* 完成日期 ：2019-8-5
* 修改描述 ：1.修改LEDOFF时的半秒延时
*
* 版本     ：V1.11.0
* 作者     ：
* 完成日期 ：2019-1-15
* 修改描述 ：1.增加小电阻测量（0-60欧）（电源板至少为FV3.0.1）
*
* 版本     ：V1.10.0
* 作者     ：
* 完成日期 ：2018-12-18
* 修改描述 ：1.修复回读Power测量值兼容性bug，读老版本电源板，仍能返回前五路报警状态
*            
*
* 版本     ：V1.9.0
* 作者     ：
* 完成日期 ：2018-11-17
* 修改描述 ：1.增加可外输入PWM信号控制背光电流（电源板至少为S10 FV3.0.0, H10 FV2.0.0）
*			 2.增加直接设置背光电压（电源板至少为S10 FV3.0.0, H10 FV2.0.0）
*
* 版本     ：V1.8.0
* 作者     ：
* 完成日期 ：2018-8-7
* 修改描述 ：1.增加POWER_GetAlarmFlag()函数 用于获取距离上一次调用此函数时，到现在的过程中是否有报警。
*              必须配合 固件版本至少为FV2.3.0 的电源板
*
* 版本     ：V1.0.7
* 作者     ：
* 完成日期 ：2018-7-10
* 修改描述 ：1.修改VDD6 7 8 回读bug
*
* 版本     ：V1.0.6
* 作者     ：
* 完成日期 ：2018-6-30
* 修改描述 ：1.1.增加3路电源VDD6 7 8，2路背光
*
* 版本     ：V1.0.5
* 作者     ：
* 完成日期 ：2018-6-23
* 修改描述 ：1.增加电源电流过大断电保护设置函数 int POWER_SetVDDLimit(u8 ch, float mA);
*
* 版本     ：V1.0.4
* 作者     ：
* 完成日期 ：2018-5-17
* 修改描述 ：1.修改注释错误, ALV_NULL 改为 LIMITNULL
*            2.修改LED_ON()
*
* 版本     ：V1.0.3
* 作者     ：
* 完成日期 ：2018-4-28
* 修改描述 ：1.与电源板程序V1.0.10 配合，LED-开机短路标志
*
* 版本     ：V1.0.2
* 作者     ：
* 完成日期 ：2018-4-25
* 修改描述 ：1.修改报警标志的宏值
*            2.修改背光电流报警值单位的漏洞
*
* 版本     ：V1.0.1
* 作者     ：
* 完成日期 ：2018-4-20
* 修改描述 ：1.修改方波频率报警 类型
*            2.修改蜂鸣器设置bug
*							            
*							        
******************************************************************************/

#ifndef __S10POWER_H
#define	__S10POWER_H

#include "stm32f4xx.h"
#include <stdio.h>

#define LIMITNULL 10000000.0  //不设报警时传递的参数


/*通道类型和通道号宏定义*/
#define WAVE1_NAME 0x00
#define WAVE2_NAME 0x01
#define VDD1_NAME 0x02
#define VDD2_NAME 0x03
#define VDD3_NAME 0x04
#define VDD4_NAME 0x05
#define VDD5_NAME 0x06


/*电流量程宏定义*/
#define RANGE_MA 0X00 //mA档 量程0.0~800.0mA 
#define RANGE_UA 0X01 //uA档 量程0 ~ 8000uA

/*报警状态宏定义*/
#define NUL_ALARM 0X00 //无报警
#define MAX_ALARM 0X01 //上限报警
#define MIN_ALARM 0X02 //下限报警

/*颜色宏定义*/
#define GUI_BLUE          0x00FF0000
#define GUI_GREEN         0x0000FF00
#define GUI_RED           0x000000FF
#define GUI_CYAN          0x00FFFF00
#define GUI_MAGENTA       0x00FF00FF
#define GUI_YELLOW        0x0000FFFF
#define GUI_LIGHTBLUE     0x00FF8080
#define GUI_LIGHTGREEN    0x0080FF80
#define GUI_LIGHTRED      0x008080FF
#define GUI_LIGHTCYAN     0x00FFFF80
#define GUI_LIGHTMAGENTA  0x00FF80FF
#define GUI_LIGHTYELLOW   0x0080FFFF
#define GUI_DARKBLUE      0x00800000
#define GUI_DARKGREEN     0x00008000
#define GUI_DARKRED       0x00000080
#define GUI_DARKCYAN      0x00808000
#define GUI_DARKMAGENTA   0x00800080
#define GUI_DARKYELLOW    0x00008080
#define GUI_WHITE         0x00FFFFFF
#define GUI_LIGHTGRAY     0x00D3D3D3
#define GUI_GRAY          0x00808080
#define GUI_DARKGRAY      0x00404040
#define GUI_BLACK         0x00000000
#define GUI_BROWN         0x002A2AA5
#define GUI_ORANGE        0x0000A5FF
#define GUI_TRANSPARENT   0xFF000000


/*VDD 和 LED背光测量值结构体*/
typedef struct
{
    /*VDD测量值。数组 [0]对应VDD1 [1]对应VDD2 ..[4]对应VDD5*/
    float VDD_V[8];       //VDD 电压测量值 单位V
    u8    VDD_V_Alarm[8]; //VDD 电压报警状态, =0x0无报警, =0x1下限报警，=0x2上限报警，参看上面宏定义"报警状态宏定义"
    
    float VDD_I_uA[8];    //VDD 电流测量值 以uA为单位 I_uA = I_mA*1000
    float VDD_I_mA[8];    //VDD 电流测量值 以mA为单位 I_mA = I_uA/1000
    u8    VDD_I_Alarm[8]; //VDD 电流报警状态，=0x0无报警, =0x1下限报警，=0x2上限报警，参看上面宏定义"报警状态宏定义"
    
    /*背光测量值*/
    float LED_V;       //背光电压
    u8    LED_V_Alarm; //背光电压报警状态标志。=0x0无报警, =0x1下限报警，=0x2上限报警，参看上面宏定义"报警状态宏定义"
    
    float LED_I;       //背光总电流
    u8    LED_I_Alarm; //背光总电流报警状态标志。=0x0无报警, =0x1下限报警，=0x2上限报警，参看上面宏定义"报警状态宏定义"
}PowerMeasureTypeDef;


typedef struct
{
    /*方波测量值。数组[0] 对应WAVE1，[1]对应WAVE2 */ 
    float WAVE_F[2];        //频率 单位 Hz
    u8    WAVE_F_Alarm[2];  //频率报警状态,=0x0无报警, =0x1下限报警，=0x2上限报警，参看上面宏定义"报警状态宏定义"
    
    float WAVE_D[2];        //占空比 单位 %
    u8    WAVE_D_Alarm[2];  //占空比报警状态,=0x0无报警, =0x1下限报警，=0x2上限报警，参看上面宏定义"报警状态宏定义"
    
    /*电阻型ID 和 电压型ID*/
    float ID_R;        //电阻型ID 单位 kohm
    u8    ID_R_Alarm;  //电阻型ID报警状态，=0x0无报警, =0x1下限报警，=0x2上限报警，参看上面宏定义"报警状态宏定义"
    
    float ID_V;        //电压型ID 单位 V
    u8 ID_V_Alarm;  //电压型ID报警状态，=0x0无报警, =0x1下限报警，=0x2上限报警，参看上面宏定义"报警状态宏定义"
}WaveMeasureTypeDef;

typedef struct
{
    /*电阻型ID 和 电压型ID*/
    float SMR;        //电阻 单位 ohm
    u8    SMR_Alarm;  //电阻型ID报警状态，=0x0无报警, =0x1下限报警，=0x2上限报警，参看上面宏定义"报警状态宏定义"
    
}SMRMeasureTypeDef;

typedef struct
{
	u8    LED_V_Alarm; //背光电压报警状态标志。=0x0无报警, =0x1下限报警，=0x2上限报警，参看上面宏定义"报警状态宏定义"
	u8    LED_I_Alarm; //背光总电流报警状态标志。=0x0无报警, =0x1下限报警，=0x2上限报警，参看上面宏定义"报警状态宏定义"
	u32   LED_Alarm_Time; //背光电压报警时间， 从上一次读取函数开始，第一次报警的时间到这次读取的时间 单位ms
	u8    VDD_V_Alarm[8]; //VDD 电压报警状态, =0x0无报警, =0x1下限报警，=0x2上限报警，参看上面宏定义"报警状态宏定义"
	u8    VDD_I_Alarm[8]; //VDD 电流报警状态，=0x0无报警, =0x1下限报警，=0x2上限报警，参看上面宏定义"报警状态宏定义"
	u32   VDD_Alarm_Time[8]; //VDD电压报警时间， 从上一次读取函数开始，第一次报警的时间到这次读取的时间 单位ms
	
}PowerAlarmTypeDef;


/*
*函数名: POWER_Init
*描述  : 初始电源板驱动程序
*参数  : 无
*返回  : =0 初始化成功，<0 失败      
*/
extern int POWER_Init(void);

/*
*函数名: POWER_GetVersion
*描述  : 读取MONITOR 板 版本号
*参数  : FIRM_VER  固件版本号
		 HARD_VER  硬件版本号
*返回  : =0 成功，<0 失败      
*/
extern int POWER_GetVersion(u8 FIRM_VER[3], u8 HARD_VER[2]);
	

/*
*函数名: POWER_SetVDD
*描述  : 设置某VDD通道电压
*参数  : 
    1. ch     : 通道号，值范围1~8。 =1 代表VDD1
    2. v      : 要输出的电压值，单位V, 精度 0.1V
                VDD5输出的是负电压，设置VDD5时,参数v 需为负值，如 -5.5
*返回  : =0 设置成功，<0 失败 
*/
extern int POWER_SetVDD(u8 ch, float v);

/*
*函数名: POWER_SetVDDLimit
*描述  : 设置某VDD通道达到多大电流时断电保护，（默认为800mA）
*参数  : 
    1. ch     : 通道号，值范围1~8。 =1 代表VDD1
    2. mA     : 要设置的电流保护值，单位mA, 
*返回  : =0 设置成功，<0 失败 
*/
extern int POWER_SetVDDLimit(u8 ch, float mA);
    
/*
*函数名: POWER_SetVxAlarmValue
*描述  : 设置某通道 电压报警限定值
*参数  : 
    1. ch     : 通道号，值范围1~8。 =1 代表VDD1
    2. vMax   : 电压上限值，单位V，精度0.01V，若不监控此项需赋值为 LIMITNULL
    3. vMin   : 电压下限值，单位V，精度0.01V，若不监控此项需赋值为 LIMITNULL
*返回  : =0 设置成功，<0 失败      
*/
extern int POWER_SetVxAlarmValue(u8 ch, float vMax, float vMin);


/*
*函数名: POWER_SetIxAlarmValueMA
*描述  : 设置某通道 某端口的电流报警限定值  毫安为单位
*参数  : 
    1. ch      : 通道号，值范围1~8。 =1 代表VDD1
    2. vMax    : 电流上限值，以mA为单位,精度0.1mA若不监控此项需赋值为 LIMITNULL
    3. vMin    : 电流下限值，以mA为单位,精度0.1mA若不监控此项需赋值为 LIMITNULL
*返回  : =0 设置成功，<0 失败     
*/
extern int POWER_SetIxAlarmValueMA(u8 ch, float iMax, float iMin);


/*
*函数名: POWER_SetIxAlarmValueUA
*描述  : 设置某通道 某端口的电流报警限定值 微安为单位
*参数  : 
    1. ch      : 通道号，值范围1~8。 =1 代表VDD1
    2. vMax    : 电流上限值，以mA为单位,精度0.1uA若不监控此项需赋值为 LIMITNULL
    3. vMin    : 电流下限值，以mA为单位,精度0.1uA若不监控此项需赋值为 LIMITNULL
*返回  : =0 设置成功，<0 失败     
*/
extern int POWER_SetIxAlarmValueUA(u8 ch, float iMax, float iMin);

/*
*函数名: POWER_SetIxMeaRange
*描述  : 设置某通道 某端口的电流测量量程(档位)
*参数  : 
    1. ch    : 通道号，值范围1~8。 =1 代表VDD1
    2. rang  : 量程 =RANGE_MA mA量程；=RANGE_UA uA量程
*返回  : =0 设置成功，<0 失败通信失败      
*/
extern int POWER_SetIxMeaRange(u8 ch, u8 range);


/*
*函数名: LED_SetI
*描述  : 设置某LED通道电流
*参数  : ledx - LED通道号 1~6
*        mA   - 电流值 单位mA, 精度0.1mA
*返回  : =0 成功，<0 失败
*/
extern int LED_SetI(u8 ledx, float mA);

/*
*函数名: LED_ExPWMOn
*描述  : 开启 使用外部PWM控制背光电流(默认为关闭)
*参数  : 无
*返回  : =0 成功，<0 失败
*/
int LED_ExPWMOn(void);

/*
*函数名: LED_ExPWMOff
*描述  : 关闭 使用外部PWM控制背光电流的功能(默认为关闭)
*参数  : 无
*返回  : =0 成功，<0 失败
*/
int LED_ExPWMOff(void);

/*
*函数名: LED_SetVoltage
*描述  : 直接设置LED背光电压，LED+对GND的电压；设置为0是自动升压
*参数  : backVol :电压值 ，单位 V
*返回  : =0 成功，<0 失败
*/
int LED_SetVoltage(float backVol);

/*
*函数名: LED_ON
*描述  : 开启LED背光，背光通道设置电流後要调用此函数才会通电
*        同时会开启报警功能
*参数  : 无
*返回  : =0 成功，<0 失败， -49断路 -17阳极短地 -33阳极短高 -65阴极短地
*/
extern int LED_ON(void);


/*
*函数名: LED_OFF
*描述  : 关灭LED背光，也会关闭LED报警
*参数  : 无
*返回  : =0 成功，<0 失败
*/
extern int LED_OFF(void);


/*
*函数名: LED_SetVxAlarmValue
*描述  : 设置电压报警限定值
*参数  : 
    1. vMax   : 电压上限值，单位V，精度0.01V，若不监控此项需赋值为 LIMITNULL
    2. vMin   : 电压下限值，单位V，精度0.01V，若不监控此项需赋值为 LIMITNULL
*返回  : =0 设置成功，<0 失败      
*/
extern int LED_SetVxAlarmValue(float vMax, float vMin);


/*
*函数名: LED_SetIxAlarmValue
*描述  : 设置某通道 某端口的电流报警限定值  毫安为单位
*参数  : 
    1. vMax    : 电流上限值，以mA为单位,精度0.1mA若不监控此项需赋值为 LIMITNULL
    2. vMin    : 电流下限值，以mA为单位,精度0.1mA若不监控此项需赋值为 LIMITNULL
*返回  : =0 设置成功，<0 失败     
*/
extern int LED_SetIxAlarmValue(float iMax, float iMin);



/*
*函数名: POWER_GetMeasure
*描述  : 回读VDD LED的测量值和报警状态
*参数  : 
    1. MeterMeasure : 测量值，请看结构体 MeterMeasureTypeDef的定义
*返回  : =0 成功，<0 失败 
*/
extern int POWER_GetMeasure(PowerMeasureTypeDef* MeterMeasure);

/*
*函数名: POWER_GetWaringFlag
*描述  : 回读距离上一次调用此函数时，到现在的过程中是否有报警，具体看结构体定义
*参数  : 
    1. Alarm : 所有通道的回读报警状态保存值与记录的时间，请看结构体 PowerAlarmTypeDef定义
*返回  : =0 成功，<0 失败 
*/
int POWER_GetAlarmFlag(PowerAlarmTypeDef* Alarm);
	

///////////////////////////////////////////////////////////////////////////////


/*
*函数名: WAVE_SetIDVAlarmValue
*描述  : 设置IDR报警限定值
*参数  : 
    1. rMax   : 电阻上限值，单位千欧，精度0.1k，若不监控此项需赋值为 LIMITNULL
    2. rMin   : 电阻下限值，单位千欧，精度0.1k，若不监控此项需赋值为 LIMITNULL
*返回  : =0 设置成功，<0 失败      
*/
extern int WAVE_SetIDRAlarmValue(float rMax, float rMin);


/*
*函数名: WAVE_SetIDVAlarmValue
*描述  : 设置IDV报警限定值
*参数  : 
    1. vMax   : 电压上限值，单位V，精度0.01V，若不监控此项需赋值为 LIMITNULL
    2. vMin   : 电压下限值，单位V，精度0.01V，若不监控此项需赋值为 LIMITNULL
*返回  : =0 设置成功，<0 失败      
*/
extern int WAVE_SetIDVAlarmValue(float vMax, float vMin);


/*
*函数名: WAVE_SetDutyxAlarmValue
*描述  : 设置某通道 占空比的限定范围
*参数  : 
    1. ch      : 通道号，值范围1~2。 =1 代表WAVE1
    2. dMax    : 上限值，取值范围 0~100.0，若不监控此项需赋值为 LIMITNULL
    3. dMin    : 下限值，取值范围 0~100.0，若不监控此项需赋值为 LIMITNULL
*返回  : =0 设置成功，<0 失败     
*/
extern int WAVE_SetDutyxAlarmValue(u8 ch,float dMax,float dMin);

/*
*函数名: WAVE_SetFreqxAlarmValue
*描述  : 设置某通道 频率的限定范围
*参数  : 
    1. ch      : 通道号，值范围1~2。 =1 代表WAVE1
    2. fMax    : 上限值，取值范围 20~500000.0，若不监控此项需赋值为 LIMITNULL
    3. fMin    : 下限值，取值范围 20~500000.0，若不监控此项需赋值为 LIMITNULL
*返回  : =0 设置成功，<0 失败     
*/
extern int WAVE_SetFreqxAlarmValue(u8 ch,float fMax,float fMin);	

/*
*函数名: WAVE_GetMeasure
*描述  : 回读方波 ID（电压/电阻型）测量值，报警状态
*参数  : 
    1. WaveMeasure : 所有回读值，请看结构体 WaveMeasureTypeDef定义
*返回  : =0 成功，<0 失败 
*/
extern int WAVE_GetMeasure(WaveMeasureTypeDef* WaveMeasure);

///////////////////////////////////////////////////////////////////////////////

/*
*函数名: S10_StartAlarm
*描述  : 开启报警功能。设置通道限定值後，需调用此函数才能开启报警功能
*参数  : 
*返回  : =0 成功，<0 失败 
*/
extern int S10_StartAlarm(void);


/*
*函数名: S10_StopAlarm
*描述  : 关闭报警功能
*参数  : 
*返回  : =0 成功，<0 失败 
*/
extern int S10_StopAlarm(void);

///////////////////////////////////////////////////////////////////////////////

/*
*函数名: MONITOR_Printf
*描述  : 打印输出语句
*参数  : 与printf相同
*返回  : =0 成功，<0 失败
*/
int MONITOR_Printf(const char *fmt, ...);

/*
*函数名: MONITOR_ClearPrintf
*描述  : 清除MONITOR_Printf打印界面，光标定位在第一行
*参数  : 无
*返回  : =0 成功，<0 失败
*/
extern int MONITOR_ClearPrintf(void);

/*
*函数名: MONITOR_SetBee
*描述  : 蜂鸣器控制
*参数  : bee_onoff - 设置蜂鸣器状态； =1响蜂鸣器，=0关蜂鸣器
*返回  : =0 初始化成功，<0 失败      
*/
extern int MONITOR_SetBee(u8 bee_onoff);

/*
*函数名: MONITOR_ShowPage
*描述  : 设置显示哪个页面
*参数  : page - (0~10)， 第10页是printf打印界面
*返回  : =0 初始化成功，<0 失败      
*/
extern int MONITOR_ShowPage(u8 page);

/*
*函数名: MONITOR_SetProjectName
*描述  : 设置第0页底部信息
*参数  : str - 字符串
*返回  : =0 初始化成功，<0 失败      
**/
extern int MONITOR_SetProjectName(char *str);

/*
*函数名: MONITOR_SetName
*描述  : 设置各通道显示的名称
*参数  : type - (0-6)通道类型和通道号，请用上面的"通道类型和通道号宏定义"
*        name - 名称字符串
*返回  : =0 初始化成功，<0 失败      
*/
extern int MONITOR_SetName(u8 type, char *name);

/*
*函数名: MONITOR_GetIP
*描述  : 获取显示屏保存的ip
*参数  : ipAddr - IP数组
*返回  : =0 初始化成功，<0 失败      
*/
extern int MONITOR_GetIP(u8 ipAddr[4]);

/*
*函数名: MONITOR_SetIP
*描述  : 设置IP并保存
*参数  : ipAddr - IP数组
*返回  : =0 初始化成功，<0 失败      
*/
extern int MONITOR_SetIP(u8 ipAddr[4]);

/*
*函数名: MONITOR_GetPN
*描述  : 获取显示屏保存的PNumber
*参数  : 返回数据保存位置
*返回  : =0 初始化成功，<0 失败      
*/
extern int MONITOR_GetPN(u8 *PN);

/*
*函数名: MONITOR_SetPN
*描述  : 设置PNumber并保存  
*参数  : PN值  1~5
*返回  : =0 初始化成功，<0 失败      
*/
extern int MONITOR_SetPN(u8 PN);

/*
*函数名: SMR_SetAlarmValue
*描述  : 设置SMR报警限定值(外扩小电阻)
*参数  : 
    1. rMax   : 电阻上限值，单位千欧，精度0.1k，若不监控此项需赋值为 LIMITNULL
    2. rMin   : 电阻下限值，单位千欧，精度0.1k，若不监控此项需赋值为 LIMITNULL
*返回  : =0 设置成功，<0 失败      
*/
int SMR_SetAlarmValue(float rMax, float rMin);

/*
*函数名: SMR_GetMeasure
*描述  : 回读电压值与报警状态
*参数  : 
    1. MeterMeasure : 所有通道的回读值，请看结构体 MeterMeasureTypeDef的定义
*返回  : =0 成功，<0 失败 
*/
int SMR_GetMeasure(SMRMeasureTypeDef* MeterMeasure);

/*
*函数名: Power_SetMonitorStringName
*描述  : 设置Monitor中的名字
*参数  : 
    1. u8 ch,char *name : 请看结构体 DataTypeDef定义
*返回  : =0 成功，<0 失败 
*/
int Power_SetMonitorStringName(u8 ch,char *name);
/*
*函数名: Power_SetMonitorStringBackColor
*描述  : 设置Monitor中的名字的背景颜色
*参数  : 
    1. u8 ch,u32 Color : 请看结构体 DataTypeDef定义
*返回  : =0 成功，<0 失败 
*/
int Power_SetMonitorStringBackColor(u8 ch,u32 Color);

int POWER_SetVersionCheck(char *version);

#endif 

/********************************* SKYCODE ***********************************/
