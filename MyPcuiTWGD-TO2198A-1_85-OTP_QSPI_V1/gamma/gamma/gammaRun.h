/********************************* SKYCODE *************************************
*            
* 版    本 ：V1.0.0
* 作    者 ：
* 完成日期 ：2022-07-25
* 修改描述 ：1. 支持多组模式的gamma调节
*            2. 增加PID调节gamma并生成模型库
*            3. 通过宏定义DEBUG_GAMMA_INFO控制是否通过串口打印调试信息
*            4. 生成模型库通过串口打印及内存卡保存在为相应的模式.txt文件（关于文件的命名为NEW_GammaConfigStruct结构体中的displaymode字符串）
*            5. 可根据相应模式模型库使用新算法进行gamma调节
*            6. 通过gamma_version()获取软件版本号
*            
*            
* 版    本 ：V1.0.1
* 作    者 ：
* 完成日期 ：2022-08-03
* 修改描述 ：1. 修改PID接口可传入P系数
*
*            
* 版    本 ：V1.0.2
* 作    者 ：
* 完成日期 ：2022-08-10
* 修改描述 ：1. 增加GPU版本选择支持（在new_gamma_def.h中#define H10_GPU）（*作废，版本区分通过版本分开实现）
*            2. 修改SD写入模型文件方式，写入时默认SD卡为空文件
*            
*            
* 版    本 ：V1.0.3
* 作    者 ：
* 完成日期 ：2022-08-15
* 修改描述 ：1. 修复PID初始调值出现的黑屏现象
*            
*            
* 版    本 ：V1.0.4
* 作    者 ：
* 完成日期 ：2022-08-18
* 修改描述 ：1. 修复调gamma出现的边框亮线问题
*            2. 修复加载模型时SD卡写入不了值问题
*            3. 增加调试日志，通过宏定义开启，文件保存在SD卡
*            4. 增加复判功能接口，开启后每次调试完成全写以后调度每个绑点灰阶读取镜头值打印及保存SD卡
*
*            
* 版    本 ：V1.1.0
* 作    者 ：
* 完成日期 ：2022-09-23
* 修改描述 ：1. 修改保存SD卡日志文件名可修改，默认在宏定义修改，可通过Rename(char *)函数在程序运行过程中重新命名并创建对应的新的日志文件
*            2. 整理用户区与配置接口区的布局，用户自定义功能需要编写的代码函数存放在用户区域
*            3. 增加读镜头延时时间配置
*            4. 优化浮点型数据条件判断规则
*            5. 卡控为float类型数据修改时均需要带F结尾
*
*            
* 版    本 ：V1.2.0
* 作    者 ：
* 完成日期 ：2022-12-01
* 修改描述 ：1. 优化调试打印信息格式
*            2. 增加日志打印自定义格式接口
*            3. 完善相关注释
*            4. 规范接口命名格式
*
*            
* 版    本 ：V1.2.2
* 作    者 ：
* 完成日期 ：2023-02-02
* 修改描述 ：1. 修复出现越调越暗或越调越亮的问题  
*
*
*
*
*            
* 版    本 ：V1.2.2
* 作    者 ：
* 完成日期 ：2025-05-22
* 修改描述 ：1. 目标亮度，色坐标对应绑点分配  

*******************************************************************************/

#ifndef __GAMMARUN_H
#define	__GAMMARUN_H

#include "new_gamma_def.h"
#include "pcui.h"


/**
 * 函数名 : gamma_version
 * 描述   : gamma软件版本号
 * 参数   : 无
 * 例程	 ： _DEBUG("gamma version:%s\r\n",gamma_version());
 * 返回   : 字符串--eg.“V1.0.0 -2022.07.25”   
*/
char* gamma_version();


/**
 * 函数名 : Get_Model
 * 描述   : 跑gamma模型数据
 * 参数：
 * gammaConfig_t：  对应模式gamma结构体
 * Get_Data: 		该选项配合PID调节使用
 * 			 		=1 -执行跑模型数据（配合开启PID调节，则执行完本次PID调节后就本次的初值进行跑模型数据）
 *			 		=2 -如果已经有初值，存放到new_gamma_def.h的XXX模式_DEF_GAMMA_RGB_REG里，则直接执行跑数据不进行PID调节
 * dev: 	 		该参数为打印串口对象
 * 返回   : =1成功， =-1失败
*/
int Get_Model(NEW_GammaConfigStruct *gammaConfig_t, int Get_data, sky_comDriver *dev);


/**
 * 函数名 : Rename_LOG
 * 描述   : 重命名打印日志文件名
 * 参数：
 * name：  字符串指针
 * 返回   : void
*/
void Rename_LOG(char* name);


/**
 * 函数名 : run_gammaP
 * 描述   : gamma PID调试
 * 参数：
 * gammaConfig_t：  对应模式gamma结构体
 * P: 	 		    P系数
 * dev: 	 		该参数为打印串口对象
 * 返回   : =1成功， =-1错误
*/
int run_gammaP(NEW_GammaConfigStruct *gammaConfig, float P, sky_comDriver *dev);


/**
 * 函数名 : run_gammaN
 * 描述   : gamma Net调试
 * 参数   : 
 * gammaConfig_t：  对应模式gamma结构体
 * dev: 	 		该参数为打印串口对象
 * 返回   : =1成功， =-1错误
*/
int run_gammaN(NEW_GammaConfigStruct *gammaConfig,sky_comDriver *dev);
#endif
