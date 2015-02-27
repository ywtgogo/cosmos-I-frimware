#ifndef _dicor_h_
#define _dicor_h_
/************************************************************
* 
* Copyright (c) 2010  convertergy;
* All Rights Reserved
* FileName: dicor.h
* Version : 
* Date    : 
* Ower   : peter li
*
* Comments:
*   The main configuration file for dicor project. 
*   for this project, all definition in this file is globle
*
****************************************************************/

#include <mqx.h>
#include <bsp.h>
#include <lwevent.h>
#include <ipcfg.h>
#include <message.h>
#include <rtcs.h>
#include <ftpc.h>
#include <ftpd.h>

#if MQX_KERNEL_LOGGING
#include <klog.h>
#endif



#define DICOR_DEBUG  

#ifdef DICOR_DEBUG
	#define DEBUG_DIS(x)     x
#else
	#define DEBUG_DIS(x)
#endif

#define		WD_START_H		60*60*1000
#define 	WD_FEED_H		60*60*1000
#define 	WD_FEED_30M		30*60*1000
#define 	WD_FEED_10M		10*60*1000
#define		WD_FEED_M		   60*1000

#define     COODADDRDEFAULT                 0x01
#define 	SOFTVERSION						{3, 8, 6}			//软件版本号
//3.4.0改变：Bootloader升级DiDi。取得功率软件版本和通信软件版本
//3.4.1改变：将部分任务空间栈分配到外部RAM中
//3.4.2改变：增加支出8245 DiDi的升级,增加修改UID显示告警及修改格式提示。

//3.4.3改变：将部分分配到外部RAM的任务空间栈分配到内部RAM中,还原
//3.4.4改变：修复DiDi升级Bug,将网络地址写死改成可变,增加DiDi Bootloader版本号显示
//3.4.5改变：更改RF任务看门狗时间，解决升级DiDi看门狗重启问题，增加RF打印丢包log看门狗
//           在开机的时候延时15秒再启动第三方任务，防止内存不够引起wall失败
//           增加重启文件检查，如果发现文件，重启，可以用TFTP传文件重启，避免Telnet死掉无计可施
//           增加每天夜里4点06分会定时重启一次，防止DiCor永久死机
//3.4.6改变：启动过程中增加有部分任务创建失败重启，将串口SHELL任务栈减小0.5K，将ALIVE任务栈增大0.5K
//			 ALIVE任务每隔5分钟打印一个心跳LOG
//3.4.7改变  将LOST WATCHDOG时间改为20分钟
//3.4.8改变  将探测DiDi默认频率改为434.6M, 缩短升级DIDI超时时间
//3.4.9改变  增加8255的升级，和探测
//3.5.0改变  增加0x11的设备
//3.5.1改变  修复丢包统计代码BUG，使其支持DiDiD
//3.5.3改RF为485通讯
#define DICORCFG_ENABLE_TELNET_SERVER       1   /* enable telnet server */
#define DICORCFG_ENABLE_RF_NWK              1
#define DICORCFG_ENABLE_DATA_UP_LOAD        1
#define DICORCFG_ENABLE_CONNECT				1
#define DICORCFG_ENABLE_ALIVE				0 //0 3.12
#define DICORCFG_ENABLE_LOGGING				1
#define DICORCFG_ENABLE_WATCHDOG			1
#define DICORCFG_ENABLE_THIRD_DEVICE	    1


//可裁剪的任务
#define DICORCFG_ENABLE_SHELL				1
#define DICORCFG_ENABLE_LOSTLOGGING			1
#define DICORCFG_ENABLE_MFS_USB				0
#define DICORCFG_ENABLE_WEBSERVER           0   // we will use it for local access


#define DICORCFG_RTCS_POOL_ADDR  (uint_32)(BSP_EXTERNAL_MRAM_BASE)
#define DICORCFG_RTCS_POOL_SIZE  0x0000A000		//40K
#define DICORCFG_MFS_POOL_ADDR   (uint_32)(DICORCFG_RTCS_POOL_ADDR + DICORCFG_RTCS_POOL_SIZE)
#define DICORCFG_MFS_POOL_SIZE   0x00004000		//16K
#define DICORCFG_ENET_POOL_ADDR  (uint_32)(DICORCFG_MFS_POOL_ADDR + DICORCFG_MFS_POOL_SIZE)
#define DICORCFG_ENET_POOL_SIZE  0x00004000		//16K
#define DICORCFG_USER_POOL_ADDR  (uint_32)(DICORCFG_ENET_POOL_ADDR + DICORCFG_ENET_POOL_SIZE)
#define DICORCFG_USER_POOL_SIZE  0x00040000		//256K
//#define DEMOCFG_KLOG_ADDR       (uint_32)(DEMOCFG_MFS_POOL_ADDR + DEMOCFG_MFS_POOL_SIZE)
//#define DEMOCFG_KLOG_SIZE       4000
    

//#define DICORCFG_ENABLE_DNS                 0
//#define DICOR_IP_ADDRESS_DHCP               0
//#define DICORCFG_ENABLE_SNTP                0



#define DICOR_STATIC_REG_TAB                0//1 2015-01-07   

#define DICOR_ACK_RESEND                    1 


//#define DIDI_NUM_IN_SYSTEM             (2)


//#define DICOR_ENET_IPADDR               IPADDR(192,168,8,78) 
//#define DICOR_ENET_IPMASK               IPADDR(255,255,255,0) 
//#define DICOR_ENET_IPGATEWAY            IPADDR(192,168,8,1) 
//#define DICOR_ENET_IPDNS    	        IPADDR(202,96,209,5) 


#define  DICOR_ID_LEN                  (4)
extern IPCFG_IP_ADDRESS_DATA           dicor_ip_data;


typedef uint_8  DICOR_ID[DICOR_ID_LEN];

#if DICORCFG_ENABLE_WEBSERVER
   #include "httpd.h"
#endif

#include <shell.h>

enum 
{
   DICOR_UPLOAD_DATA_TASK = 1,
   DICOR_RF_NWK_TASK,

   DICOR_TELNET_SERVER_TASK,
   DICOR_WEB_SERVER_TASK,
   DICOR_CONNECT_TASK,
   DICOR_SHELL_TASK,
   DICOR_ALIVE_TASK,
   DICOR_LOGGING_TASK,
   DICOR_LOSTLOGGING_TASK,
   DICOR_MFS_USB_TASK,
   DICOR_WATCHDOG_TASK,
   DICOR_THIRD_DEVICE_TASK,
   DICOR_MAX
};


enum 
{
   WORKMODE_NORMOR = 0,
   WORKMODE_DEBUG,
   WORKMODE_DIHAVST4,
   WORKMODE_MAX
};


#if DICORCFG_ENABLE_MFS_USB
//MFS_USB
#define DICOR_MFS_USB_TASK_STACK_SIZE	(2560)	//2.5k
#define DICOR_MFS_USB_TASK_PRIO			(12)

extern void dicor_mfs_usb_task(uint_32 initial_data);
#endif

#if DICORCFG_ENABLE_THIRD_DEVICE
// 第三方设备轮询任务 
#define  DICOR_THIRD_DEVICE_TASK_STACK_SIZE    (2048)	//2k
#define  DICOR_THIRD_DEVICE_TASK_PRIO     		(13)

extern void  dicor_third_device_task(uint_32 initial_data);
#endif 

#if DICORCFG_ENABLE_SHELL
// shell 
#define  DICOR_SHELL_TASK_STACK_SIZE    (1536)	//1.5k//原2k
#define  DICOR_SHELL_TASK_PRIO     (18)

extern void  dicor_shell_task(uint_32 initial_data);
#endif 

#if DICORCFG_ENABLE_LOSTLOGGING
// lostlogging
#define  DICOR_LOSTLOGGING_TASK_STACK_SIZE    (2048) //2k
#define  DICOR_LOSTLOGGING_TASK_PRIO     (17)

extern void  dicor_lostlogging_task(uint_32 initial_data);
#endif 

#if DICORCFG_ENABLE_ALIVE
// alive
#define  DICOR_ALIVE_TASK_STACK_SIZE    (1024) //1k//原0.5k
#define  DICOR_ALIVE_TASK_PRIO     (16)

extern void  dicor_heartbeat_task(uint_32 initial_data);
#endif


#if DICORCFG_ENABLE_CONNECT
// connect
#define  DICOR_CONNECT_TASK_STACK_SIZE    (1024) //2k
#define  DICOR_CONNECT_TASK_PRIO     (14)

extern void  dicor_connect_task(uint_32 initial_data);
#endif 

#if  DICORCFG_ENABLE_TELNET_SERVER   

//telnet server  task
#define  DICOR_TELNET_SERVER_TASK_STACK_SIZE    (2560)	//2.5k
#define  DICOR_TELNET_SERVER_TASK_PRIO          (12)

extern void  dicor_telnet_server_task(uint_32 initial_data);
#endif 


#if DICORCFG_ENABLE_DATA_UP_LOAD
#define  DICOR_UPLOAD_DATA_TASK_STACK_SIZE    (3072)	//3k
#define  DICOR_UPLOAD_DATA_TASK_PRIO          (11)

extern void  dicor_upload_data_task(uint_32 initial_data);
#endif 

#if DICORCFG_ENABLE_RF_NWK
// PLC's LLC layer  task 
#define  DICOR_RF_NWK_TASK_STACK_SIZE    (2048)	//2k
#define  DICOR_RF_NWK_TASK_PRIO          (10)

extern void  dicor_rf_nwk_task(uint_32 initial_data);
#endif 

#if DICORCFG_ENABLE_LOGGING
// logging
#define  DICOR_LOGGING_TASK_STACK_SIZE    (2048) //2k
#define  DICOR_LOGGING_TASK_PRIO     (9)

extern void  dicor_logging_task(uint_32 initial_data);
#endif 

#if DICORCFG_ENABLE_WATCHDOG
// watchdog layer  task 
#define  DICOR_WATCHDOG_TASK_STACK_SIZE    (1024)	//1k
#define  DICOR_WATCHDOG_TASK_PRIO          (8)

extern void  dicor_watchdog_task(uint_32 initial_data);
#endif 


#if DICORCFG_ENABLE_WEBSERVER
// web server  task
#define  DICOR_WEB_SERVER_TASK_STACK_SIZE       (1024)
#define  DICOR_WEB_SERVER_TASK_PRIO             (8)

extern void  dicor_web_server_task(uint_32 initial_data);
#endif

/*
** MQX initialization information
*/
extern uint_32  dicor_InitializeNetworking(uint_32 pcbs, uint_32 msgs, 
                                             uint_32 sockets, boolean dhcp) ;
extern uint_32  dicor_GetTime(void) ;
extern void Wacthdog_Error(pointer td_ptr);

#endif
