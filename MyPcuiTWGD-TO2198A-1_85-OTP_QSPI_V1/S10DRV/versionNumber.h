/********************************* SKYCODE ************************************
* Copyright (c) 2018, 深圳思凯测试技术有限公司（广州天码电子技术有限公司）
* All rights reserved.
*
* 文件名   ：versionNumber.c   
* 描述     ：1. 版本信息记录与获取
*            2. 要给cg_ver_message[] 数组赋值正确的版本信息
*            3. 调用Version_Init()函数后,软件版本号和硬件版本号保存在
                 g_software_ver和g_hardware_ver数组中
*            4. 尽早调用 Version_Init() 函数 以免有冲突（ADC型硬件版本）
*
* 版    本 ：V1.0.0
* 作    者 ：
* 完成日期 ：2018-7-30
* 修改描述 ：
*           					           						                          	
******************************************************************************/

#ifndef  __VERSIONNUMBER_H__
#define  __VERSIONNUMBER_H__

#include "sys.h"

/*
调用Version_Init()函数之后，保存着软件版本号和硬件版本号的数组
[0]是主版本号，[1]是次版本号
千万不要给这两个数组赋值。
*/
extern u8 g_software_ver[3]; //软件版本号
extern u8 g_hardware_ver[2]; //硬件版本号

/**/
void Version_Init(void);


#endif

