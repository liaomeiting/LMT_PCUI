#ifndef __driverUsart1_H
#define __driverUsart1_H

#include "stdio.h"
#include "sys.h"
#include "commonIOInterface.h"

/*******************************************************************************
 * 函数名 : driverUSART1_Init
 * 描  述 : 串口1驱动初始化
 * 参  数 : 无
 * 返  回 : sky_comDriver结构体指针句柄
*******************************************************************************/
sky_comDriver *driverUSART1_Init(void);

/*******************************************************************************
 * 函数名 : usart1_write
 * 描  述 : 串口1写数据
 * 参  数 : *self:_Tsky_comDriver结构体指针
 * 参  数 : *data:要写的数据
 * 参  数 : len:数据长度
 * 返  回 : 0
*******************************************************************************/
int usart1_write(struct _Tsky_comDriver *self, uint8_t *data, int len);//串口1写数据
#endif
