/**********************************************************************
* 
* Copyright (c) 2010 convertergy;
* All Rights Reserved
* FileName: dicor.c
* Version : 
* Date    : 
*
* Comments:
*
*   this file implements the feature about network part. it handle the share part that used by web server and 
*    telnet server, or shell command.
*
*************************************************************************/
#include <mqx.h>
#include <bsp.h>
#include <ipcfg.h>
#include <lwtimer.h>
#include <Watchdog.h>
#include <string.h>
#include "dicor.h"
#include "eeprom.h"
#include "led.h"
#include "dicor_upload.h"
#include "logging_public.h"
#include "dicor_upload.h"

extern UINT_8 	Upload_Alive;

const ENET_PARAM_STRUCT ENET_param[BSP_ENET_DEVICE_COUNT] = {
   {
      &ENET_0,
     Auto_Negotiate,
     0,

     BSPCFG_TX_RING_LEN,   // # tx ring entries
     BSPCFG_TX_RING_LEN,   // # large tx packets
     ENET_FRAMESIZE,       // tx packet size
     
     BSPCFG_RX_RING_LEN,   // # rx ring entries
     BSPCFG_RX_RING_LEN,   // # normal rx packets - must be >= rx ring entries
     ENET_FRAMESIZE,       // ENET_FRAMESIZE,   // rx packet size
     BSPCFG_RX_RING_LEN,   // # rx PCBs - should be >= large rx packets.
     
     0,     
     0     
   }
};


IPCFG_IP_ADDRESS_DATA dicor_ip_data;

//extern const DICOR_ID    dicor_uid ;

//
//#if (DEMOCFG_ENABLE_FTP_SERVER+DEMOCFG_ENABLE_TELNET_SERVER) != 1
//#warning Please enable one of the network services./
//#endif

const TASK_TEMPLATE_STRUCT MQX_template_list[] = 
{



#if DICORCFG_ENABLE_MFS_USB
// MFS_USB
	{ DICOR_MFS_USB_TASK,
	dicor_mfs_usb_task, DICOR_MFS_USB_TASK_STACK_SIZE,
	DICOR_MFS_USB_TASK_PRIO, "dicor_mfs_usb_task",
	  MQX_AUTO_START_TASK, 0L,
      0},
#endif

#if DICORCFG_ENABLE_SHELL
// shell     
      { DICOR_SHELL_TASK, 
   	dicor_shell_task, DICOR_SHELL_TASK_STACK_SIZE, 
   	DICOR_SHELL_TASK_PRIO, "dicor_shell_task",
      MQX_AUTO_START_TASK, 0L,
      0}, // time slice =0 ; 
#endif 

#if DICORCFG_ENABLE_CONNECT
// connect    
      { DICOR_CONNECT_TASK, 
   	dicor_connect_task, DICOR_CONNECT_TASK_STACK_SIZE, 
   	DICOR_CONNECT_TASK_PRIO, "dicor_connect_task",
      0, 0L,
      0}, // time slice =0 ; //原任务是被动,2014.11.19MQX_AUTO_START_TASK
#endif 

#if DICORCFG_ENABLE_WATCHDOG
// watchdog    
      { DICOR_WATCHDOG_TASK, 
   	dicor_watchdog_task, DICOR_WATCHDOG_TASK_STACK_SIZE, 
   	DICOR_WATCHDOG_TASK_PRIO, "dicor_watchdog_task",
      MQX_AUTO_START_TASK, 0L,
      0}, // time slice =0 ; 
#endif 

#if DICORCFG_ENABLE_THIRD_DEVICE
// third device     
      { DICOR_THIRD_DEVICE_TASK, 
   	dicor_third_device_task, DICOR_THIRD_DEVICE_TASK_STACK_SIZE, 
   	DICOR_THIRD_DEVICE_TASK_PRIO, "dicor_third_device_task",
      0, 0L,
      0}, // time slice =0 ; 
#endif 

#if DICORCFG_ENABLE_DATA_UP_LOAD
// upload data to data center task       
      { DICOR_UPLOAD_DATA_TASK, 
   	dicor_upload_data_task, DICOR_UPLOAD_DATA_TASK_STACK_SIZE, 
   	DICOR_UPLOAD_DATA_TASK_PRIO, "dicor_upload_data_task",
      MQX_AUTO_START_TASK, 0L,
      0}, // time slice =0 ; 
#endif 

#if     DICORCFG_ENABLE_RF_NWK
// PLC's llc layer task       
      { DICOR_RF_NWK_TASK, 
   	    dicor_rf_nwk_task, DICOR_RF_NWK_TASK_STACK_SIZE, 
   	    DICOR_RF_NWK_TASK_PRIO, "dicor_rf_nwk_task",
        MQX_AUTO_START_TASK, 0L,
     0}, // time slice =0 ; 
#endif 

      
#if  DICORCFG_ENABLE_TELNET_SERVER  
// telnet server task       
      { DICOR_TELNET_SERVER_TASK, 
   	dicor_telnet_server_task, DICOR_TELNET_SERVER_TASK_STACK_SIZE, 
   	DICOR_TELNET_SERVER_TASK_PRIO, "dicor_telnet_server_task",
      MQX_AUTO_START_TASK, 0L,
      0}, // time slice =0 ; 
#endif 

#if DICORCFG_ENABLE_ALIVE 
	// alive task    
	{DICOR_ALIVE_TASK,
	dicor_heartbeat_task,    DICOR_ALIVE_TASK_STACK_SIZE,   
	DICOR_ALIVE_TASK_PRIO,      "HeartBeat",                  
	0,    0,     
	0},
#endif

#if DICORCFG_ENABLE_LOGGING
	//logging task
    {DICOR_LOGGING_TASK, 
    dicor_logging_task,      DICOR_LOGGING_TASK_STACK_SIZE,   
    DICOR_LOGGING_TASK_PRIO,      "Logging",                    
    0,    0,
    0},
#endif

#if DICORCFG_ENABLE_LOSTLOGGING
	//lostlogging task
    {DICOR_LOSTLOGGING_TASK, 
    dicor_lostlogging_task,      DICOR_LOSTLOGGING_TASK_STACK_SIZE,   
    DICOR_LOSTLOGGING_TASK_PRIO,      "LostLogging",                    
    0,    0,
    0},
#endif

#if DICORCFG_ENABLE_WEBSERVER
// web server task       
      { DICOR_WEB_SERVER_TASK, 
   	    dicor_web_server_task, DICOR_WEB_SERVER_TASK_STACK_SIZE, DICOR_WEB_SERVER_TASK_PRIO, "dicor_web_server_task",
        MQX_AUTO_START_TASK, 0L,
        0}, // time slice =0 ; 

#endif

   { 0,  0,  0,   0, 0,  0,  0, 0}
};   


extern void Dicor_Reboot(void);
/*TASK*-----------------------------------------------------------------
*
* Function Name  : dicor_heartbeat_task
* Returned Value : void
* Comments       :
*
*END------------------------------------------------------------------*/

void dicor_heartbeat_task(uint_32 initial_data)
{ 
	// initialize IO before starting this task
	_mqx_int delay = 50;
	_mqx_int value = 0;
	uchar rfledflashtimes = 0;
	_mqx_int rfvalue = 0;
	uchar aliveledflashtimes = 0;
	DATE_STRUCT date;
	TIME_STRUCT newtime, oldtime;
	uint_8 times;
	
	//start watchdog 
	if (_watchdog_create_component(BSP_TIMER_INTERRUPT_VECTOR,Wacthdog_Error)!= MQX_OK)  
	{ 
	  printf("connect task create watchdog failed !");
	  //Dicor_Reboot();
	}

	_watchdog_start(60*1000*60);
	_time_get(&oldtime);
	times = 0;
	while (TRUE) 
	{
	    _time_delay(delay);
	    aliveledflashtimes++;
	    if (aliveledflashtimes > 10)
	    {
	    	aliveledflashtimes = 0;
	    	LedOutPut(DICOR_ALIVE_LED, value);
	    
	    	value ^= 1;  // toggle next value
	    }
	    
	    
	    if (RFLedNum > 0)
	    {
	    	rfledflashtimes++;
	    	LedOutPut(DICOR_RF_LED, rfvalue);
	    	rfvalue ^= 1;
	    	if (rfledflashtimes >= 8)
	    	{
	    		rfvalue = 0;
	    		LedOutPut(DICOR_RF_LED, rfvalue);
	    		rfledflashtimes = 0;
	    		RFLedNum = 0; 
	    	}
	    }
	    
	    dicor_get_logtime(&date);
	    if (date.HOUR==4 && date.MINUTE==6)
	    {
	    	_time_delay(10*1000);
	    	printf("date.HOUR==4 && date.MINUTE==6！\n");
	    	Dicor_Reboot();
	    }
	    
	    _time_get(&newtime);
	    //5分钟打印一个心跳包log
		if(newtime.SECONDS-oldtime.SECONDS > 60)
		{
			_time_get(&oldtime);
			times++;
			if (times == 10)
			{
				times = 0;
				sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t 心跳包打印\r\n", date.HOUR,date.MINUTE,date.SECOND);
				PrintLog(pDiCorLog->logbuf);
			}
			
			if (Upload_Alive != 1)
			{
				sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t Upload任务已死机，系统将重启\r\n", date.HOUR,date.MINUTE,date.SECOND);
				PrintLog(pDiCorLog->logbuf);
				_time_delay(1000*5);
		//	 printf("upload_not_alive！\n");
				Dicor_Reboot();   // by younger 9.26hao
			}
			Upload_Alive = 0;
				Dicor_Reboot();
				printf("one hour reboot！\n"); 
		}
	    
	    
	    _watchdog_start(60*1000*60);
	}
}


/* EOF */
