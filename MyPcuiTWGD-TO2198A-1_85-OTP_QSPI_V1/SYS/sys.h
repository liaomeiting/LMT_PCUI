
#ifndef _SYS_H_
#define _SYS_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "systick.h"
#include "delay.h"

#include "debug_def.h"

// /* Exported types ------------------------------------------------------------*/
// typedef int32_t  s32;
// typedef int16_t s16;
// typedef int8_t  s8;

// typedef const int32_t sc32;  /*!< Read Only */
// typedef const int16_t sc16;  /*!< Read Only */
// typedef const int8_t sc8;   /*!< Read Only */

// typedef __IO int32_t  vs32;
// typedef __IO int16_t  vs16;
// typedef __IO int8_t   vs8;

// typedef __I int32_t vsc32;  /*!< Read Only */
// typedef __I int16_t vsc16;  /*!< Read Only */
// typedef __I int8_t vsc8;   /*!< Read Only */

// typedef uint32_t  u32;
// typedef uint16_t u16;
// typedef uint8_t  u8;

// typedef const uint32_t uc32;  /*!< Read Only */
// typedef const uint16_t uc16;  /*!< Read Only */
// typedef const uint8_t uc8;   /*!< Read Only */

// typedef __IO uint32_t  vu32;
// typedef __IO uint16_t vu16;
// typedef __IO uint8_t  vu8;

// typedef __I uint32_t vuc32;  /*!< Read Only */
// typedef __I uint16_t vuc16;  /*!< Read Only */
// typedef __I uint8_t vuc8;   /*!< Read Only */

typedef enum {FALSE = 0, TRUE = !FALSE} bool;



/* Exported functions ------------------------------------------------------- */

/*
*函數名：Sys_CreateTask
*描述  ：添加需要定時執行的函數
*参数  ：task_num - 任務編號
         fp       - 函數指針
         td       - 間隔時間，即每隔td時間 執行一次fp函數
*返回  ：調用MeasureTimeStart()時到現在時間長度 单位1个SYSTICK (1ms)
*/
void Sys_CreateTask(u8 task_num, void (*fp)(void), unsigned int td);

#endif
