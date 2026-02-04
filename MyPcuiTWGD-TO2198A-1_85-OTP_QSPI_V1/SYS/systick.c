/********************************* SKYCODE *************************************
* Copyright (c) 2014-2016, 深圳思凯测试技术有限公司（廣州天碼電子技術有限公司）
* All rights reserved.
*
* 文件名   ：systick.c 
* 描述     : TICK时钟程序
*
* 版    本 ：V03
* 作者     ：SKYCODE2013
* 完成日期 ：2016-11-12
* 修改描述 ：1. SYYTICK 1ms, 可修改SYSTICK_FRE_HZ来改变
*
* 版    本 ：V02
* 作者     ：SKYCODE2013
* 完成日期 ：2014-09-11
* 修改描述 ：1.增加測量時間的兩個函數
*
* 版    本 ：V01
* 作者     ：SKYCODE2013
* 完成日期 ：2013
* 修改描述 ：
*							                          	
*******************************************************************************/

#include "SysTick.h"


static u32 sg_systick_1ms_con=0;

static uint32_t _20ms_cnt;
unsigned int _1ms_ok = 0;
unsigned int _20ms_ok = 0;

unsigned int task_time_cnt[MAX_TASK]; 

typedef struct
{
    unsigned int td; //
    void (*fp)(void);
}TaskStruct;

static TaskStruct sg_task[MAX_TASK];


/********************************************************************************
*函數名：SysTick_Init
*描述  ：啟動系統Tick時鐘
*参数  ：无
*返回  ：無
********************************************************************************/
void SysTick_Init(void)
{
	unsigned int i;
    
    for(i=0; i<MAX_TASK; i++)
    {
        sg_task[i].td = 0;
    }
        
    if (SysTick_Config(SystemCoreClock / SYSTICK_FRE_HZ))
    { 
        while (1);
    }
}


/********************************************************************************
*函數名：TimingDelay_Decrement
*描述  ：Tick時鐘的中斷函數，在SysTick_Handler()调用
*参数  ：無
*返回  ：無
********************************************************************************/ 
void SysTick_Handler(void)
{
	unsigned int i;
	
//    SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk; //關SysTick中斷
    
    ++sg_systick_1ms_con;
    
    _1ms_ok = 1;


    //多任務任務時間計時
    for(i = 0; i < MAX_TASK; i++)
    {
        if(task_time_cnt[i] > 0)
        {
            task_time_cnt[i]--;
        }
        else
        {
            if(sg_task[i].td != 0)
            {
                task_time_cnt[i] = sg_task[i].td;
                sg_task[i].fp(); //執行程序
                break;     //每次只執行一個程序
            }
        }
    }


	if(++_20ms_cnt >= 20)
	{
		_20ms_cnt = 0;
		_20ms_ok = 1;
	}
    
 //   SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk; //開SysTick中斷
}

/********************************************************************************
*函數名：MeasureTimeStart_ms
*描述  ：獲取初始時間，開始測量時間
*参数  ：無
*返回  ：當前時間計算值 单位ms
********************************************************************************/
unsigned int MeasureTimeStart_ms(void)
{
	return sg_systick_1ms_con;
}


/********************************************************************************
*函數名：GetMeasureTime_ms
*描述  ：獲取 調用MeasureTimeStart_ms()時到現在時間長度 
*参数  ：ts-是初試時間，就是調用MeasureTimeStart_ms()函數時的返回值。
            本函數要根據這個初始時間來計算時間差
*返回  ：調用MeasureTimeStart_ms()時到現在時間長度 单位ms
********************************************************************************/
unsigned int GetMeasureTime_ms(unsigned int ts)
{
	u32 tmp;
	tmp = sg_systick_1ms_con;
    if(tmp < ts)
	{
		tmp = 0xffffffff - ts + tmp;
	}
	else
	{
		tmp = tmp - ts;
	}
	return tmp;
}


/********************************************************************************
*函數名：Sys_CreateTask
*描述  ：添加需要定時執行的函數
*参数  ：task_num - 任務編號
         fp       - 函數指針
         td       - 間隔時間，即每隔td時間 執行一次fp函數
*返回  ：調用MeasureTimeStart()時到現在時間長度 单位1个SYSTICK (1ms)
********************************************************************************/
void Sys_CreateTask(u8 task_num, void (*fp)(void), unsigned int td)
{
    sg_task[task_num].fp = fp;
    sg_task[task_num].td = td;
    task_time_cnt[task_num] = td;
}

/********************************* SKYCODE ************************************/
