#ifndef __COMMUNICATIONOLD_H
#define __COMMUNICATIONOLD_H
#include "sys.h" 
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#define OFFSETOF(TYPE, MEMBER) ((int)(&((TYPE *)0)->MEMBER))
	

	

typedef struct
{
	u8 saveFlag;	
	u8 startc;
	u8 endc;
	u32 maxlen;
	u32 dataLen;
	u16 saveNum;
	u8 *data;
	u32 oldTime;
	
	int (*receivePact)(void *this,u8 c);           					//传入接收数据  底层校验
	int (*sendPact)(void *this,u8 cmd,u8 *data,u32 len);  					//发送数据包
	int (*runCmdPact)(void *this,u8 cmd,u8 *data,u32 len,u32 time);	    //运行命令 返回数据放在缓冲区中
}MyByteReceiveDataTypeDef;



int receivePact(MyByteReceiveDataTypeDef *myRecType,u8 newc);
int runCmdPact(MyByteReceiveDataTypeDef *this,u8 cmd,u8 *data,u32 len,u32 time);
#endif

