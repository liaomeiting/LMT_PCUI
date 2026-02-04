/********************************* SKYCODE ************************************
* Copyright (c) 2019, 深圳思凯测试技术有限公司
* All rights reserved.
*
* 文件名   ：S10.c    
* 描述     ：H10_8LANE 测机底层驱动初始化
*
* 版本     ：V1.1.0
* 作者     ：
* 完成日期 ：2019-10-17
* 修改描述 ：s10power V1.1.0 增加一些UI的设置
*
* 版本     ：V1.0.0
* 作者     ：
* 完成日期 ：2019-8-26
* 修改描述 ：继承S10 S10.C V2.9.0
*       程序版本组合
        1.lwip      - V1.0.1
        2.sd_file   - V2.5.0
        --S10DRV.lib
        3.gpu       - V1.0.0
        4.s10power  - V1.12.0
        5.OtherVdd  - V1.0.3
        6.key       - V1.3.0
        7.S10       - V2.5.0
        8.versionNumber - V1.0.0
         							                          	
******************************************************************************/

#include "S10.h"
#include "sys.h"
#include "versionNumber.h"
#include "bus.h"
#include "ssd2828.h"
#include "sd_file.h"



void NVIC_Configuration(void);

/*---------------------------------------------------------------------------*/
void S10_Init(void)
{
    int res=0;
    int init_state=1;
    u8 firm_ver[3], hard_ver[2];
    
    /*不可更改*/
    SCB->VTOR = FLASH_BASE | 0x10000;
    
//    POWER_SetVersionCheck("0");
    
    SysTick_Init();
    NVIC_Configuration();
    Version_Init();
    
    Bus_Init();
    
    OTHERVDD_Init();
        
    SSD2828_Init();

    Delay_ms(1000);
    
    res=POWER_Init();
    
    MONITOR_ShowPage(10);

    if(res==0)
    {
        MONITOR_Printf("POWER Init OK\r\n");
        
        POWER_GetVersion(firm_ver, hard_ver);
        MONITOR_Printf("  FV%d.%d.%d, HV%d.%d\r\n", 
                        firm_ver[0],firm_ver[1],firm_ver[2],hard_ver[0],hard_ver[1]);    
    }
    else
    {
        MONITOR_Printf("POWER Init NG\r\n");
        init_state = -1;
    }
     
    _DEBUG("CORE\r\n  DrvV%d.%d.%d, HV%d.%d\r\n",g_software_ver[0],g_software_ver[1],g_software_ver[2],g_hardware_ver[0],g_hardware_ver[1]);
    
    /*初始化GPU*/
    res=GPU_Init();
    if(res==0)
    {
        GPU_GetVersion(firm_ver);
        _DEBUG("  FV%d.%d.%d\r\n", firm_ver[0],firm_ver[1],firm_ver[2]);
    }
    else
    {
        init_state = -1;
    }
    
    
    
    KEY_Init();
    
    SDCard_Init();
    
    /*基础硬件出错响蜂鸣器10s*/    
    if(init_state!=1)
    {
        MONITOR_SetBee(1);
        Delay_ms(10000);
        MONITOR_SetBee(0);
        while(1);
    }
    /*---*/
}

/*不可修改*/
void NVIC_Configuration(void)
{

  /* Configure the NVIC Preemption Priority Bits */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);	//

}

/********************************* SKYCODE ***********************************/
