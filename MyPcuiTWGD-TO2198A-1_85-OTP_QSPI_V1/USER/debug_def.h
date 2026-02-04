/********************************* SKYCODE *************************************
* Copyright (c) 2016, 深圳思凯测试技术有限公司（广州天码电子技术有限公司）
* All rights reserved.
*
* 文件名   ：debug_def.h    
* 描述     ：DEBUG宏定义
*
* 版    本 ：V0.1
* 作    者 ：
* 完成日期 ：2016-04-25
* 修改描述 ：
*					                          	
*******************************************************************************/

#ifndef _DEBUG_DEF_
#define _DEBUG_DEF_

#include <stdio.h>
#include "s10power.h"

//注释掉下面这句 则代码不DEBUG
#define __DEBUG 1


#ifdef __DEBUG
//#define _DEBUG(format, ...) printf("FILE: "__FILE__", LINE: %d: "format"\n\r", __LINE__, ##__VA_ARGS__) /*SEGGER_RTT_printf(0,"FILE: "__FILE__", LINE: %d: "format"\n\r", __LINE__, ##__VA_ARGS__)*/
#define _DEBUG(format, ...) MONITOR_Printf(format,##__VA_ARGS__)
#else
#define _DEBUG(format,...)   
#endif


#endif

/********************************* SKYCODE ************************************/
