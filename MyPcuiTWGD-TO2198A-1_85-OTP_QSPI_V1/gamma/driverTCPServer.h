#ifndef __driverTCPServer_H
#define __driverTCPServer_H

#include "stdio.h"
#include "sys.h"
#include "commonIOInterface.h"
#include "lwip/tcp.h"
#include "lwip_comm.h"

/*******************************************************************************
 * 函数名 : driverTCPServer_Init
 * 描  述 : TCP驱动初始化
 * 参  数 : localPort:本地端口
 * 返  回 : sky_comDriver结构体指针句柄
            0:初始化失败
*******************************************************************************/
sky_comDriver *driverTCPServer_Init(uint16_t localPort);

/*******************************************************************************
 * 函数名 : tcp_server_mode_set
 * 描  述 : TCP服务器模式设置
 * 参  数 : mode:0(默认,服务器可以给其他客户端连接抢占),1(服务器不可以给其他客户端连接抢占)
 * 返  回 : 无
*******************************************************************************/
void tcp_server_mode_set(uint8_t mode);

#endif
