#ifndef __driverTCPClient_H
#define __driverTCPClient_H

#include "stm32f4xx.h"
#include "delay.h"
#include "stdio.h"
#include "commonIOInterface.h"
#include "lwip/tcp.h"
#include "lwip_comm.h"

/*******************************************************************************
 * 函数名 : driverTCPClient_Init
 * 描  述 : TCP客户端驱动初始化
 * 参  数 : remoteIp[4]:远端ip
 * 参  数 : remotePort:远端端口
 * 参  数 : localPort:本地端口
 * 返  回 : sky_comDriver结构体指针句柄
            0:初始化失败
*******************************************************************************/
sky_comDriver *driverTCPClient_Init(uint8_t remoteIp[4], uint16_t remotePort, uint16_t localPort);

/*******************************************************************************
 * 函数名 : tcp_client_detection
 * 描  述 : TCP客户端检测TCP服务器是否有监听并连接上TCP服务器
 * 参  数 : remoteIp[4]:远端ip
 * 参  数 : remotePort:远端端口
 * 参  数 : localPort:本地端口
 * 返  回 : TCPState:TCP客户端连接状态
*******************************************************************************/
int tcp_client_detection(uint8_t remoteIp[4], uint16_t remotePort, uint16_t localPort);

#endif
