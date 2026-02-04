/********************************* SKYCODE ************************************
* Copyright (c) 2015-2018, 深圳思凯测试技术有限公司
* All rights reserved.
*
* 文件名   ：auto_vcom.c    
* 描述     ：
*            1.自动查找最佳VCOM
*
* 版    本 ：V1.0.0
* 作    者 ：
* 完成日期 ：2018-4-20
* 修改描述 : 
*                                                                     
******************************************************************************/

#ifndef _AUTOVCOM_H_
#define _AUTOVCOM_H_

/*函数返回值*/
#define AUTOVCOM_OK    1  //完成
#define AUTOVCOM_ERROR -1 //错误
#define AUTOVCOM_EXIT  -2 //按键退出

/*
*函数名 : AutoVcomInit
*描述   : autovcom初始化
*参数   : 1. vcom_max - 全扫描上限VCOM
          2. vcom_min - 全扫描下限VCOM
          3. step - 全扫描步进阶数
          4. lcd_delay_ms - 每一步间隔时时间
          5. *pfunc_set_vcom - 指向设置vcom的函数, 
                               函数格式：参数- VCOM设定值
                                         返回值- 无
          
          6. *pfunc_get_flcker - 指向测量Flcker的函数
                                函数格式：参数- 指针，读到到flcker值，%
                                          返回值- 状态，<0读取失败
*返回   : 无
*/
void AutoVcomInit(void(*pfunc_set_vcom)(unsigned int vcom), int(*pfunc_get_flcker)(float*));


/*
*函数名 : AutoVcom_ScanRange
*描述   : 在规定的范围内 从小到大扫描，找到最佳VCOM。(是VCOM从小到底 不是闪烁从小到大)
*参数   : 1.vcom_min - 最小VCOM值，开始值
          2.vcom_max - 最大VCOM值，结束值
          3.step     - VCOM递增的阶数
          4.step_delay_ms - 每一步间隔时时间
          5.*vcom - 最终找到的最佳vcom值
*返回   : 1. AUTOVCOM_OK- 扫描完成，查找到最佳VCOM
          2. AUTOVCOM_ERROR - 出错，结束。
          3. AUTOVCOM_EXIT - 按键退出
*/
int AutoVcom_ScanRange(unsigned int vcom_min, unsigned int vcom_max, 
                       unsigned int step, unsigned int step_delay_ms,
                       unsigned int *vcom);

/*
*函数名 : AutoVcom_ScanDef
*描述   : 从默认值开始扫描,寻找最佳VCOM
*参数   : 1.vcom_def - VCOM默认值，以这个值为中心值寻找最佳VCOM
          2.step     - VCOM递增的阶数
          3.step_delay_ms - 每一步间隔时时间
          4.*vcom - 最终找到的最佳vcom值
*返回   : 1. AUTOVCOM_OK- 扫描完成，查找到最佳VCOM
          2. AUTOVCOM_ERROR - 出错，结束。
          3. AUTOVCOM_EXIT - 按键退出
*/
int AutoVcom_ScanDef(unsigned int vcom_def,
                       unsigned int step, unsigned int step_delay_ms,
                       unsigned int *vcom);

#endif

/********************************* SKYCODE ***********************************/
