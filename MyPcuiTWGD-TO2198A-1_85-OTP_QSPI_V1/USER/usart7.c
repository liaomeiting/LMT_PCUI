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

#include "usart7.h"
#include <stdarg.h>
							
/******************************************************************************
 * 函数名：USART7_Init
 * 描述  ：USART7 GPIO 配置,工作模式配置。115200 8bit 無奇偶校驗 1停止位
 * 输入  ：无
 * 输出  : 无
 * 调用  ：外部调用
 *****************************************************************************/
void USART7_Init(void)
{
	
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF,ENABLE); //使能GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART7,ENABLE);//使能USART7时钟
 
	//串口7对应引脚复用映射
	GPIO_PinAFConfig(GPIOF,GPIO_PinSource6,GPIO_AF_UART7); //复用为UART7
	GPIO_PinAFConfig(GPIOF,GPIO_PinSource7,GPIO_AF_UART7); //复用为UART7
	
	//USART7端口配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; //GPIOA9与GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOF,&GPIO_InitStructure); //初始化PA9，PA10
	
	/* USART7 mode config */
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx;
	USART_Init(UART7, &USART_InitStructure); 
	USART_Cmd(UART7, ENABLE);
     
    
}

/********************************************************************************
 * 函数名：fputc
 * 描述  ：重定向c库函数printf到USART7
 * 输入  ：无
 * 输出  ：无
 * 调用  ：由printf调用
 ********************************************************************************/
//int fputc(int ch, FILE *f)
//{
//	while((UART7->SR&0X40)==0);//循环发送,直到发送完毕   
//	UART7->DR = (u8) ch;      
//	return ch;
//}

/******************************* 广州天码电子 *********************************/
