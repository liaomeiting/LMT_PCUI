/********************************* SKYCODE ************************************
* Copyright (c) 2015-2017, 深圳思凯测试技术有限公司
* All rights reserved.
*
* 文件名   ：usart7.h    
* 描述     ：
*
* 版    本 ：V1.0.0
* 作    者 ：
* 完成日期 ：2017-12-29
* 修改描述 ：
*					                          	
******************************************************************************/

#include "driverUsart3.h"
#include <stdarg.h>
#include "myFIFO.h"
#include "commonIOInterface.h"


static sky_comDriver Usart3_Driver;
static int USART3Write(struct _Tsky_comDriver *self, u8 *data,int len);

/******************************************************************************
 * 函数名：USART7_Init
 * 描述  ：USART7 GPIO 配置,工作模式配置。115200 8bit 無奇偶校驗 1停止位
 * 输入  ：无
 * 输出  : 无
 * 调用  ：外部调用
 *****************************************************************************/
sky_comDriver* USART3_Init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
    
    /**/

	memset(&Usart3_Driver,0,sizeof(sky_comDriver));
	Usart3_Driver.write = USART3Write;
	
	/**/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE); //使能GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);//使能USART7时钟
 
	//串口7对应引脚复用映射
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource8,GPIO_AF_USART3); //复用为USART3
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource9,GPIO_AF_USART3); //复用为USART3
	
	//USART7端口配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9; //GPIOA9与GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOD,&GPIO_InitStructure); //初始化PA9，PA10
	
//	//USART3 NVIC 配置
//	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;//串口1中断通道
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;//抢占优先级1
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority =4;		//子优先级4
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
//	NVIC_Init(&NVIC_InitStructure);
	
	/* USART7 mode config */
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx|USART_Mode_Rx;
	USART_Init(USART3, &USART_InitStructure); 

	
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//开启相关中断
	
	USART_Cmd(USART3, ENABLE);
	
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//开启相关中断

	//USART3 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;//串口1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =4;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、
	
	return &Usart3_Driver;
}



static int USART3Write(struct _Tsky_comDriver *self, u8 *data,int len)
{
	while(len > 0)
	{
		while((USART3->SR&0X40)==0);//循环发送,直到发送完毕   
		USART3->DR = (u8) *data;      
		len--;
		data++;
	}
	
	return 0;
}


//$Sub$$:定义新功能函数,在USART3_IRQHandler函数(之前/之后)使用$Sub$$USART3_IRQHandler可以添加该函数一些新的程序代码
//void $Sub$$USART3_IRQHandler(void)
void USART3_IRQHandler(void)
{
	u8 Res;
	if(USART_GetITStatus(USART3,USART_IT_RXNE)!=RESET)
	{
		Res =USART_ReceiveData(USART3);
        Write2dev(&Usart3_Driver, &Res,1);

//		if (Usart3_Driver.fifo != 0)
//			myFIFOWrite(Usart3_Driver.fifo, &Res, 1);

//		if (Usart3_Driver.single != 0)
//			Usart3_Driver.single(Usart3_Driver.userData, &Res, 1);
	}
}



/******************************* 广州天码电子 *********************************/
