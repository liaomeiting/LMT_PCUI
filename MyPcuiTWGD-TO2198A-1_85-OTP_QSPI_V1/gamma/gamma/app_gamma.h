#ifndef __APP_GAMMA_H
#define	__APP_GAMMA_H

#include <stdint.h>
#include "new_gamma_def.h"
#include "pcui.h"
#define SDCard_Log 

/*进模式指令*/
void go_idle(void);     
void go_hbm(void);      
void go_normal(void);   
void go_normal3(void);  
void go_normal2(void);  
void read_gamma(void);
void go_idle_5nit(void);
void go_nor_51_1F(void);
void go_PLC(void);
void ConvertAndSplit(int decimal, unsigned char* high, unsigned char* low);
void PLC_calculation(void);
void go_plc(void);
/*写寄存器操作*/
int gammaRegWriteAll_HBM(uint16_t *data);
int gammaRegWrite_HBM(int j, uint16_t *data);
/*写寄存器操作*/
int gammaRegWriteAll_Normal(uint16_t *data);
int gammaRegWrite_Normal(int j, uint16_t *data);
/*写寄存器操作*/
int gammaRegWriteAll_Normal3(uint16_t *data);
int gammaRegWrite_Normal3(int j, uint16_t *data);
/*写寄存器操作*/
int gammaRegWriteAll_Normal2(uint16_t *data);
int gammaRegWrite_Normal2(int j, uint16_t *data);
/*写寄存器操作*/
int gammaRegWriteAll_Idle(uint16_t *data);
int gammaRegWrite_Idle(int j, uint16_t *data);

int gammaRegReadAll_Normal(void);
int gammaRegReadAll_HBM(void);
int gammaRegReadAll_Idle(void);

/**
 * 函数名 : GAMMA_Inint
 * 描述   : gamma调试初始化函数
 * 参数   : 无
 * 返回   : 无返回   
*/
void GAMMA_Inint(void);



/**
 * 函数名：Gamma_Correction
 * 描述：gamma调试接口函数
 * 参数：
 * Pid_Net： 该选项选择调节gamma方式
 *			 =1 -执行模型调试
 *			 =0 -执行PID调试
 * Get_Data: 该选项配合PID调节使用
 * 			 =0 -不执行跑模型数据操作
 * 			 =1 -执行跑模型数据（配合开启PID调节，则执行完本次PID调节后讲本次的初值进行跑模型数据）
 *			 =2 -如果已经有初值，存放到new_gamma_def.h的XXX模式_DEF_GAMMA_RGB_REG里，则直接执行跑数据不进行PID调节
 * RUN_IDLE: 该选项及其它相关模式选项选择开启调节的模式（需根据项目需要自行增减）
 *			 =1 -执行调试
 *			 =0 -不执行调试
 * dev: 	 该参数为打印串口对象
*/
int Gamma_Correction(int Pid_Net, int Get_Data, int RUN_IDLE,int RUN_HBM,int RUN_Normal,int RUN_NOR5,int RUN_NOR4,int RUN_NOR3,int RUN_NOR2,sky_comDriver *dev);


#endif
