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
#define 	SOFTVERSION						{3, 8, 6}			//����汾��
//3.4.0�ı䣺Bootloader����DiDi��ȡ�ù�������汾��ͨ������汾
//3.4.1�ı䣺����������ռ�ջ���䵽�ⲿRAM��
//3.4.2�ı䣺����֧��8245 DiDi������,�����޸�UID��ʾ�澯���޸ĸ�ʽ��ʾ��

//3.4.3�ı䣺�����ַ��䵽�ⲿRAM������ռ�ջ���䵽�ڲ�RAM��,��ԭ
//3.4.4�ı䣺�޸�DiDi����Bug,�������ַд���ĳɿɱ�,����DiDi Bootloader�汾����ʾ
//3.4.5�ı䣺����RF�����Ź�ʱ�䣬�������DiDi���Ź��������⣬����RF��ӡ����log���Ź�
//           �ڿ�����ʱ����ʱ15�����������������񣬷�ֹ�ڴ治������wallʧ��
//           ���������ļ���飬��������ļ���������������TFTP���ļ�����������Telnet�����޼ƿ�ʩ
//           ����ÿ��ҹ��4��06�ֻᶨʱ����һ�Σ���ֹDiCor��������
//3.4.6�ı䣺���������������в������񴴽�ʧ��������������SHELL����ջ��С0.5K����ALIVE����ջ����0.5K
//			 ALIVE����ÿ��5���Ӵ�ӡһ������LOG
//3.4.7�ı�  ��LOST WATCHDOGʱ���Ϊ20����
//3.4.8�ı�  ��̽��DiDiĬ��Ƶ�ʸ�Ϊ434.6M, ��������DIDI��ʱʱ��
//3.4.9�ı�  ����8255����������̽��
//3.5.0�ı�  ����0x11���豸
//3.5.1�ı�  �޸�����ͳ�ƴ���BUG��ʹ��֧��DiDiD
//3.5.3��RFΪ485ͨѶ
#define DICORCFG_ENABLE_TELNET_SERVER       1   /* enable telnet server */
#define DICORCFG_ENABLE_RF_NWK              1
#define DICORCFG_ENABLE_DATA_UP_LOAD        1
#define DICORCFG_ENABLE_CONNECT				1
#define DICORCFG_ENABLE_ALIVE				0 //0 3.12
#define DICORCFG_ENABLE_LOGGING				1
#define DICORCFG_ENABLE_WATCHDOG			1
#define DICORCFG_ENABLE_THIRD_DEVICE	    1


//�ɲü�������
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
// �������豸��ѯ���� 
#define  DICOR_THIRD_DEVICE_TASK_STACK_SIZE    (2048)	//2k
#define  DICOR_THIRD_DEVICE_TASK_PRIO     		(13)

extern void  dicor_third_device_task(uint_32 initial_data);
#endif 

#if DICORCFG_ENABLE_SHELL
// shell 
#define  DICOR_SHELL_TASK_STACK_SIZE    (1536)	//1.5k//ԭ2k
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
#define  DICOR_ALIVE_TASK_STACK_SIZE    (1024) //1k//ԭ0.5k
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
