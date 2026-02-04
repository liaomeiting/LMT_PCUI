/********************************* SKYCODE ************************************
 * Copyright (c) 2019, 深圳思凯测试技术有限公司
 * All rights reserved.
 *
 * 文件名   ：Servo_com.c
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

#include "servo_com.h"
#include "communication.h"
#include "systick.h"
#include "Public_SoftFunpack.h"
#include "systick.h"
#include "Bus.h"
#include "delay.h"
#include <stdarg.h>
#include "string.h"

ServoId ServoUserId;

// static can_node canNodeServo[8];
static sky_comDriver candev[8];

static sky_MyByteReceiveDataTypeDef MAINRecServo[8];
static char MAINRecServodata[8][512];

static int BUSServo_SendData(struct _Tsky_comDriver *self, u8 *tx_data_buff, int tx_data_len);
static void BusREC_BusRxIrqHandler(void *bus_rx);
// extern  DataTypeDef myData;

int xxid[8];

int sky_runNFunServo(int group, sky_MyByteReceiveDataTypeDef *recType,
                     uint8_t cmd, uint8_t *data, uint32_t len, uint32_t time,
                     uint8_t *outData, uint32_t outOnelen)
{
    int i;
    uint8_t tem = 0;

    for (i = 0; i < 8; i++)
    {
        if (group & (1 << i))
        {
            ServoUserId.userId  = i;
            recType[i].saveFlag = 0;
            if (tem != 0)
            {
                sky_Delay_ms(recType, 20);  // 防止反馈时间冲突 此时间应根据反馈的数据量来判断
            }
            tem = 1;
            if (sky_sendData(&recType[i], cmd, data, len) < 0)
            {
                return -1;
            }
        }
    }

    if (sky_waitNData(group, recType, cmd, time) < 0)
    {
        return -2;
    }

    for (i = 0; i < 8; i++)
    {
        if (group & (1 << i))
        {
            // if (*((uint32_t *)(&recType[i].data[2])) != (outOnelen + 4))
            // {
            //     return -3;
            // }

            if (recType[i].saveFlag == 3)
            {
                if (*((uint32_t *)(&recType[i].data[2])) > outOnelen)
                {
                    return -3;
                }

                memcpy(outData + i * outOnelen, &(recType[i].data[6]), *((uint32_t *)(&recType[i].data[2])));
            }
        }
    }

    return 0;
}

int bspBusServo_Init(void)
{
    for (int i = 0; i < 8; i++)
    {
        memset(&candev[i], 0, sizeof(sky_comDriver));
        //        xxid[i]=i;
        candev[i].userData = (void *)i;  //(&xxid[i]);
        candev[i].write    = BUSServo_SendData;

        u8 ret;
        BusFilterStruct filter_struct;

        // 配置接收滤波
        filter_struct.src_type = BUS_SERVOMAIN;
        filter_struct.src_id   = i;
        filter_struct.dst_type = BUS_MAIN;
        filter_struct.dst_id   = 0;

        filter_struct.filt_src       = 1;  // 滤波 發送端的类型和ID
        filter_struct.filt_dst       = 1;
        filter_struct.rx_irq_handler = BusREC_BusRxIrqHandler;  // 接收中断处理函数

        ret = Bus_AddRxMdl(&filter_struct);  // 增加接收

        if (ret >= BUS_MAX_FILTER)
        {
            return -1;
        }
    }

    return 0;
}

static void BusREC_BusRxIrqHandler(void *bus_rx)
{
    //	unsigned int i;

    BusRxMsgStruct *bus_rx_msg;

    bus_rx_msg = (BusRxMsgStruct *)bus_rx;
    sky_comDriver *dev;

    dev = &candev[bus_rx_msg->src_id];

    if (dev->fifo != 0)
        myFIFOWrite(dev->fifo, bus_rx_msg->data, bus_rx_msg->dlc);

    if (dev->single != 0)
        dev->single(dev, bus_rx_msg->data, bus_rx_msg->dlc);
}

// extern ServoId ServoUserId;

static int BUSServo_SendData(struct _Tsky_comDriver *self, u8 *tx_data_buff, int tx_data_len)
{
    BusTxMsgStruct bus_tx_msg;
    unsigned int i;

    bus_tx_msg.dst_type = BUS_SERVOMAIN;
    bus_tx_msg.dst_id   = ServoUserId.userId;
    bus_tx_msg.src_type = BUS_MAIN;
    bus_tx_msg.src_id   = 0x00;
    bus_tx_msg.index    = 0;

    while (tx_data_len)
    {
        if (tx_data_len > 8)
        {
            bus_tx_msg.dlc = 8;
            for (i = 0; i < 8; i++)
            {
                bus_tx_msg.data[i] = *tx_data_buff;
                tx_data_buff++;
            }
            tx_data_len -= 8;
        }
        else
        {
            bus_tx_msg.dlc = tx_data_len;
            for (i = 0; i < tx_data_len; i++)
            {
                bus_tx_msg.data[i] = *tx_data_buff;
                tx_data_buff++;
            }

            tx_data_len = 0;
        }

        if (Bus_SendMsg(&bus_tx_msg) >= BUS_NO_MB)
        {
            return -1;
        }
    }
    return 0;
}

// static void BusREC_BusRxIrqHandler(void* bus_rx)
//{
//	unsigned int i;
//
//     BusRxMsgStruct* bus_rx_msg;
//
//     bus_rx_msg = (BusRxMsgStruct*)bus_rx;
//
//	for(i=0; i<bus_rx_msg->dlc; i++)
//	{
//		MAINRec.receivePact(&MAINRec,bus_rx_msg->data[i]);
//	}
// }

void BUSServo_Init(void)
{
    int i;
    //	bus_config bc;

    bspBusServo_Init();
    for (i = 0; i < 8; i++)
    {
        //		//顺序不可改变
        //		bc.src_type = BUS_ServoMAIN;
        //		bc.src_id = i;
        //		bc.filt_src  =1;
        //		bc.dst_type = BUS_MAIN;
        //		bc.dst_id = 0;
        //		bc.filt_dst  =1;
        //		P_CAN_SetNodeByBus(&canNodeServo[i], &bc);

        sky_ConmmunicationInit(&MAINRecServo[i], (uint8_t *)&MAINRecServodata[i], sizeof(MAINRecServodata[i]), 0xbb, 0xee, 0, MeasureTimeStart_ms);
        //		MAINRecServo[i].getTime = MeasureTimeStart_ms;  //获取当前系统时间ms数

        //

        sky_ConmBindDriver(&MAINRecServo[i], &candev[i]);
        //
        //		P_CAN_AddNode(adap, &canNodeServo[i]);
    }
}
/*********************************特有协议***********************************************/
static uint8_t data_buffer[256];

/*******************************************************************************
 * 函数名 : Servo_Scan
 * 描  述 : 扫描字符串格式(相当于sscanf函数)
 * 参  数 : *str1:传入要扫描的字符串
 * 参  数 : *str2:传入要取参或不取参格式字符串
 * 返  回 : >=0:成功
            -1:失败
*******************************************************************************/
int Servo_Scan(char *str1, char *str2, ...)
{
    va_list argptr;
    int cnt;
    int i   = 0;
    char *p = str2;

    while (1)
    {
        if (*p == '%')
        {
            i++;
        }
        else if (*p == 0)
        {
            break;
        }
        p++;
    }

    if (i != 0)
    {
        va_start(argptr, str2);
        cnt = vsscanf(str1, str2, argptr);
        va_end(argptr);
        if (i == cnt)
            return 0;
        else
            return -1;
    }
    else
    {
        if (strcmp(str1, str2) == 0)
            return 1;
        else
            return -1;
    }
}

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
int BUSServo_CmdPassthrough(int group, const uint8_t *Wdata, uint32_t Wdata_len, uint8_t *Rdata, uint32_t Rdata_len)
{
    uint32_t data_len;
    uint8_t cmd      = 0x62;
    uint32_t timeout = 200;

    data_len = sprintf((char *)data_buffer, "CmdTT:");
    memcpy(data_buffer + data_len, Wdata, Wdata_len);
    data_len += Wdata_len;

    return sky_runNFunServo(group, MAINRecServo, cmd, (uint8_t *)data_buffer, data_len, timeout, (uint8_t *)Rdata, Rdata_len);
}

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
int32_t BUSServo_ProgramVersion(int group, char *Rdata, uint32_t Rdata_len)
{
    uint32_t data_len;
    uint8_t cmd = 0x62;

    data_len = sprintf((char *)data_buffer, "ProgramVersion");

    return sky_runNFunServo(group, MAINRecServo, cmd, (uint8_t *)data_buffer, data_len, 200, (uint8_t *)Rdata, Rdata_len);
}

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
int32_t BUSServo_ESTOPStatus(int group, char *Rdata, uint32_t Rdata_len)
{
    uint32_t data_len;
    uint8_t cmd = 0x62;

    data_len = sprintf((char *)data_buffer, "ESTOPStatus");

    return sky_runNFunServo(group, MAINRecServo, cmd, (uint8_t *)data_buffer, data_len, 200, (uint8_t *)Rdata, Rdata_len);
}

/*
 *函数名: BUSServo_read_location
 *描述  : 获取保存的0度, 与90都得位置
 *参数  : group (Servo_CN1,Servo_CN2,Servo_CN4)
 *        location_0 (0 度数据)
 *        location_90 (90 度数据)
 *返回  : =0 成功，<0 失败
 */
int32_t BUSServo_read_location(int group, int32_t *location_0, int32_t *location_90)
{
    uint32_t data_len;
    uint8_t cmd = 0x62;
    uint8_t Rdata[32];
    uint32_t Rdata_len = 32;

    data_len = sprintf((char *)data_buffer, "read_location");

    if (sky_runNFunServo(group, MAINRecServo, cmd, (uint8_t *)data_buffer, data_len, 200, (uint8_t *)Rdata, Rdata_len) >= 0)
    {
        if (Servo_Scan((char *)Rdata, "read_location OK:%d %d", location_0, location_90) >= 0)
        {
            return 0;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }
}

/*
 *函数名: BUSServo_write_location
 *描述  : 写入0度, 与90都得位置
 *参数  : group (Servo_CN1,Servo_CN2,Servo_CN4)
 *        location_0 (0 度数据)
 *        location_90 (90 度数据)
 *返回  : =0 成功，<0 失败
 */
int32_t BUSServo_write_location(int group, int32_t location_0, int32_t location_90)
{
    uint32_t data_len;
    uint8_t cmd = 0x62;
    uint8_t Rdata[32];
    uint32_t Rdata_len = 32;

    data_len = sprintf((char *)data_buffer, "write_location:%d %d", location_0, location_90);

    if (sky_runNFunServo(group, MAINRecServo, cmd, (uint8_t *)data_buffer, data_len, 200, (uint8_t *)Rdata, Rdata_len) >= 0)
    {
        if (Servo_Scan((char *)Rdata, "write_location OK") >= 0)
        {
            return 0;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }
}

