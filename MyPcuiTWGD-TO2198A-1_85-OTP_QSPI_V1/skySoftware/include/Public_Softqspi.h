#ifndef _PUBLIC_QSPI_H__
#define	_PUBLIC_QSPI_H__

#include "stdint.h"

typedef struct{
	void (*f_set_cs)(uint8_t);   //CS引脚控制函数，参数=0 输出低电平，=1输出高电平
	void (*f_set_sclk)(uint8_t); //SCL引脚控制函数
	void (*f_set_io0)(uint8_t);  //数据线输出口控制函数 
	void (*f_set_io1)(uint8_t);
	void (*f_set_io2)(uint8_t);
	void (*f_set_io3)(uint8_t);
	uint8_t (*f_read_io0)(void); //数据线回读函数
	uint8_t (*f_read_io1)(void);
	uint8_t (*f_read_io2)(void);
	uint8_t (*f_read_io3)(void);
	void (*f_change_io)(uint8_t); //数据线方向切换，用于读和写时的切换,参数=1输出， 参数=0输入
	void (*f_delay)(void);  //时钟延时函数
}qspi_adapter;

typedef enum
{
//	QSPI_1L_WR = 0x00000001, //Write_Read
	QSPI_1L_W = 0x00000002,//Write_Data
	QSPI_1L_R = 0x00000004,//Read_Data;
	
//	QSPI_2L_WR = 0x00000100, //Write_Read
	QSPI_2L_W = 0x00000200, //Write_Data
	QSPI_2L_R = 0x00000400, //Read_Data;
	
//	QSPI_4L_WR = 0x00010000, //Write_Read
	QSPI_4L_W = 0x00020000, //Write_Data
	QSPI_4L_R = 0x00040000, //Read_Data;
}qspi_type;

typedef struct 
{
	uint32_t type;    	//模式
	uint32_t len;		//数据包的长度
	uint8_t dc;			//只有9位spi的时候使用
	uint8_t *txbuf;		//只有双向传输与只发送的时候使用
	uint8_t *rxbuf;		//只有双向传输与只接收的时候使用
}qspi_msg;


/***********************************
*函数名:P_QSPI_DeInit
* 描 述:默认值
* 参 数:
	adap : QSPI结构体
* 返 回:无
***********************************/	
void P_QSPI_DeInit(qspi_adapter *adap);

/***********************************
*函数名:P_QSPI_Init
* 描 述:初始化引脚状态
* 参 数:
	adap : QSPI结构体
* 返 回:无
***********************************/
void P_QSPI_Init(qspi_adapter *adap);

/***********************************
*函数名:P_QSPI_transfer
* 描 述:最终调用到得收发函数
* 参 数:
	adap : QSPI结构体
	msgs : QSPI收发参数结构体	
	msgs_num :连续收发个数
* 返 回:
	= 1 : 成功
	= 0 : 失败
***********************************/
int P_QSPI_transfer(qspi_adapter *adap,qspi_msg *msgs,uint16_t msgs_num);



#endif