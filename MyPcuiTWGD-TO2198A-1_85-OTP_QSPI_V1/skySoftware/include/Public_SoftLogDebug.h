#ifndef __PUBLIC_SOFTLOGDEBUG_H
#define __PUBLIC_SOFTLOGDEBUG_H

#include "stdlib.h"
#include "stdint.h"
#include <stdio.h>
#include "commonIOInterface.h"

struct _Tsky_comLog;

typedef struct _Tsky_comLog
{
	
	void *userData;       //bsp层自己使用的指针(可改变类型), 外部应用层尽量不要使用

	unsigned char *dataStr;		/*解析缓冲区*/
	
	sky_comDriver *dev;   
	
}sky_comLog;

int P_DbgLog_Init(sky_comDriver *driver, unsigned char *buf,int len);


int Log_output(const char *fmt, ...);














#endif




