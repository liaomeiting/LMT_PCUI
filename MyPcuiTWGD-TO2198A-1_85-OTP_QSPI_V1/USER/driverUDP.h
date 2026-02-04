#ifndef __driverUDP_H
#define __driverUDP_H

#include "stdio.h"
#include "commonIOInterface.h"
#include "lwip/udp.h"

/*******************************************************************************
 * 函数名 : driverUDP_Init
 * 描  述 : UDP驱动初始化
 * 参  数 : remoteIp[4]:远端ip
 * 参  数 : remotePort:远端端口
 * 参  数 : localPort:本地端口
 * 返  回 : sky_comDriver结构体指针句柄
            0:初始化失败
*******************************************************************************/
sky_comDriver *driverUDP_Init(uint8_t remoteIp[4], uint16_t remotePort, uint16_t localPort);

#endif
