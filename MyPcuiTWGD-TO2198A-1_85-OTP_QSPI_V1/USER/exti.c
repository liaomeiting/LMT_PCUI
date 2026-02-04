#include "exti.h"

/*
 * 函数名：EXTI_PC3_Config
 * 描述  ：配置 PC3 为线中断口
 * 输入  ：无
 * 输出  ：无
 * 调用  ：外部调用
 */
void EXTI_PC3_Config(void)
{	
    GPIO_InitTypeDef   GPIO_InitStructure;
    EXTI_InitTypeDef   EXTI_InitStructure;
    NVIC_InitTypeDef   NVIC_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);//使能GPIOC时钟

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//普通输入模式
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化GPIOC3

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);//使能SYSCFG时钟

    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource3);//PC3 连接到中断线3

    /* 配置EXTI_Line3 */
    EXTI_InitStructure.EXTI_Line = EXTI_Line3;//LINE3
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//中断事件
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; //上升沿触发 
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;//使能LINE0
    EXTI_Init(&EXTI_InitStructure);//配置

    NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;//外部中断3
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;//抢占优先级0
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;//子优先级2
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能外部中断通道
    NVIC_Init(&NVIC_InitStructure);//配置    
}


