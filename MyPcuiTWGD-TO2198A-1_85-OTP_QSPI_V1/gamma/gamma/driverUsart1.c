#include "driverUsart1.h"

static sky_comDriver Usart1_Driver;


//static int usart1_write(struct _Tsky_comDriver *self, uint8_t *data, int len);//串口1写数据
/*******************************************************************************
 * 函数名 : driverUSART1_Init
 * 描  述 : 串口1驱动初始化
 * 参  数 : 无
 * 返  回 : sky_comDriver结构体指针句柄
*******************************************************************************/
sky_comDriver *driverUSART1_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	memset(&Usart1_Driver, 0, sizeof(sky_comDriver));
	
	Usart1_Driver.write = usart1_write;//注册写函数
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	
	USART_Cmd(USART1, ENABLE);
	
	return &Usart1_Driver;
}

/*******************************************************************************
 * 函数名 : usart1_write
 * 描  述 : 串口1写数据
 * 参  数 : *self:_Tsky_comDriver结构体指针
 * 参  数 : *data:要写的数据
 * 参  数 : len:数据长度
 * 返  回 : 0
*******************************************************************************/
int usart1_write(struct _Tsky_comDriver *self, uint8_t *data, int len)
{
	while(len > 0)
	{
		while((USART1->SR & 0x40) == 0); 
		USART1->DR = (uint8_t)*data;      
		len--;
		data++;
	}
	
	return 0;
}

/*******************************************************************************
 * 函数名 : USART1_IRQHandler
 * 描  述 : 串口1中断请求
 * 参  数 : 无
 * 返  回 : 0
*******************************************************************************/
void USART1_IRQHandler(void)
{
	uint8_t Res;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		Res = USART_ReceiveData(USART1);
		
		/* 若sky_comDriver结构体里的fifo不为空,则写fifo不为空 */
		if(Usart1_Driver.fifo != 0)
			myFIFOWrite(Usart1_Driver.fifo, &Res, 1);
		
		/* 若sky_comDriver结构体里的single不为空 */
		if(Usart1_Driver.single != 0)
			Usart1_Driver.single(&Usart1_Driver, &Res, 1);
	}
}
