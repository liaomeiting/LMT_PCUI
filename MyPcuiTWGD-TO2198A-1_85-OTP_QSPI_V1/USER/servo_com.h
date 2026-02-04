/********************************* SKYCODE ************************************
 * Copyright (c) 2019, 深圳思凯测试技术有限公司
 * All rights reserved.
 *
 * 文件名   ：Servo_com.h
 * 描述     ：舵机控制驱动程序
 *
 * 函数列表 ：
 *           void BUSServo_Init(void);
 *
 * 版本     ：V1.0.0
 * 作者     ：
 * 完成日期 ：2026-01-15
 * 修改描述 ：1.初版发布
 *
 ******************************************************************************/
#ifndef __SERVO_COM_H
#define __SERVO_COM_H

#include "stm32f4xx.h"
#include "sys.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "Public_SoftCan.h"

#ifndef BUS_SERVOMAIN
#define BUS_SERVOMAIN 32  // 舵机
#endif

#define Servo_ID0 1
#define Servo_ID1 2
#define Servo_ID2 4
#define Servo_ID3 8
#define Servo_ID4 16
#define Servo_ID5 32
#define Servo_ID6 64
#define Servo_ID7 128  // ID号指的拨码数

#define Servo_CN1 1
#define Servo_CN2 2
#define Servo_CN4 4  // 对应的是ID

/*************BL20********************/

typedef struct
{
    u8 flag;  // 等待数据采集为1  采集完成为0

} ServoTypeDef;

typedef struct
{
    u8 userId;  // 找出ID

} ServoId;

/*
 *函数名: BUSServo_Init
 *描述  : 初始化Bus通讯
 *
 *参数  : 无
 *返回  ; 无
 */
void BUSServo_Init(void);

/*******************************************************************************
 * 函数名 : Servo_Scan
 * 描  述 : 扫描字符串格式(相当于sscanf函数)
 * 参  数 : *str1:传入要扫描的字符串
 * 参  数 : *str2:传入要取参或不取参格式字符串
 * 返  回 : >=0:成功
            -1:失败
*******************************************************************************/
int Servo_Scan(char *str1, char *str2, ...);

/*
 *函数名: BUSServo_CmdPassthrough
 *描述  : 数据透传(指令转发)
 *参数  : group (Servo_CN1,Servo_CN2,Servo_CN4)
 *        Wdata (待发送数据)
 *        Wdata_len (待发送数据长度)
 *        Rdata (接收数据)
 *        Rdata_len (接收数据长度)
 *返回  : =0 成功，<0 失败
 */
int BUSServo_CmdPassthrough(int group, const uint8_t *Wdata, uint32_t Wdata_len, uint8_t *Rdata, uint32_t Rdata_len);

/*
 *函数名: BUSServo_ProgramVersion
 *描述  : 获取舵机控制器版本号
 *参数  : group (Servo_CN1,Servo_CN2,Servo_CN4)
 *        Wdata (待发送数据)
 *        Wdata_len (待发送数据长度)
 *        Rdata (接收数据)
 *        Rdata_len (接收数据长度)
 *返回  : =0 成功，<0 失败
 */
int32_t BUSServo_ProgramVersion(int group, char *Rdata, uint32_t Rdata_len);

/*
 *函数名: ESTOPStatus
 *描述  : 获取急停状态
 *参数  : group (Servo_CN1,Servo_CN2,Servo_CN4)
 *        Wdata (待发送数据)
 *        Wdata_len (待发送数据长度)
 *        Rdata (接收数据)
 *        Rdata_len (接收数据长度)
 *返回  : =0 成功，<0 失败
 */
int32_t BUSServo_ESTOPStatus(int group, char *Rdata, uint32_t Rdata_len);

/*
 *函数名: BUSServo_read_location
 *描述  : 获取保存的0度, 与90都得位置
 *参数  : group (Servo_CN1,Servo_CN2,Servo_CN4)
 *        location_0 (0 度数据)
 *        location_90 (90 度数据)
 *返回  : =0 成功，<0 失败
 */
int32_t BUSServo_read_location(int group, int32_t *location_0, int32_t *location_90);

/*
 *函数名: BUSServo_write_location
 *描述  : 写入0度, 与90都得位置
 *参数  : group (Servo_CN1,Servo_CN2,Servo_CN4)
 *        location_0 (0 度数据)
 *        location_90 (90 度数据)
 *返回  : =0 成功，<0 失败
 */
int32_t BUSServo_write_location(int group, int32_t location_0, int32_t location_90);

#endif
