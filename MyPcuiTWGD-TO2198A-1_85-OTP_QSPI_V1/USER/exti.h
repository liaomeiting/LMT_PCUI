/********************************* SKYCODE *************************************
* Copyright (c) 2013-2015, 深圳思凯测试技术有限公司（广州天码电子技术有限公司）
* All rights reserved.
*
* 文件名   ：exti.h    
* 描述     ：
* 函数列表 ：
* 當前版本 ：
* 作    者 ：
* 完成日期 ：2018-06-23
* 修改描述 ：
*
*            						                          	
*******************************************************************************/
#ifndef __EXTI_H
#define	__EXTI_H

#include "stm32f4xx.h"

#define EXTPC3 		GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_3) //PC3

void EXTI_PC3_Config(void);

#endif
