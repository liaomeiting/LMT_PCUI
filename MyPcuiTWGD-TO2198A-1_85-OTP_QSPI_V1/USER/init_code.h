#ifndef _INIT_CODE_H_
#define _INIT_CODE_H_

#include "stm32f4xx.h"
#include "user_control.h"

/*
*函數名：Read_Compare
*描述  ：初始化回读比较
*参数  ：无
*        
*返回  ：1 回读与写入的初始化一致
*      // -1 回读出错
*        0 出现回读值与初始化写入值不一致的寄存器
*/

int ReadALLcode(void);


#endif