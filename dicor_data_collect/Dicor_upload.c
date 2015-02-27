/*************************************************************
* 
* Copyright (c) convertergy ;
* All Rights Reserved
* FileName: dicor_upload.c
* Version : 
* Date    : 
* ower: peter
*
* Comments:*
*
*
***************************************************************/

#include <mqx.h>
#include <bsp.h>
#include <rtcs.h>
#include <string.h>
#include <lwtimer.h>
#include <mfs.h>
#include <Watchdog.h>
#include "dicor.h"
#include "dicor_upload.h"
#include "shell.h"
#include "dicor_update.h"
#include  "thirddev_gateway.h"
#include "breakpoint.h"
#include "eeprom.h"
#include "led.h"
#include  "modbus.h"
#include "dicor_sd.h"
#include "pmu.h"
#include "rs485.h"
#include "modbus.h"
#include "logging_public.h"
#include "lostlogging_public.h"
#include "tftp.h"
#include "third_device.h"
#include  "upgradedidi.h"
#include  "My_Shell_Commands.h"
#include "utility.h"
#include <stdlib.h>

#include  "modbus.h"
/*-----------------------------------------------------
* 
* Task Name    : dicor_upload_data_task   
* Comments     :
*    This task is responsible for uploading all data from MG to convertergy data center.
*     
*
*-----------------------------------------------------*/
#define TFTPHOSTIPADDR2			IPADDR(115,29,192,154)
//#define TFTPHOSTIPADDR2		IPADDR(baseconfig->ip[0],baseconfig->ip[1],baseconfig->ip[2],baseconfig->ip[3])
_mem_pool_id _user_pool_id;
//DICOR_LOG DiCorLog;
EMN_DICOR_DATA_ST DiCorRunStatus;

DIDI_DATA_BUFFER upload_buffer;
DIDI_DATA_BUFFER upload_buffer_mux;

LWEVENT_STRUCT RF_NET_event;

volatile UINT_8 Upload_Alive = 0;

UINT_8 FirstPackage = 0;

UINT_8	SERVER_MODE;
extern uchar CoodAddr;

//第三方设备相关变量
//LWSEM_STRUCT ThirdDevDataSem;
//uint_8 ThirdDevDataFlag;
extern THIRD_DEVICE_DATA_ST* g_pThirdDeviceData;
extern void EepromReadCoodaddr(void);

static MCF5225_GPIO_STRUCT_PTR led_gpio_ptr;
static TIME_STRUCT SpaceTime;

//timer for update display
//atic LWTIMER_PERIOD_STRUCT    dicor_timer_queue;
//atic LWTIMER_STRUCT_PTR       dis_timer;
UINT_8 NoRegTimes;
UINT_8 NoRfTimes;
UINT_8 watchdogenable = 0;
static UINT_8 breakpointstart = 0;

//static DIMO_RES_DATA Dimo_response_data;
DIMO_RES_DATA Dimo_response_data;
DIMO_HB_DATA  Dimo_hb_data;  
DIMO_HT_DATA  Dimo_ht_data; 
DIMO_HT_DATA  Cosmos_to_tele;
RF_NWK_STATE rfnwk_state;
void Wacthdog_Error(pointer td_ptr);

void RF_SetupNetwork(void);
uint_32 dicor_connect_datacenter(void);
uint_32 dicor_connect_datacentermux(void);
void RF_Waiter3G(void);

static UINT_32 dicor_process_DiMo_packet(uint_32 sock);
static void DiCor_SendIntfaceVersion(void);
void dicor_get_rtctime(DATE_STRUCT * date);
static void test_closetime(void);
void upload_data_exit(void);
void dicor_check_connect(void);
extern MQX_FILE_PTR filesystem_handle;


void dicor_dis_timer(void);
void dicor_get_timer(void);
void dicor_get_time(DATE_STRUCT * date);
void UpLoadbuffer_init(void);
void dicor_close_socket(void);

#define UART2_CHANNEL "ttyc:"

//MQX_FILE_PTR uart2_dev = NULL;
//MQX_FILE_PTR uart2_dev;
MQX_FILE_PTR uart1_dev;

extern void dicor_get_logtime(DATE_STRUCT * date);

static void PrintInfo(void)
{
	printf("\n================INFO===================\n");
	printf("  Softwave Version: %c%d.%d.%d\n", pBaseConfig->softversion[0],pBaseConfig->softversion[1],pBaseConfig->softversion[2],pBaseConfig->softversion[3]);
	printf("  Hardwave Version: %c%d.%d.%d\n", pBaseConfig->hardversion[0],pBaseConfig->hardversion[1],pBaseConfig->hardversion[2],pBaseConfig->hardversion[3]);
	printf("  build time: %s %s\n", __DATE__,__TIME__);
	printf("  UID: %02X-%02X-%02X-%02X\n", pBaseConfig->uid[0],pBaseConfig->uid[1],pBaseConfig->uid[2],pBaseConfig->uid[3]);
	printf("  NETADDR: 0x%02X\n", pBaseConfig->net_addr);
	printf("=======================================\n");
}

static void Testsram(void)
{


	uchar *p;
	uchar test;
	UINT_32 i;
	printf("\nSRAM Test...\n");

	printf("size=%dk\n", BSP_EXTERNAL_MRAM_SIZE/1024);
	
	p = (uchar*) BSP_EXTERNAL_MRAM_BASE;
/*	*p = 10;
	printf("p=%d\n", *p);
	
	*(p+BSP_EXTERNAL_MRAM_SIZE/2) = 20;
	
	printf("p+size/2=%d\n", *(p+BSP_EXTERNAL_MRAM_SIZE/2));
	
	*(p+BSP_EXTERNAL_MRAM_SIZE-1) = 30;
	printf("p+size-1=%d\n", *(p+BSP_EXTERNAL_MRAM_SIZE-1));*/
	
	for (i=0; i<BSP_EXTERNAL_MRAM_SIZE; i++)
	{
		test = i % 256;
		*(p+i) = test;
		if (*(p+i) != test)
		{
			printf("error when write or read addr[%08X]", i);
			break;
		}
	}
	if (i==BSP_EXTERNAL_MRAM_SIZE)
	{
		printf("SRAM Ok!\n");
	}
	for (i=0; i<BSP_EXTERNAL_MRAM_SIZE; i++)
	{
		*(p+i) = 0;		
	}
	
}

void UpLoadbuffer_init(void)
{
	MUTEX_ATTR_STRUCT mutexattr;
	uint_8 i=0;
	printf("\n \n********************************\n");
	printf("********************************");
	printf("\n \n       Starting System \n");
	
	Testsram();

	/* Initialize mutex attributes */
	if (_mutatr_init(&mutexattr) != MQX_OK) {
		printf("Initialize mutex attributes failed.\n");
		_mqx_exit(0);
	}
	/* Initialize the mutex */
	if (_mutex_init(&upload_buffer.mutex, &mutexattr) != MQX_OK) {
		printf("Initialize mutex failed.\n");
		_mqx_exit(0);
	}
	
	_user_pool_id = _mem_create_pool((pointer)DICORCFG_USER_POOL_ADDR, DICORCFG_USER_POOL_SIZE);
	if (_user_pool_id == NULL)
	{
		printf("error mem create pool!\r\n");
	}
	
	EepromInit();
	PrintInfo();
	LedInit(); // 3.12 test

	printf("\npull up cc2530\n");
	upload_buffer.write_index = 0;
	upload_buffer.state = CAN_WRITE;
	upload_buffer.eth_st = ETH_NO_INIT, rfnwk_state = RF_NWK_REGING;
	NoRegTimes = 0;
	NoRfTimes = 0;
	
	upload_buffer.data0.dicor_id[0] = pBaseConfig->uid[0];
	upload_buffer.data0.dicor_id[1] = pBaseConfig->uid[1];
	upload_buffer.data0.dicor_id[2] = pBaseConfig->uid[2];
	upload_buffer.data0.dicor_id[3] = pBaseConfig->uid[3];

	DiCorRunStatus.type = 0x40;
	DiCorRunStatus.len = 6;
	DiCorRunStatus.subnet = 0;
	DiCorRunStatus.dev = 0;
	
	dicor_ip_data.ip = 0x00000000L;
	_time_get(&SpaceTime);	//用于记录RF收到数据的时间间隔


	if (_lwevent_create(&RF_NET_event, 0) != MQX_OK) {
		DEBUG_DIS(printf("\nMake event failed"));
		_mqx_exit(0);
	}
	
}

void dicor_waite_rf(uint_8 flag, uint_32 timeout)
{
	if (RFNET_EVENT == flag) {
		_lwevent_wait_ticks(&RF_NET_event, RFNET_EVENT, TRUE,
				    timeout);
		_lwevent_clear(&RF_NET_event, RFNET_EVENT);
	} else if (NET_SEND_DATA_END == flag) {
		_lwevent_wait_ticks(&RF_NET_event, NET_SEND_DATA_END,
				    TRUE, timeout);
		_lwevent_clear(&RF_NET_event, NET_SEND_DATA_END);
	} else if (RF_GET_DATA_END == flag) {
		_lwevent_wait_ticks(&RF_NET_event, RF_GET_DATA_END, TRUE,
				    timeout);
		_lwevent_clear(&RF_NET_event, RF_GET_DATA_END);
	} else if (RF_REG_START == flag) {
		_lwevent_wait_ticks(&RF_NET_event, RF_REG_START, TRUE,
				    timeout);
		_lwevent_clear(&RF_NET_event, RF_REG_START);
	} else if (RF_REG_END == flag) {
		_lwevent_wait_ticks(&RF_NET_event, RF_REG_END, TRUE,
				    timeout);
		_lwevent_clear(&RF_NET_event, RF_REG_END);
	}
	else if (THIRD_DEVICE_DATA_END == flag)
	{
		_lwevent_wait_ticks(&RF_NET_event, THIRD_DEVICE_DATA_END, TRUE,
				    timeout);
		_lwevent_clear(&RF_NET_event, THIRD_DEVICE_DATA_END);
	}

}

void dicor_rf_signal(uint_32 flag)
{
//      _lwevent_set(&RF_NET_event,flag);

#if 1
	if (_lwevent_set(&RF_NET_event, flag) != MQX_OK) {
		DEBUG_DIS(printf("\net Event failed"));
		_mqx_exit(0);
	}
#endif
}



void  Downloadconfigfile(int_32 argc, char_ptr argv[])
{   

	boolean  print_usage, shorthelp = FALSE;
	int_32   return_code = SHELL_EXIT_SUCCESS;
	char filename[100]; 
	char DiDiFileName[100] = 
{
	'\0'
};
	filename[99] = '\0';
	print_usage = Shell_check_help_request(argc, argv, &shorthelp );
	if (!print_usage)  
	{
		if (argc == 2)  
		{	
			strcpy(filename, "d:\\dicorconfig\\");
			strcat(filename, argv[1]);
			if (Dicor_TFTP_client(TFTPHOSTIPADDR2, argv[1], filename) == 0)
			
			{
				printf("downloade file succeed\n");
				strcpy(DiDiFileName, filename);
			}
		} 
		else  
		{
			printf("Error, %s invoked with incorrect number of arguments\n", argv[0]);
			print_usage = TRUE;
		}
	}

	//return return_code;
}
/*---------------------------------------------------------------
Dicor’s UID(4bytes)+ package type+ status
1) 链接建立后，每隔5分钟就发送一个
           type =0x01,status=0x00,0x00的状态包 
2) 如果系统出现异常立即发送一个type =0x01的状态包,
3) 状态域的定义：
0x00,0x00：everything are OK
0x00,0x01:   Dimo is in truble (包括数据库错误,java应用异常如nullpoint)
0x00,0x02:   register table is wrong
0x00,0x03:   设备网络地址错。就是序列号没有通过后台加入。
0x00,0x04:  datapackage from dicor is wrong

---------------------------------------------------------------*/
static UINT_32 dicor_process_DiMo_packet(uint_32 sock)
{
	int_32 length;
	static int_32 mux_error = 0;
	DATE_STRUCT date;
	uint_8 UpdateChkComplete = 0;
	uint_32 error;
	_task_id taskid;
//	uint_16 i;
	_mem_zero(Dimo_response_data.Data_Buffer, sizeof(Dimo_response_data.Data_Buffer));
	length = recv(sock, (char_ptr) (&Dimo_response_data.Dicor_UID[0]), sizeof(DIMO_RES_DATA), 0);

		if (length == RTCS_ERROR) {
			if (sock == upload_buffer.sock_mux)
			{
#if	1						
				if (mux_error++ >1000)
				{
					if (SERVER_MODE == 2)
						dicor_connect_datacentermux();	
						mux_error = 0;
				}
			//	printf("\n___________________________\n");
#endif								
				return ;	
			}
			
			shutdown(sock, FLAG_ABORT_CONNECTION);
			DEBUG_DIS(printf("\n(DiMo receive error 0x%lx)", RTCS_geterror(sock)));
			upload_buffer.eth_st = ETH_INIT;
			dicor_get_logtime(&date);
			sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t 从DiMo接收数据出错，recv出错，错误代码:%08X\r\n", date.HOUR,date.MINUTE,date.SECOND,RTCS_geterror(sock));
			PrintLog(pDiCorLog->logbuf);
			return ;    
	} else {
		
		if (length == 0)       
			return;
		DEBUG_DIS(printf("\n"));
		dicor_dis_timer();
		DEBUG_DIS(printf("\n*********************************************************"));
		DEBUG_DIS(printf("\n%d bytes received from teletraan", length));
        DEBUG_DIS(printf("\nDicor_UID:%02x ", Dimo_response_data.Dicor_UID[0]));
        DEBUG_DIS(printf("%02x ", Dimo_response_data.Dicor_UID[1]));
        DEBUG_DIS(printf("%02x ", Dimo_response_data.Dicor_UID[2]));
        DEBUG_DIS(printf("%02x ", Dimo_response_data.Dicor_UID[3]));
        DEBUG_DIS(printf("\nobj_type_role:%02x ", Dimo_response_data.pack_obj));
        DEBUG_DIS(printf("%02x ", Dimo_response_data.pack_type));
        DEBUG_DIS(printf("%02x ", Dimo_response_data.role));
        
//        DEBUG_DIS(printf("\nData_Buffer:\n"));
//        for (i=0; i<length; i++)
//        {
//	       	DEBUG_DIS(printf("%02x ", Dimo_response_data.Data_Buffer[i])); 
//        }
   	}

	// check UID
	if ((Dimo_response_data.Dicor_UID[0] != pBaseConfig->uid[0]) ||
	    (Dimo_response_data.Dicor_UID[1] != pBaseConfig->uid[1]) ||
	    (Dimo_response_data.Dicor_UID[2] != pBaseConfig->uid[2]) ||
	    (Dimo_response_data.Dicor_UID[3] != pBaseConfig->uid[3])
	     //||(Dimo_response_data.pack_type != 0x0001)
	     ) {
		DEBUG_DIS(printf("\n Teletraan packet error"));
	} else {

		/* Determine and handle DiMo request */
		//  switch(ntohs(Dimo_response_data.status)) 
		switch (Dimo_response_data.pack_obj) {
		case 0x02:
				
			DEBUG_DIS(printf("\n Cosmos's packet "));
			
			switch(Dimo_response_data.pack_type)			
			{
				case 0x00:
				    DEBUG_DIS(printf("\n receive heartbeat back"));
				    DEBUG_DIS(printf("\n"));
				    DEBUG_DIS(printf("\n"));
				    break;
			   
				case 0x02:
					dicor_kill_task("dicor_third_device_task");
					DEBUG_DIS(printf("\n download baseconfig.txt"));
					_time_delay(2000);
					{	            	
						char filename[100]; 
						char DiDiFileName[100] = 
						{
						'\0'
						};
						filename[99] = '\0';
						strcpy(filename, "d:\\dicorconfig\\");
						strcat(filename,"baseconfig.txt");
						if (Dicor_TFTP_client(IPADDR(pBaseConfig->datacenter[0],pBaseConfig->datacenter[1],pBaseConfig->datacenter[2],pBaseConfig->datacenter[3]), "baseconfig.txt", filename) == 0)
						{
							printf("downloade baseconfig.txt file succeed\n");
							strcpy(DiDiFileName, filename);
							Dimo_ht_data.state=0x00;
						}
						else
						{
							printf("downloade baseconfig.txt file failed\n");
							strcpy(DiDiFileName, filename);
							Dimo_ht_data.state=0x01;							
						}
					}
					Dimo_ht_data.Dicor_UID[0]= Dimo_response_data.Dicor_UID[0];
					Dimo_ht_data.Dicor_UID[1]= Dimo_response_data.Dicor_UID[1];
					Dimo_ht_data.Dicor_UID[2]= Dimo_response_data.Dicor_UID[2];
					Dimo_ht_data.Dicor_UID[3]= Dimo_response_data.Dicor_UID[3];
					Dimo_ht_data.pack_obj=Dimo_response_data.pack_obj;
					Dimo_ht_data.pack_type=Dimo_response_data.pack_type;
					if (SERVER_MODE == 2)
						error = send(upload_buffer.sock_mux,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]), sizeof(DIMO_HT_DATA), 0);    	         
					error = send(upload_buffer.sock,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]), sizeof(DIMO_HT_DATA), 0);	            	            
					break;
	    
			    case 0x0a:	
					DEBUG_DIS(printf("\n download thirddevicetable.txt"));
					dicor_kill_task("dicor_third_device_task");
					_time_delay(1000);
					{
						char filename[100]; 
						char DiDiFileName[100] = {'\0'};
						filename[99] = '\0';
						strcpy(filename, "d:\\dicorconfig\\");
						strcat(filename,"thirddevicetable.txt");
						if (Dicor_TFTP_client(IPADDR(pBaseConfig->datacenter[0],pBaseConfig->datacenter[1],pBaseConfig->datacenter[2],pBaseConfig->datacenter[3]), "thirddevicetable.txt", filename) == 0)
						{
							printf("downloade thirddevicetable.txt file succeed\n");
							strcpy(DiDiFileName, filename);
							Dimo_ht_data.state=0x00;
						}
						else
						{
							printf("downloade thirddevicetable.txt file failed\n");
							strcpy(DiDiFileName, filename);
							Dimo_ht_data.state=0x01;							
						}
					}
					Dimo_ht_data.Dicor_UID[0]= Dimo_response_data.Dicor_UID[0];
					Dimo_ht_data.Dicor_UID[1]= Dimo_response_data.Dicor_UID[1];
					Dimo_ht_data.Dicor_UID[2]= Dimo_response_data.Dicor_UID[2];
					Dimo_ht_data.Dicor_UID[3]= Dimo_response_data.Dicor_UID[3];
					Dimo_ht_data.pack_obj=Dimo_response_data.pack_obj;
					Dimo_ht_data.pack_type=Dimo_response_data.pack_type;					
					if (SERVER_MODE == 2)
						error = send(upload_buffer.sock_mux,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]), sizeof(DIMO_HT_DATA), 0);    
					error = send(upload_buffer.sock,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]), sizeof(DIMO_HT_DATA), 0);
			    	break;			    
			     
				case 0x30:	
			   		DEBUG_DIS(printf("\n download magnus Code"));			   
					{
						char filename[100]; 
						char DiDiFileName[100] = {'\0'};
		            	filename[99] = '\0';
			            strcpy(filename, "d:\\didiprogram\\");
				    	strcat(filename,"magnus.hex");
				    	if (Dicor_TFTP_client(IPADDR(pBaseConfig->datacenter[0],pBaseConfig->datacenter[1],pBaseConfig->datacenter[2],pBaseConfig->datacenter[3]), "magnus.hex", filename) == 0)
					    {
							printf("downloade magnus.hex file succeed\n");
							strcpy(DiDiFileName, filename);
							Dimo_ht_data.state=0x00;
					    }
						else
						{
							printf("downloade magnus.hex file failed\n");
							strcpy(DiDiFileName, filename);
							Dimo_ht_data.state=0x01;							
						}				    
					}				   
					Dimo_ht_data.Dicor_UID[0]= Dimo_response_data.Dicor_UID[0];
					Dimo_ht_data.Dicor_UID[1]= Dimo_response_data.Dicor_UID[1];
					Dimo_ht_data.Dicor_UID[2]= Dimo_response_data.Dicor_UID[2];
					Dimo_ht_data.Dicor_UID[3]= Dimo_response_data.Dicor_UID[3];
					Dimo_ht_data.pack_obj=Dimo_response_data.pack_obj;
					Dimo_ht_data.pack_type=Dimo_response_data.pack_type;		           
		            if (SERVER_MODE == 2)
		           		error = send(upload_buffer.sock_mux,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]), sizeof(DIMO_HT_DATA), 0);
		            error = send(upload_buffer.sock,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]), sizeof(DIMO_HT_DATA), 0); 		            
				    break;
/*				case 0x20:	
			   		DEBUG_DIS(printf("\n download optimus Code"));			   
					{
						char filename[100]; 
						char DiDiFileName[100] = {'\0'};
		            	filename[99] = '\0';
			            strcpy(filename, "d:\\didiprogram\\");
				    	strcat(filename,"magnus.hex");//optimus
				    	if (Dicor_TFTP_client(IPADDR(pBaseConfig->datacenter[0],pBaseConfig->datacenter[1],pBaseConfig->datacenter[2],pBaseConfig->datacenter[3]), "optimus.hex", filename) == 0)
					    {
							printf("downloade optimus.hex file succeed\n");
							strcpy(DiDiFileName, filename);
							Dimo_ht_data.state=0x00;
					    }
						else
						{
							printf("downloade optimus.hex file failed\n");
							strcpy(DiDiFileName, filename);
							Dimo_ht_data.state=0x01;							
						}				    
					}				   
					Dimo_ht_data.Dicor_UID[0]= Dimo_response_data.Dicor_UID[0];
					Dimo_ht_data.Dicor_UID[1]= Dimo_response_data.Dicor_UID[1];
					Dimo_ht_data.Dicor_UID[2]= Dimo_response_data.Dicor_UID[2];
					Dimo_ht_data.Dicor_UID[3]= Dimo_response_data.Dicor_UID[3];
					Dimo_ht_data.pack_obj=Dimo_response_data.pack_obj;
					Dimo_ht_data.pack_type=Dimo_response_data.pack_type;		           
		            if (SERVER_MODE == 2)
		           		error = send(upload_buffer.sock_mux,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]), sizeof(DIMO_HT_DATA), 0);
		            error = send(upload_buffer.sock,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]), sizeof(DIMO_HT_DATA), 0); 		            
				    break;
*/
			    case 0x05:	
			   DEBUG_DIS(printf("\n download Cosmos Code.S19\n"));
			   {

			   Dimo_ht_data.Dicor_UID[0]= Dimo_response_data.Dicor_UID[0];
	           Dimo_ht_data.Dicor_UID[1]= Dimo_response_data.Dicor_UID[1];
	           Dimo_ht_data.Dicor_UID[2]= Dimo_response_data.Dicor_UID[2];
	           Dimo_ht_data.Dicor_UID[3]= Dimo_response_data.Dicor_UID[3];
	           Dimo_ht_data.pack_obj=Dimo_response_data.pack_obj;
	           Dimo_ht_data.pack_type=Dimo_response_data.pack_type;
	           Dimo_ht_data.state=0x00;
			if (SERVER_MODE == 2)
				error = send(upload_buffer.sock_mux,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]),sizeof(DIMO_HT_DATA), 0); 
	         error = send(upload_buffer.sock,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]),sizeof(DIMO_HT_DATA), 0); 
		       Dicor_Update(); 

			   }
		
			    break;


			case 0x07:	
				DEBUG_DIS(printf("\n wall the file "));
				EepromInit();
				{
				//	boolean print_usage;
					boolean shorthelp = FALSE;
					int_32 return_code = SHELL_EXIT_SUCCESS;
					char_ptr argv1[] = {"wbaseconfig","wregtable","wroutertable","wdidi_diantable"};
					{
						Shell_wbaseconfig(1, argv1);
						Shell_wregtable(1, argv1+1);
						_time_delay(2000);
					}
			  	}			  
				Dimo_ht_data.Dicor_UID[0]= Dimo_response_data.Dicor_UID[0];
				Dimo_ht_data.Dicor_UID[1]= Dimo_response_data.Dicor_UID[1];
				Dimo_ht_data.Dicor_UID[2]= Dimo_response_data.Dicor_UID[2];
				Dimo_ht_data.Dicor_UID[3]= Dimo_response_data.Dicor_UID[3];
				Dimo_ht_data.pack_obj=Dimo_response_data.pack_obj;
				Dimo_ht_data.pack_type=Dimo_response_data.pack_type;
				Dimo_ht_data.state=0x00;
				if (SERVER_MODE == 2)
					error = send(upload_buffer.sock_mux,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]), sizeof(DIMO_HT_DATA), 0);    	         
				error = send(upload_buffer.sock,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]), sizeof(DIMO_HT_DATA), 0);		            
			  	Dicor_Reboot();
			    break;			    			    			    
			case 0x08:	
				DEBUG_DIS(printf("\n regtable Fback to Cosmos\n "));
				break;
			default:
				break;			    
			}
			
			
		   _watchdog_start(10*60000);     
			break;
	
	
		case 0x01:
			DEBUG_DIS(printf("\n Magnus's packet"));
				switch(Dimo_response_data.pack_type)
				{
			

				case 0x31:
				 DEBUG_DIS(printf("\n begin to  magnus-bootloader"));
				 
				 //Dimo_response_data.role = 0x01;
		
			 
			   	dicor_kill_task("dicor_third_device_task");
			   	//	 _watchdog_start(10*60000);
			   {
			   
			  char MagnusFileName[100] = 
                            {
                           	'\0'
                            };
	boolean  print_usage, shorthelp = TRUE;
	int_32   return_code = SHELL_EXIT_SUCCESS;
	int_32 argc=3; 
	char_ptr argv[4];
	uint_16 addr_f,addr_r;
 	char hex[3]; 
	unsigned char *didiflash;
    EEPROMDATA_PTR p;
    uint_8* data_buffer;
	data_buffer = Dimo_response_data.Data_Buffer;	 
	p = (EEPROMDATA_PTR) base_addr;
   	hex[2] = '\0';


		if (argc == 3)  
		{	
	
			printf("magnus addr_f=%04X addr_r=%04X", addr_f, addr_r);

			
			StopGetData();
	
			
			strcpy(MagnusFileName, "d:\\didiprogram\\magnus.hex");
	
			{
			   //send begin command
			  	//测试申请外部内存
				didiflash = (unsigned char *) _mem_alloc_zero_from(_user_pool_id, BOOT_DIDIPROGRAMABLESIZE_MAGNUS);
				if (didiflash == NULL)
				{
					printf("error when mem alloc magnusflash buf\r\n");	
					return -1;
				}
				//先用TFTP下载程序到SD卡中
				
				printf("正在从SD卡中载入magnus程序文件到内存并解析，请稍等...\n");
				if (Boot_AnalysisHEX(MagnusFileName, didiflash, data_buffer)==1)
		
				{
					
					printf("载入DiDi程序文件[%s]到内存完成，并解析成功！\n",MagnusFileName);
				}
				else 
				{
					printf("error when analysis didi program file\n");
					printf("载入或解析DiDi程序文件出错，请用ls,dir等命令检查程序文件[%s]是否存在或格式是否正确！\n", MagnusFileName);
				}
				_mem_free(didiflash);
			}
		}
		else  
		{
			printf("Error, %s invoked with incorrect number of arguments\n", argv[0]);
			print_usage = TRUE;
		}

//		taskid = _task_create(0, DICOR_THIRD_DEVICE_TASK, 0);

			return return_code;
	
			 }
			 break;
			 /*
			 	case 0x21:
				 DEBUG_DIS(printf("\n begin to  optimus-bootloader"));
				 
				 //Dimo_response_data.role = 0x01;
		
			   	dicor_kill_task("dicor_third_device_task");
			   	//	 _watchdog_start(10*60000);
			   {
			   
			   
			  char OptimusFileName[100] = 
                            {
                           	'\0'
                            };
	boolean  print_usage, shorthelp = TRUE;
	int_32   return_code = SHELL_EXIT_SUCCESS;
	int_32 argc=3; 
	char_ptr argv[4];
	uint_8  i;
	uint_8  len;
	uint_16 addr_f,addr_r;
 	char hex[3]; 
	unsigned char *didiflash;
	//TIME_STRUCT time1, time2;
	//uint_32 dt;
//	uint_32 minute, second, mil;
    EEPROMDATA_PTR p;
    //uint_8* data_buffer;
    uint_8* data_buffer;
	data_buffer = Dimo_response_data.Data_Buffer;	 
	p = (EEPROMDATA_PTR) base_addr;
   	hex[2] = '\0';


		if (argc == 3)  
		{	
	
			printf("Optimus addr_f=%04X addr_r=%04X", addr_f, addr_r);

			
			StopGetData();
	
			
			strcpy(OptimusFileName, "d:\\didiprogram\\8245V1.0.S");
	
			{
			   //send begin command
			  	//测试申请外部内存
					didiflash = (unsigned char *) _mem_alloc_zero_from(_user_pool_id, BOOT_DIDIPROGRAMABLESIZE_MAGNUS);
					if (didiflash == NULL)
					{
						printf("error when mem alloc magnusflash buf\r\n");	
						return -1;
					}
					//先用TFTP下载程序到SD卡中
					
					printf("正在从SD卡中载入magnus程序文件到内存并解析，请稍等...\n");
				if (!Boot_AnalysisSRecord_8245(OptimusFileName, didiflash,data_buffer))
			
					{
						
						printf("载入DiDi程序文件[%s]到内存完成，并解析成功！\n",OptimusFileName);
					//	_time_get(&time1);
		
					}
					else
					{
						printf("error when analysis didi program file\n");
						printf("载入或解析DiDi程序文件出错，请用ls,dir等命令检查程序文件[%s]是否存在或格式是否正确！\n", OptimusFileName);
					}
					_mem_free(didiflash);
		
						}
			
		} 
		else  
		{
			printf("Error, %s invoked with incorrect number of arguments\n", argv[0]);
			print_usage = TRUE;
		}

	

	return return_code;
			   
			  
			   Dimo_ht_data.Dicor_UID[0]= Dimo_response_data.Dicor_UID[0];
	           Dimo_ht_data.Dicor_UID[1]= Dimo_response_data.Dicor_UID[1];
	           Dimo_ht_data.Dicor_UID[2]= Dimo_response_data.Dicor_UID[2];
	           Dimo_ht_data.Dicor_UID[3]= Dimo_response_data.Dicor_UID[3];
	           Dimo_ht_data.pack_obj=Dimo_response_data.pack_obj;
	           Dimo_ht_data.pack_type=Dimo_response_data.pack_type;
	           Dimo_ht_data.state=0x00;
	         error = send(upload_buffer.sock,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]),
		    sizeof(DIMO_HT_DATA), 0);  	
			 
			   }
			    	 break;
			    	 
				case 0x47:
			   DEBUG_DIS(printf("\n set magnus frequency"));
			    {
			  uint_16 addr;	
			  uint_8* data_buffer;
			  data_buffer = Dimo_response_data.Data_Buffer;
			  SET_MAGNUS_ROLE_Command(addr, data_buffer);
			    }
			   Dimo_ht_data.Dicor_UID[0]= Dimo_response_data.Dicor_UID[0];
	           Dimo_ht_data.Dicor_UID[1]= Dimo_response_data.Dicor_UID[1];
	           Dimo_ht_data.Dicor_UID[2]= Dimo_response_data.Dicor_UID[2];
	           Dimo_ht_data.Dicor_UID[3]= Dimo_response_data.Dicor_UID[3];
	           Dimo_ht_data.pack_obj=Dimo_response_data.pack_obj;
	           Dimo_ht_data.pack_type=Dimo_response_data.pack_type;
	           Dimo_ht_data.state=0x00;
	         error = send(upload_buffer.sock,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]),
		    sizeof(DIMO_HT_DATA), 0);    
			  
			    break;
			    */
			    case 0x66:
			  DEBUG_DIS(printf("\n set FFD table\n"));
			  //	dicor_kill_task("dicor_third_device_task");
			  
			  {
			  uint_16 addr;	
			  uint_8* data_buffer;
			
			  data_buffer = Dimo_response_data.Data_Buffer;
			addr = (uint_16)data_buffer[2];
            addr = addr << 8;
            addr = addr & 0xFF00 ;
            addr|= data_buffer[3];
			  //addr = (Dimo_response_data.data_buffer[0]<<8)+Dimo_response_data.data_buffer[1];
			 SET_FFD_TABLE_Command(addr, data_buffer); 
			  }
			  /*
			   Dimo_ht_data.Dicor_UID[0]= Dimo_response_data.Dicor_UID[0];
	           Dimo_ht_data.Dicor_UID[1]= Dimo_response_data.Dicor_UID[1];
	           Dimo_ht_data.Dicor_UID[2]= Dimo_response_data.Dicor_UID[2];
	           Dimo_ht_data.Dicor_UID[3]= Dimo_response_data.Dicor_UID[3];
	           Dimo_ht_data.pack_obj=Dimo_response_data.pack_obj;
	           Dimo_ht_data.pack_type=Dimo_response_data.pack_type;
	           Dimo_ht_data.state=0x00;
	         error = send(upload_buffer.sock,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]),
		    sizeof(DIMO_HT_DATA), 0);  
		    */  
			    break;  
			    
			   	case 0x04:
			   	dicor_kill_task("dicor_third_device_task");
			   	_time_delay(2000);
			  DEBUG_DIS(printf("\n COLECT RFD DATA"));
			  {
			  uint_16 addr,readstart,readlen;	
			  uint_8* data_buffer;
			  uint_8 result;
			  //uint_16 addr1;
			  // uint_32 period;
			  //void *ptr;
			  data_buffer = Dimo_response_data.Data_Buffer;
			  result=Modbus_SendReadRegCommand(addr,data_buffer,readstart,readlen);
			  // addr1=(uint_16)Dimo_response_data.Data_Buffer[2]+Dimo_response_data.Data_Buffer[3];
			  // period =600;
			  // register_device_Gateway(addr1,period);
			  }
			  	_time_delay(2000);
			   taskid = _task_create(0, DICOR_THIRD_DEVICE_TASK, 0);
			   _time_delay(1000);
			   /*
			   Dimo_ht_data.Dicor_UID[0]= Dimo_response_data.Dicor_UID[0];
	           Dimo_ht_data.Dicor_UID[1]= Dimo_response_data.Dicor_UID[1];
	           Dimo_ht_data.Dicor_UID[2]= Dimo_response_data.Dicor_UID[2];
	           Dimo_ht_data.Dicor_UID[3]= Dimo_response_data.Dicor_UID[3];
	           Dimo_ht_data.pack_obj=Dimo_response_data.pack_obj;
	           Dimo_ht_data.pack_type=Dimo_response_data.pack_type;
	           Dimo_ht_data.state=0x00;
	         error = send(upload_buffer.sock,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]),
		    sizeof(DIMO_HT_DATA), 0);   
		       */
			    break;  
			   
			   	case 0x45:
			  		DEBUG_DIS(printf("\n let  FFD bind RFD\n"));
			  //	dicor_kill_task("dicor_third_device_task");
			  
			  	{
			  		uint_16 addr;	
			  		uint_8* data_buffer;
			  		data_buffer = Dimo_response_data.Data_Buffer;
			  		addr = (uint_16)data_buffer[2];
                 addr = addr<< 8;
            addr = addr & 0xFF00 ;
            addr|= data_buffer[3];
			 		FFD_BIND_RFD_Command(addr, data_buffer); 
			 	//	TURN_ON_Command(addr, data_buffer); 
			 	
			 	
			  	} 
			  	_time_delay(2000);
			   taskid = _task_create(0, DICOR_THIRD_DEVICE_TASK, 0);
			   _time_delay(1000);
			  /*
			    Dimo_ht_data.Dicor_UID[0]= Dimo_response_data.Dicor_UID[0];
	           Dimo_ht_data.Dicor_UID[1]= Dimo_response_data.Dicor_UID[1];
	           Dimo_ht_data.Dicor_UID[2]= Dimo_response_data.Dicor_UID[2];
	           Dimo_ht_data.Dicor_UID[3]= Dimo_response_data.Dicor_UID[3];
	           Dimo_ht_data.pack_obj=Dimo_response_data.pack_obj;
	           Dimo_ht_data.pack_type=Dimo_response_data.pack_type;
	           Dimo_ht_data.state=0x00;
	         error = send(upload_buffer.sock,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]),
		    sizeof(DIMO_HT_DATA), 0);   
		    */ 
			    break;  
			    
			    case 0x42:
			  DEBUG_DIS(printf("\n let FFD start collect data"));
			    	dicor_kill_task("dicor_third_device_task");
			 
			  {
			  uint_16 addr;	
			  uint_8* data_buffer;
			  data_buffer = Dimo_response_data.Data_Buffer;
			addr = (uint_16)data_buffer[2];
            addr = addr << 8;
            addr = addr & 0xFF00 ;
            addr|= data_buffer[3];
			 SW_Command(addr, data_buffer); 
			  } 
			   _time_delay(2000);
			   taskid = _task_create(0, DICOR_THIRD_DEVICE_TASK, 0);
			   _time_delay(1000);
			   /*	 
			   Dimo_ht_data.Dicor_UID[0]= Dimo_response_data.Dicor_UID[0];
	           Dimo_ht_data.Dicor_UID[1]= Dimo_response_data.Dicor_UID[1];
	           Dimo_ht_data.Dicor_UID[2]= Dimo_response_data.Dicor_UID[2];
	           Dimo_ht_data.Dicor_UID[3]= Dimo_response_data.Dicor_UID[3];
	           Dimo_ht_data.pack_obj=Dimo_response_data.pack_obj;
	           Dimo_ht_data.pack_type=Dimo_response_data.pack_type;
	           Dimo_ht_data.state=0x00;
	         error = send(upload_buffer.sock,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]),
		    sizeof(DIMO_HT_DATA), 0);  
		      */
		     
			    break; 
			    
			     case 0x44:
			   DEBUG_DIS(printf("\n let magnus shutdown\n"));
			   	dicor_kill_task("dicor_third_device_task");
			   	_time_delay(2000);
			  {
			  uint_16 addr;	
			  uint_8* data_buffer;
			  data_buffer = Dimo_response_data.Data_Buffer;
			  addr = (uint_16)data_buffer[2];
            addr = addr << 8;
            addr = addr & 0xFF00 ;
            addr|= data_buffer[3];
			  MAGNUS_SHUTDOWN_Command(addr, data_buffer); 
			  }
			  DEBUG_DIS(printf("\n shutdown--OK\n"));
			    _time_delay(1000);
			   taskid = _task_create(0, DICOR_THIRD_DEVICE_TASK, 0);
			    /*
			    _time_delay(1000);
			   Dimo_ht_data.Dicor_UID[0]= Dimo_response_data.Dicor_UID[0];
	           Dimo_ht_data.Dicor_UID[1]= Dimo_response_data.Dicor_UID[1];
	           Dimo_ht_data.Dicor_UID[2]= Dimo_response_data.Dicor_UID[2];
	           Dimo_ht_data.Dicor_UID[3]= Dimo_response_data.Dicor_UID[3];
	           Dimo_ht_data.pack_obj=Dimo_response_data.pack_obj;
	           Dimo_ht_data.pack_type=Dimo_response_data.pack_type;
	           Dimo_ht_data.state=0x00;
	            error = send(upload_buffer.sock,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]),
		    sizeof(DIMO_HT_DATA), 0);
		     */ 
			    break; 
			    
			    case 0x51:
			  DEBUG_DIS(printf("\n set router table\n"));
			   	dicor_kill_task("dicor_third_device_task");
			   	 _time_delay(2000);
			  
			  {
			  uint_16 addr;	
			  uint_8* data_buffer;
			  data_buffer = Dimo_response_data.Data_Buffer;
			addr = (uint_16)data_buffer[2];
            addr = addr<< 8;
            addr = addr & 0xFF00 ;
           addr|= data_buffer[3];
			 SET_ROUTER_TABLE_Command(addr, data_buffer); 
			  } 
			  /*
			   Dimo_ht_data.Dicor_UID[0]= Dimo_response_data.Dicor_UID[0];
	           Dimo_ht_data.Dicor_UID[1]= Dimo_response_data.Dicor_UID[1];
	           Dimo_ht_data.Dicor_UID[2]= Dimo_response_data.Dicor_UID[2];
	           Dimo_ht_data.Dicor_UID[3]= Dimo_response_data.Dicor_UID[3];
	           Dimo_ht_data.pack_obj=Dimo_response_data.pack_obj;
	           Dimo_ht_data.pack_type=Dimo_response_data.pack_type;
	           Dimo_ht_data.state=0x00;
	         error = send(upload_buffer.sock,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]),
		    sizeof(DIMO_HT_DATA), 0);
		    */    
			    break; 
			    
			case 0x41:
				DEBUG_DIS(printf("\n let magnus power-on\n"));
				dicor_kill_task("dicor_third_device_task");
				_time_delay(2000);
				{
					uint_16 addr;	
					uint_8* data_buffer;
					data_buffer = Dimo_response_data.Data_Buffer;
					addr = (uint_16)data_buffer[2];
					addr = addr << 8;
					addr = addr & 0xFF00 ;
					addr|= data_buffer[3];
					TURN_ON_Command(addr, data_buffer); 
				}
				_time_delay(1000);
				taskid = _task_create(0, DICOR_THIRD_DEVICE_TASK, 0);
			    break; 
			    
			case 0x46:
				DEBUG_DIS(printf("\n start poll\n"));
				dicor_kill_task("dicor_third_device_task");
				_time_delay(2000);
				{
					uint_16 addr;	
					uint_8* data_buffer;
					data_buffer = Dimo_response_data.Data_Buffer;
					addr = (uint_16)data_buffer[2];
					addr = addr << 8;
					addr = addr & 0xFF00 ;
					addr|= data_buffer[3];
					Modbus_SendStartCommand(addr, data_buffer); 
				}
				_time_delay(1000);
				taskid = _task_create(0, DICOR_THIRD_DEVICE_TASK, 0);
			    break;
			    
			case 0x03:
				DEBUG_DIS(printf("\n read magnus parameter\n"));
				dicor_kill_task("dicor_third_device_task");
				_time_delay(2000);
				{
					uint_16 addr,addr_f;	
					uint_8* data_buffer;
					data_buffer = Dimo_response_data.Data_Buffer;
					addr = (uint_16)data_buffer[4];
					addr = addr << 8;
					addr = addr & 0xFF00 ;
					addr|= data_buffer[5];
					Read_Magnus_Parameter_CMD(addr_f,addr,data_buffer);
				}
				_time_delay(2000);
				taskid = _task_create(0, DICOR_THIRD_DEVICE_TASK, 0);
				_time_delay(1000);
			    break;
			    
			    
			case 0x10:
				DEBUG_DIS(printf("\n write magnus parameter\n"));
				DEBUG_DIS(printf("Cosmos current GatewayID 0x%04x\n", CoodAddr));
				dicor_kill_task("dicor_third_device_task");
				_time_delay(4000);
				{
					uint_16 addr,addr_f;	
					uint_8* data_buffer;
					data_buffer = Dimo_response_data.Data_Buffer;
					addr = (uint_16)data_buffer[4];
					addr = addr << 8;
					addr = addr & 0xFF00 ;
					addr|= data_buffer[5];
					Write_Magnus_Parameter_CMD(addr_f,addr,data_buffer);
			
					_time_delay(2000);
					taskid = _task_create(0, DICOR_THIRD_DEVICE_TASK, 0);
					_time_delay(2000);

					if ((data_buffer[1] == data_buffer[3]) \
						&& (data_buffer[3] == data_buffer[5]) \
						&& (data_buffer[1] != data_buffer[12]))
					{
						EepromWriteCoodaddr(&data_buffer[12]);
						printf("_flash write finish_\n");
					}
		       	}
			    break;
			  
			  	}
		 break;
	
		default:
			break;
		}		/* Endswitch */
	}			/* Endif */

	

}				/* Endbody */

void dicor_close_socket(void)
{
	shutdown(upload_buffer.sock, FLAG_CLOSE_TX);
	if (SERVER_MODE == 2)
		shutdown(upload_buffer.sock_mux, FLAG_CLOSE_TX);
	printf("\nClose Socket FLAG!\n");
	_time_delay(1000);
}

static uint_32 dicor_connect_datacenter(void)
{
	sockaddr_in server_addr;
	uint_32 error;
	uint_32 option;
	DATE_STRUCT date;
	
	memset((char *) &server_addr, 0, sizeof(sockaddr_in));
	if (upload_buffer.sock != RTCS_SOCKET_ERROR) {
		shutdown(upload_buffer.sock, FLAG_ABORT_CONNECTION);
	}
	upload_buffer.sock = socket(PF_INET, SOCK_STREAM, 0);
	if (upload_buffer.sock == RTCS_SOCKET_ERROR) {
		dicor_get_logtime(&date);
		sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t 连接数据中心时创建Socket失败\r\n", date.HOUR,date.MINUTE,date.SECOND);
		PrintLog(pDiCorLog->logbuf);
		DEBUG_DIS(printf("Create a socket error!\n"));
		return (1);
	}

	option = FALSE;
	error = setsockopt(upload_buffer.sock, SOL_TCP, OPT_SEND_NOWAIT, &option, sizeof(option));

	option = 0;		//because the above setting        
	error = setsockopt(upload_buffer.sock, SOL_TCP, OPT_SEND_TIMEOUT, &option, sizeof(option));

	option = TRUE;
	error = setsockopt(upload_buffer.sock, SOL_TCP, OPT_RECEIVE_NOWAIT,
		       &option, sizeof(option));


	option = TRUE;		// send data immediately,not waite buffer is full       
	error = setsockopt(upload_buffer.sock, SOL_TCP, OPT_SEND_PUSH, &option, sizeof(option));
	error = setsockopt(upload_buffer.sock, SOL_TCP, OPT_NO_NAGLE_ALGORITHM, &option, sizeof(option));

	option = 1460*2;		// max TCP packet in protocol   
	error =  setsockopt(upload_buffer.sock, SOL_TCP, OPT_TBSIZE, &option, sizeof(option));

	option = 1460;
	error = setsockopt(upload_buffer.sock, SOL_TCP, OPT_RBSIZE, &option, sizeof(option));

	option = 60000UL;	//5min改成1min
	error =  setsockopt(upload_buffer.sock, SOL_TCP, OPT_CONNECT_TIMEOUT, &option, sizeof(option));
	if (error != RTCS_OK) {
		shutdown(upload_buffer.sock, FLAG_ABORT_CONNECTION);
		DEBUG_DIS(printf("\n to set parameters to socket gets wrong"));
		dicor_get_logtime(&date);
		sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t 设置Socket参数时失败\r\n", date.HOUR,date.MINUTE,date.SECOND);
		PrintLog(pDiCorLog->logbuf);
		return (3);
	}
	option = 4000;
	error = setsockopt(upload_buffer.sock, SOL_TCP, OPT_RETRANSMISSION_TIMEOUT, &option, sizeof(option));
	option = 30000;
	error = setsockopt(upload_buffer.sock, SOL_TCP, OPT_MAXRTO, &option, sizeof(option));
	option = 2000;
	error = setsockopt(upload_buffer.sock, SOL_TCP, OPT_TIMEWAIT_TIMEOUT, &option, sizeof(option));


	option = 0;		//not detect, decting the peer is present.
	error = setsockopt(upload_buffer.sock, SOL_TCP, OPT_KEEPALIVE, &option, sizeof(option));
	
	if (error != RTCS_OK) {
		shutdown(upload_buffer.sock, FLAG_ABORT_CONNECTION);
		DEBUG_DIS(printf("\n to set parameters to socket gets wrong"));
		dicor_get_logtime(&date);
		sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t 设置Socket参数时失败\r\n", date.HOUR,date.MINUTE,date.SECOND);
		PrintLog(pDiCorLog->logbuf);

		return (3);

	}
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = 0;
	server_addr.sin_addr.s_addr = dicor_ip_data.ip;
	error = bind(upload_buffer.sock, &server_addr, sizeof(server_addr));
	if (error != RTCS_OK) {
		shutdown(upload_buffer.sock, FLAG_ABORT_CONNECTION);
		DEBUG_DIS(printf("\n to bind gets wrong"));
		dicor_get_logtime(&date);
		sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t Socket bind时出错\r\n", date.HOUR,date.MINUTE,date.SECOND);
		PrintLog(pDiCorLog->logbuf);
		return (2);
	}
	server_addr.sin_port = IPADDR(pBaseConfig->ipport[0],pBaseConfig->ipport[1],pBaseConfig->ipport[2],pBaseConfig->ipport[3]);

	if (pBaseConfig->enabledns)
	{	
		DEBUG_DIS(printf("\nGetting ip from DNS server ... "));
		if (RTCS_resolve_ip_address(pBaseConfig->datacenterdns, &server_addr.sin_addr.s_addr, NULL, 0))
		{
			DEBUG_DIS(printf("\n get Ip: %d.%d.%d.%d \n", IPBYTES(server_addr.sin_addr.s_addr)));
		} else {
			DEBUG_DIS(printf("Failed in getting server IP\n"));
			shutdown(upload_buffer.sock, FLAG_ABORT_CONNECTION);
			dicor_get_logtime(&date);
			sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t 对数据中心域名解析出错\r\n", date.HOUR,date.MINUTE,date.SECOND);
			PrintLog(pDiCorLog->logbuf);
			return (3);
		}
	}
	else
	{
		server_addr.sin_addr.s_addr = IPADDR(pBaseConfig->datacenter[0],pBaseConfig->datacenter[1],pBaseConfig->datacenter[2],pBaseConfig->datacenter[3]);
	}
//	DEBUG_DIS(printf("\nconnecting to data center: %d.%d.%d.%d \n", IPBYTES(server_addr.sin_addr.s_addr)));
//	DEBUG_DIS(printf("port:%d \n", server_addr.sin_port));
	if (RTCS_OK != connect(upload_buffer.sock, &server_addr, sizeof(server_addr)))
	{
		shutdown(upload_buffer.sock, FLAG_ABORT_CONNECTION);
		DEBUG_DIS(printf("\n Error in connect() with error code: %01x!", RTCS_geterror(upload_buffer.sock)));
		_time_delay_ticks(100);
		dicor_get_logtime(&date);
		EthernetLedOff();
		DimoLedOff();
		SdLedOff();
		sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t Socket connect出错，错误代码:%08X\r\n", date.HOUR,date.MINUTE,date.SECOND,RTCS_geterror(upload_buffer.sock));
		PrintLog(pDiCorLog->logbuf);
		return (4);//返回4是如何处理呢？
	}
	DEBUG_DIS(printf("\nconnected default"));
//	DiCor_SendIntfaceVersion();
    EthernetLedOn();
    DimoLedOn();
    SdLedOn();
	return (0);

}

static uint_32 dicor_connect_datacentermux(void)
{
	sockaddr_in server_addr_mux;
	uint_32 error;
	uint_32 option;
	DATE_STRUCT date;
	memset((char *) &server_addr_mux, 0, sizeof(sockaddr_in));
	if (upload_buffer.sock_mux != RTCS_SOCKET_ERROR) {
		shutdown(upload_buffer.sock_mux, FLAG_ABORT_CONNECTION);
	}
	upload_buffer.sock_mux = socket(PF_INET, SOCK_STREAM, 0);
	if (upload_buffer.sock_mux == RTCS_SOCKET_ERROR) {
		dicor_get_logtime(&date);
		sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t 连接数据中心时创建Socket失败\r\n", date.HOUR,date.MINUTE,date.SECOND);
		PrintLog(pDiCorLog->logbuf);
		DEBUG_DIS(printf("Create a socket error!\n"));
		return (1);
	}
	option = FALSE;
	error = setsockopt(upload_buffer.sock_mux, SOL_TCP, OPT_SEND_NOWAIT, &option, sizeof(option));

	option = 0;	   
	error = setsockopt(upload_buffer.sock_mux, SOL_TCP, OPT_SEND_TIMEOUT, &option, sizeof(option));

	option = TRUE;
	error = setsockopt(upload_buffer.sock_mux, SOL_TCP, OPT_RECEIVE_NOWAIT, &option, sizeof(option));


	option = TRUE;		// send data immediately,not waite buffer is full       
	error = setsockopt(upload_buffer.sock_mux, SOL_TCP, OPT_SEND_PUSH, &option, sizeof(option));
	error = setsockopt(upload_buffer.sock_mux, SOL_TCP, OPT_NO_NAGLE_ALGORITHM, &option, sizeof(option));

	option = 1460*2;		// max TCP packet in protocol   
	error = setsockopt(upload_buffer.sock_mux, SOL_TCP, OPT_TBSIZE, &option, sizeof(option));

	option = 1460;
	error = setsockopt(upload_buffer.sock_mux, SOL_TCP, OPT_RBSIZE, &option, sizeof(option));

	option = 5000UL;	//5min改成1min  60000改为5000五秒
	error = setsockopt(upload_buffer.sock_mux, SOL_TCP, OPT_CONNECT_TIMEOUT, &option, sizeof(option));
	if (error != RTCS_OK) {
		shutdown(upload_buffer.sock_mux, FLAG_ABORT_CONNECTION);
		DEBUG_DIS(printf("\n to set parameters to socket gets wrong"));
		dicor_get_logtime(&date);
		sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t 设置Socket参数时失败\r\n", date.HOUR,date.MINUTE,date.SECOND);
		PrintLog(pDiCorLog->logbuf);
		return (3);
	}
	option = 4000;
	error = setsockopt(upload_buffer.sock_mux, SOL_TCP, OPT_RETRANSMISSION_TIMEOUT, &option, sizeof(option));
	
	option = 30000;
	error = setsockopt(upload_buffer.sock_mux, SOL_TCP, OPT_MAXRTO, &option, sizeof(option));
	
	option = 2000;
	error = setsockopt(upload_buffer.sock_mux, SOL_TCP, OPT_TIMEWAIT_TIMEOUT, &option, sizeof(option));
	
	option = 0;		//not detect, decting the peer is present.
	error = setsockopt(upload_buffer.sock_mux, SOL_TCP, OPT_KEEPALIVE, &option, sizeof(option));
	
	if (error != RTCS_OK) {
		shutdown(upload_buffer.sock_mux, FLAG_ABORT_CONNECTION);
		DEBUG_DIS(printf("\n to set parameters to socket gets wrong"));
		dicor_get_logtime(&date);
		sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t 设置Socket参数时失败\r\n", date.HOUR,date.MINUTE,date.SECOND);
		PrintLog(pDiCorLog->logbuf);
		return (3);
	}

	server_addr_mux.sin_family = AF_INET;
	server_addr_mux.sin_port = 0;
	server_addr_mux.sin_addr.s_addr = dicor_ip_data.ip;
	
	error = bind(upload_buffer.sock_mux, &server_addr_mux, sizeof(server_addr_mux));
	if (error != RTCS_OK) {
		shutdown(upload_buffer.sock_mux, FLAG_ABORT_CONNECTION);
		DEBUG_DIS(printf("\n to bind gets wrong"));
		dicor_get_logtime(&date);
		sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t Socket bind时出错\r\n", date.HOUR,date.MINUTE,date.SECOND);
		PrintLog(pDiCorLog->logbuf);
		return (2);
	}
	server_addr_mux.sin_port = IPADDR(pBaseConfig->ipportmux[0],pBaseConfig->ipportmux[1],pBaseConfig->ipportmux[2],pBaseConfig->ipportmux[3]);
	if (pBaseConfig->enabledns)
	{		
		DEBUG_DIS(printf("\nGetting ip from DNS server ... "));
		if (RTCS_resolve_ip_address(pBaseConfig->datacenterdns, &server_addr_mux.sin_addr.s_addr, NULL, 0)) 
		{
			DEBUG_DIS(printf("\n get Ip: %d.%d.%d.%d \n",
					 IPBYTES(server_addr_mux.sin_addr.s_addr)));
		} else {
			DEBUG_DIS(printf("Failed in getting server IP\n"));
			shutdown(upload_buffer.sock_mux, FLAG_ABORT_CONNECTION);
			dicor_get_logtime(&date);
			sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t 对数据中心域名解析出错\r\n", date.HOUR,date.MINUTE,date.SECOND);
			PrintLog(pDiCorLog->logbuf);
			return (3);
		}
	} else {
		server_addr_mux.sin_addr.s_addr = IPADDR(pBaseConfig->datacentermux[0],pBaseConfig->datacentermux[1],pBaseConfig->datacentermux[2],pBaseConfig->datacentermux[3]);
	}
//	DEBUG_DIS(printf("\nconnecting to data center mux: %d.%d.%d.%d \n", IPBYTES(server_addr_mux.sin_addr.s_addr)));
//	DEBUG_DIS(printf("port mux:%d \n", server_addr_mux.sin_port));
	
	if (RTCS_OK != connect(upload_buffer.sock_mux, &server_addr_mux, sizeof(server_addr_mux)))
	{
		shutdown(upload_buffer.sock_mux, FLAG_ABORT_CONNECTION);
		DEBUG_DIS(printf("\n Error in connect() with error code: %01x!",
			   RTCS_geterror(upload_buffer.sock_mux)));
		_time_delay_ticks(100);
		dicor_get_logtime(&date);
		//EthernetLedOff();
		//DimoLedOff();
		//SdLedOff();
		sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t Socket connect出错，错误代码:%08X\r\n", date.HOUR,date.MINUTE,date.SECOND,RTCS_geterror(upload_buffer.sock));
		PrintLog(pDiCorLog->logbuf);
		return (4);
	}
	DEBUG_DIS(printf("\nconnected mux"));
	return (0);
}

static void RF_Waiter3G(void)
{
	uint_32 error;
	UINT_8 con_time, i;
	con_time = 0;
	DEBUG_DIS(printf("\n active 3G connecting by sending  dirty data"));
	if (upload_buffer.eth_st == ETH_CABLE_IP_CON) {
		DICOR_REG_H_PTR reg_h;
		reg_h = (DICOR_REG_H_PTR) & upload_buffer.data0;
		reg_h->type = 0x00;	//dirty data 
		reg_h->flag = 0x11;	//last
		reg_h->index = 0x00;	// the first frame
		reg_h->num = 0x00;	//the number of reg item.
		for (i = 0; i < 100; i++) {
			error =
			    send(upload_buffer.sock,
				 (int_8_ptr) (&upload_buffer.data0.type),
				 128, 0);
			if (error == RTCS_ERROR) {
				con_time = 0;
				DEBUG_DIS(printf
					  ("\n Error in send() with error code: %01x!",
					   RTCS_geterror
					   (upload_buffer.sock)));

			} else {
				con_time++;
				_time_delay(2000);
				if (con_time > 3) {
					break;
				}
			}
		}
	}
}


static void DiCor_SendIntfaceVersion(void)
{
	uint_32 error;
	DATE_STRUCT date;
	DEBUG_DIS(printf("\n send protocol version data"));
	if (upload_buffer.eth_st == ETH_CABLE_IP_CON) {
		UINT_8 *data_p;
		data_p = (UINT_8 *) & upload_buffer.data0;
		*data_p++ = 0x20;	//interface version 
		//UID
		*data_p++ = pBaseConfig->uid[0];
		*data_p++ = pBaseConfig->uid[1];
		*data_p++ = pBaseConfig->uid[2];
		*data_p++ = pBaseConfig->uid[3];
		//version 01:00:00:02
		*data_p++ = 0x36;
		*data_p++ = 0x00;
		*data_p++ = 0x05;
		*data_p++ = 0x28;
		if (SERVER_MODE == 2)
			error = send(upload_buffer.sock_mux, (int_8_ptr) (&upload_buffer.data0.type), 12, 0);
		error = send(upload_buffer.sock, (int_8_ptr) (&upload_buffer.data0.type), 12, 0);
		if (error == RTCS_ERROR) {
			DEBUG_DIS(printf ("\n Error in send() with error code: %01x!", RTCS_geterror(upload_buffer.sock)));
			dicor_get_logtime(&date);
			sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t 向DiMo发送版本号时出错，调用Send出错，错误代码%08X\r\n", date.HOUR,date.MINUTE,date.SECOND,RTCS_geterror(upload_buffer.sock));
			PrintLog(pDiCorLog->logbuf);
		}
	}
}



void RF_SetupNetwork(void)
{
	uint_32 error;
	DATE_STRUCT date;

	DEBUG_DIS(printf("\n start to setup netRF"));
#if DICOR_STATIC_REG_TAB
	upload_buffer.state = CAN_READ;
	EMN_ChildNum = EMN_DEV_NUM;
	rfnwk_state = RF_NWK_IDLE;
#else
	rfnwk_state = RF_NWK_REGING;
	upload_buffer.write_index = 0;
	upload_buffer.state = CAN_WRITE;
	if (NoRegTimes == 0) {
//    dicor_rf_signal(RF_REG_START);               
	}
//  dicor_waite_rf(RF_REG_END,0);   //must waite this message 
#endif
	//send this message to data center
	if ((upload_buffer.eth_st == ETH_CABLE_IP_CON) &&
	    (upload_buffer.state == CAN_READ) && (EMN_ChildNum != 0)) {
#if DICOR_STATIC_REG_TAB
//   DEBUG_DIS(printf("\nsend register table"));
	if (SERVER_MODE == 2)
		error = send(upload_buffer.sock_mux, (int_8_ptr) (&RegTable[0]), (uint_32) (EMN_DEV_NUM * 6 + DICOR_ID_LEN + 4), 0);
	error = send(upload_buffer.sock, (int_8_ptr) (&RegTable[0]), (uint_32) (EMN_DEV_NUM * 6 + DICOR_ID_LEN + 4), 0);

			 
#else
		DICOR_REG_H_PTR reg_h;
		reg_h = (DICOR_REG_H_PTR) & upload_buffer.data0;
		reg_h->type = 0xaa;	//reg table
		reg_h->flag = 0x11;	//last
		reg_h->index = 0x00;	// the first frame
		reg_h->num = EMN_ChildNum;	//the number of reg item.
		if (SERVER_MODE == 2)
			error = send(upload_buffer.sock_mux, (int_8_ptr) (&upload_buffer.data0.type), (uint_32) (upload_buffer.write_index + DICOR_ID_LEN + 4), 0);
		error = send(upload_buffer.sock, (int_8_ptr) (&upload_buffer.data0.type), (uint_32) (upload_buffer.write_index + DICOR_ID_LEN + 4), 0);

#endif
		if (error == RTCS_ERROR) {
			dicor_get_logtime(&date);
			sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t 向DiMo发送注册表失败，调用send出错，系统将重启\r\n", date.HOUR,date.MINUTE,date.SECOND);
			PrintLog(pDiCorLog->logbuf);
			SYS_Error(0x82);
		} else {
			DEBUG_DIS(printf("\n send register table to data center"));
		}
	}
	DEBUG_DIS(printf("\n completed setuping netRF"));
}

void dicor_check_connect(void)  //1 secend do 
{
	uint_32 error;
	switch (upload_buffer.eth_st) 
	{
		case ETH_INIT:
			EthernetLedOff();
			SdLedOff();
			if (ipcfg_get_link_active(0)) {
				upload_buffer.eth_st = ETH_CABLE;//此处向下继续
			} //else {
				break;
			//}
		case ETH_CABLE:
			//添加状态检测
			if (!(ipcfg_get_link_active(0))) 
			{
				upload_buffer.eth_st = ETH_INIT;
				shutdown(upload_buffer.sock,FLAG_ABORT_CONNECTION);
				EthernetLedOff();
				//DimoLedOff();
				SdLedOff();
				DEBUG_DIS(printf("\n please plug cable"));
				return;
			} else {
			
				if (pBaseConfig->dhcp) {
					DEBUG_DIS(printf("\nDHCP bind ... "));
					error = ipcfg_bind_dhcp_wait(0, 0, &dicor_ip_data);
					if (error != IPCFG_ERROR_OK) {
						SdLedOff();
						//EthernetLedOff();
						DEBUG_DIS(printf("Error %08x!\n", error));
						break;
					} else {
						SdLedOn();
						//EthernetLedOn();
						DEBUG_DIS(printf("Successful!\n"));
						ipcfg_get_ip(0, &dicor_ip_data);
						upload_buffer.eth_st = ETH_CABLE_IP;
					}
					DEBUG_DIS(printf("\nIP Address: %d.%d.%d.%d\n", IPBYTES(dicor_ip_data.ip)));
				} else {
					/* Else bind with static IP */
					DEBUG_DIS(printf("\nStatic IP bind ... "));
					dicor_ip_data.ip = IPADDR(pBaseConfig->ip[0],pBaseConfig->ip[1],pBaseConfig->ip[2],pBaseConfig->ip[3]);
					dicor_ip_data.mask = IPADDR(pBaseConfig->mask[0],pBaseConfig->mask[1],pBaseConfig->mask[2],pBaseConfig->mask[3]);
					dicor_ip_data.gateway = IPADDR(pBaseConfig->gateway[0],pBaseConfig->gateway[1],pBaseConfig->gateway[2],pBaseConfig->gateway[3]);

					error = ipcfg_bind_staticip(0, &dicor_ip_data);

					if (error != IPCFG_ERROR_OK) {
						SdLedOff();
						//EthernetLedOff();
						DEBUG_DIS(printf("Error %08x!\n", error));
						break;
					} else {
						SdLedOn();
						//EthernetLedOn();
						DEBUG_DIS(printf("Successful!\n"));
						upload_buffer.eth_st = ETH_CABLE_IP;
					
					}	
				}
			}
			break;
			
		case ETH_CABLE_IP:
			//添加状态检测
			if (!(ipcfg_get_link_active(0))) {
				upload_buffer.eth_st = ETH_INIT;
				shutdown(upload_buffer.sock, FLAG_ABORT_CONNECTION);
				EthernetLedOff();
				DimoLedOff();
				SdLedOff();
				DEBUG_DIS(printf("\n please plug cable"));
				return;
			} else {
				dicor_get_timer();
				//error = dicor_connect_datacenter();//|dicor_connect_datacenter2()???
				error = dicor_connect_datacenter();
				if (error == 0) {
					upload_buffer.eth_st = ETH_CABLE_IP_CON;
					DEBUG_DIS(printf("\n connected data center"));
					//EthernetLedOn();
					//DimoLedOn();
					SdLedOn();//eth
					DiCor_SendIntfaceVersion();
					upload_buffer.data0.type = 0x12;	//data package
					FirstPackage = 1;
				} else {
					//DimoLedOff();
					EthernetLedOff();//server
					upload_buffer.eth_st = ETH_CABLE;
				}
			}
			break;
			
		case ETH_CABLE_IP_CON:
			SdLedOn();//eth
			EthernetLedOn();//server
			if (!(ipcfg_get_link_active(0))) {
				upload_buffer.eth_st = ETH_INIT;
				shutdown(upload_buffer.sock, FLAG_ABORT_CONNECTION);
				DEBUG_DIS(printf("\n please plug cable"));
				EthernetLedOff();
				DimoLedOff();
				SdLedOff();
			}
			break;
		default:
			break;
	}
}


void dicor_get_timer(void)
{
	TIME_STRUCT time;
	DATE_STRUCT date;
	RTC_TIME_STRUCT time_rtc;
	if (dicor_GetTime() == 0)	//成功从网络上获取到时间
	{
		_time_get(&time);
		//将互联网的格林时间转换成北京时间
		//time.SECONDS += 8*3600;
		_time_to_date(&time, &date);
		if (date.YEAR >= 2011 && date.YEAR < 2100)	//确保所得时间是对的
		{
			//设置一下RTC时间
			_rtc_time_from_mqx_time(&time, &time_rtc);
			_rtc_set_time(&time_rtc);
			printf("Set rtc time via Internet time!\n");
		}
	}

	_rtc_get_time(&time_rtc);
	_rtc_time_to_mqx_time(&time_rtc, &time);
	_time_to_date(&time, &date);

	DEBUG_DIS(printf("\nSystem Time: %02d/%02d/%02d %02d:%02d:%02d\n",
			 date.YEAR, date.MONTH, date.DAY, date.HOUR,
			 date.MINUTE, date.SECOND));
}

void dicor_get_time(DATE_STRUCT * date)
{
	TIME_STRUCT time;
	_time_get(&time);
	_time_to_date(&time, date);
}

void dicor_get_rtctime(DATE_STRUCT * date)
{
	TIME_STRUCT time;
	//DATE_STRUCT date; 
	RTC_TIME_STRUCT time_rtc;

	_rtc_get_time(&time_rtc);
	_rtc_time_to_mqx_time(&time_rtc, &time);
	_time_to_date(&time, date);

}

void dicor_dis_timer(void)
{
	TIME_STRUCT time;
	DATE_STRUCT date;
	_time_get(&time);
	_time_to_date(&time, &date);
	printf("\nSystem Time: %02d/%02d/%02d %02d:%02d:%02d",
	       date.YEAR, date.MONTH, date.DAY, date.HOUR, date.MINUTE,
	       date.SECOND);
}

static void test_closetime(void)
{
	TIME_STRUCT time1, time2;
	FILE_PTR fd_ptr1, fd_ptr2, fd_ptr3, fd_ptr4;
	char buffer[20] = "123456789abcd\r\n";
	_time_delay(5000);

	fd_ptr1 = fopen("d:123.txt", "a+");
	if (fd_ptr1 == NULL) {
		printf("SD card:open file error");
	}

	fwrite(buffer, 40, 1, fd_ptr1);

	fd_ptr3 = fopen("d:12345.txt", "a+");
	if (fd_ptr3 == NULL) {
		printf("SD card:open file error");
	}

	fwrite(buffer, 40, 1, fd_ptr3);


	fd_ptr2 = fopen("c:\\123.txt", "a+");
	if (fd_ptr2 == NULL) {
		printf("U:open file error");
	}

	fwrite(buffer, 40, 1, fd_ptr2);

	fd_ptr4 = fopen("c:\\12345.txt", "a+");
	if (fd_ptr4 == NULL) {
		printf("U:open file error");
	}

	fwrite(buffer, 40, 1, fd_ptr4);

	_time_get(&time1);
	fclose(fd_ptr1);
	fclose(fd_ptr2);
	fclose(fd_ptr4);
	fclose(fd_ptr3);
	_time_get(&time2);


	printf("SECOND1=%d MILIS1=%d\r\n", time1.SECONDS,
	       time1.MILLISECONDS);
	printf("SECOND2=%d MILIS2=%d\r\n", time2.SECONDS,
	       time2.MILLISECONDS);

	printf("dt=%d dms=%d\r\n", time2.SECONDS - time1.SECONDS,
	       time2.MILLISECONDS - time1.MILLISECONDS);

}

int_32 dicor_kill_task(char* taskname)
{
	_task_id task_id;
   	int_32  result;
	task_id = _task_get_id_from_name(taskname);
    if (task_id == MQX_NULL_TASK_ID)  
    {
		printf("No task named %s running.\n",taskname);
		result = -1;
	} 
	else  
	{
		result = _task_destroy(task_id);
		if (result == MQX_OK)  
		{
			printf("Task %s killed.\n",taskname);
		} 
		else  
		{
			printf("Unable to kill task %s.\n",taskname);
		}
	}
	return result;
}

void Dicor_rebootfile(void);
extern void rf_nwk_exit(void);


void Dicor_rebootfile(void)
{
	FILE_PTR fd_ptr;
	int error;
	DATE_STRUCT date;
	
	fd_ptr = fopen("d:\\reboot", "r");
	if (fd_ptr != NULL)
	{
		fclose(fd_ptr);

		error = ioctl(fd_ptr, IO_IOCTL_DELETE_FILE, "d:\\reboot");
		if (error)  
		{
	       printf("\r\nError deleting file!\r\n");
	       return;
	    }
	    
	 
	    
	    dicor_get_logtime(&date);
	    sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t 发现重启文件，马上重启\r\n", date.HOUR,date.MINUTE,date.SECOND);
				PrintLog(pDiCorLog->logbuf);
	
				//保存现场并重启进入bootloader 升级
				upload_data_exit();
				rf_nwk_exit();
				printf("\r\nfound reboot file, reboot now!\r\n");
				Dicor_Reboot();
	    
	}	
}

void dicor_connect_task(uint_32 initial_data)
{
	uint_32 error;
	if (_watchdog_create_component(BSP_TIMER_INTERRUPT_VECTOR,Wacthdog_Error)!= MQX_OK)  
	{ 
	  printf("connect task create watchdog failed !");
	}
	       
	_watchdog_start(60*1000);	
	while (1)
	{
		if (upload_buffer.eth_st != ETH_CABLE_IP_CON)
		{
			DimoLedOff();
			dicor_check_connect();
		}
		_watchdog_start(60*1000);
		_time_delay(1000);//由1000改为500
	}
}


void dicor_upload_data_task(uint_32 initial_data)
{
	uint_32 error;
	_mqx_uint result;
	TIME_STRUCT temptime;
	DATE_STRUCT date;
	_task_id taskid;
	uint_8 test = 0;
	uint_32 offset = 0;
	uint_32 bak_offset = 0;
	NoRfTimes = 0;
	NoRegTimes = 0;

	if (_watchdog_create_component(BSP_TIMER_INTERRUPT_VECTOR,Wacthdog_Error)!= MQX_OK)  
	{ 
		printf("upload task create watchdog failed !");
		Dicor_Reboot();
	}
	
	 _watchdog_start(60*1000);
	
	SDcard_Install();
	LogInit();
	LostLogInit();
	
	//2014.4.3  增加开机自动wall的功能
//	{
//		int_32 argc; 
//		char_ptr argv[2];
//		Shell_wall(1,argv);
//	}
	EepromReadCoodaddr();
   
	dicor_get_logtime(&date);
	sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t 开机，系统启动 (软件版本号: %c%d.%d.%d)\r\n", 
	date.HOUR,date.MINUTE,date.SECOND,
	pBaseConfig->softversion[0],pBaseConfig->softversion[1],pBaseConfig->softversion[2],pBaseConfig->softversion[3]);

	PrintLog(pDiCorLog->logbuf);
	pDiCorLog->updatetimes = 0;
	
	BreakpointInit();
	_watchdog_stop();	
	_watchdog_start(300*1000);
	DEBUG_DIS(printf("\nInitialize Networking ...\n"));
	if (pBaseConfig->dhcp) {
		error = dicor_InitializeNetworking(4, 4, 2, 1);
	} else {
		error = dicor_InitializeNetworking(4, 4, 2, 0);
	}
	if (error == 0) {
		upload_buffer.eth_st = ETH_CABLE_IP;
	} else if (error == 1) {
		upload_buffer.eth_st = ETH_INIT;
	} else if (error == 2) {
		upload_buffer.eth_st = ETH_CABLE;
	} else if (error == 3) {
		upload_buffer.eth_st = ETH_NO_INIT;
	}
	if (upload_buffer.eth_st == ETH_CABLE_IP) {
		if (SERVER_MODE == 2)
			error = dicor_connect_datacentermux();
		error = dicor_connect_datacenter(); 			
		if (error == 0)	{//ok
			DimoLedOn();
			EthernetLedOn();
			upload_buffer.eth_st = ETH_CABLE_IP_CON;
			DEBUG_DIS(printf("\n connected data center"));
		}
	}//此处要处理连接最终状态	
	_watchdog_stop();	
	_watchdog_start(60*1000);	
	DiCor_SendIntfaceVersion();

	RF_SetupNetwork();
	dicor_rf_signal(RF_REG_START);
	_watchdog_stop();
#if 0
	if (MQX_OK != _lwtimer_add_timer_to_queue(&dicor_timer_queue,
						  dis_timer, 0,
						  dis_timer_isr, NULL)) {
		printf("add_timer failed!");
	}
#endif  
	if (upload_buffer.eth_st == ETH_CABLE_IP_CON)
	{
		_watchdog_start(10*60*1000);
	}
   	else
   	{
   		watchdogenable = 0;
   	}
	
	taskid = _task_create(0, DICOR_CONNECT_TASK, 0); 
	if (taskid == MQX_NULL_TASK_ID)
	{
		printf("Could not create DICOR_CONNECT_TASK \n");
	}

	_lwevent_clear(&RF_NET_event, THIRD_DEVICE_DATA_END);
	_lwevent_set(&RF_NET_event, THIRD_DEVICE_DATA_START);

	_time_delay(1000*15);	
	
	upload_buffer.data0.type = 0x12;	//data package
	upload_buffer.state = CAN_WRITE;
	upload_buffer.write_index = 0;
	DEBUG_DIS(printf("\nwaitting data\n"));	
	
	taskid = _task_create(0, DICOR_THIRD_DEVICE_TASK, 0);
	if (taskid == MQX_NULL_TASK_ID)
	{
		printf("Could not create DICOR_THIRD_DEVICE_TASK \n");
	}
	printf ("\n---third task created---\n");
	
	while (1) 
	{
		_time_get(&temptime);
		if(temptime.SECONDS-SpaceTime.SECONDS > MAXRFBUFFFILLSECONDS) {
			DiCorRunStatus.rf_status = 1;
		}
		else {
			DiCorRunStatus.rf_status = 0;
		}
		pDiCorLog->updatetimes++;
		if (pDiCorLog->updatetimes % 600 == 0) 	
		{	
			Dicor_rebootfile();	//如果检测到有reboot文件，就重启，防止
		}
		if (pDiCorLog->updatetimes % 100 == 0) 
		{
			RunLedBlink();
		}
		if (pDiCorLog->updatetimes >= 4000)//5min保存一次 //60000
		{
			//RunLedOff();
			
			//_time_delay(1000);
			Dimo_hb_data.Dicor_UID[0] = pBaseConfig->uid[0];
			Dimo_hb_data.Dicor_UID[1] = pBaseConfig->uid[1];
			Dimo_hb_data.Dicor_UID[2] = pBaseConfig->uid[2];
			Dimo_hb_data.Dicor_UID[3] = pBaseConfig->uid[3];
			Dimo_hb_data.pack_type = 0x0200;

			Dimo_hb_data.status = 0x0000;
			if (SERVER_MODE == 2)	
				error = send(upload_buffer.sock_mux,(char_ptr)(&Dimo_hb_data.Dicor_UID[0]), sizeof(DIMO_HB_DATA), 0);		
			error = send(upload_buffer.sock,(char_ptr)(&Dimo_hb_data.Dicor_UID[0]), sizeof(DIMO_HB_DATA), 0);
			pDiCorLog->updatetimes = 0;
			printf("\n\ncosmos_update poll");
			//RunLedOn();
		}
		if (upload_buffer.eth_st == ETH_CABLE_IP_CON) {
			//接收来自服务器数据
			//uint32_t opt_value = TRUE;
			//uint32_t opt_length = sizeof(uint32_t);
			dicor_process_DiMo_packet(upload_buffer.sock);
			//失效的socket
			//error = getsockopt(upload_buffer.sock_mux, 0, OPT_SOCKET_ERROR, &opt_value, (uint32_t *)&opt_length);
			//if (RTCS_ERROR != error)
			//{
			if (SERVER_MODE == 2)
				dicor_process_DiMo_packet(upload_buffer.sock_mux);	
			//}		
			result = _lwevent_wait_ticks(&RF_NET_event, THIRD_DEVICE_DATA_END, TRUE, 1);
			_lwevent_clear(&RF_NET_event, THIRD_DEVICE_DATA_END);
		}
		switch (rfnwk_state) 
		{
			case RF_NWK_IDLE:
			if (upload_buffer.eth_st == ETH_CABLE_IP_CON) {
				if (!(ipcfg_get_link_active(0))) {
					upload_buffer.eth_st = ETH_INIT;
					shutdown(upload_buffer.sock, FLAG_ABORT_CONNECTION);
					DEBUG_DIS(printf("\n please plug cable"));
				}
				if (watchdogenable == 0) {
					watchdogenable = 1;
					_watchdog_start(60*60*1000);
				}
			}
			else {
				if (watchdogenable == 1) {
					watchdogenable = 0;
					_watchdog_stop();
				}
			}
			if (upload_buffer.state == CAN_READ) {
				_time_get(&SpaceTime);	//统计RF缓冲区满的时间
				if (_mutex_lock(&upload_buffer.mutex) != MQX_OK) {
					DEBUG_DIS(printf("\n system error, Mutex lock failed.\n"));
					dicor_get_logtime(&date);
					sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t Mutex lock出错\r\n", date.HOUR,date.MINUTE,date.SECOND);
					PrintLog(pDiCorLog->logbuf);
			
				}
				upload_buffer.data0.num = (UINT_16)upload_buffer.write_index;
				if ((upload_buffer.eth_st == ETH_CABLE_IP_CON) && (BreakPointInfo.Fileexist == 0))	//再加并且SD卡中没数据条件
				{
					if (FirstPackage) {		//增加刚连接时第一包数据可能出错的情况 
						FirstPackage = 0;
					}
					else {
						DiCorRunStatus.rf_status = 0;
						if (SERVER_MODE == 2)	
							error = send(upload_buffer.sock_mux, (int_8_ptr)(&upload_buffer.data0.type), (uint_32)(upload_buffer.write_index + DICOR_ID_LEN + 4),  0);   //向服务器发送数据
						error = send(upload_buffer.sock, (int_8_ptr)(&upload_buffer.data0.type), (uint_32)(upload_buffer.write_index + DICOR_ID_LEN + 4),  0);   //向服务器发送数据
						if (error == RTCS_ERROR) {
							shutdown(upload_buffer.sock, FLAG_ABORT_CONNECTION);
							DEBUG_DIS(printf("\n Error in send() with error code: %01x!", RTCS_geterror(upload_buffer.sock)));
							dicor_dis_timer();
							upload_buffer.eth_st = ETH_INIT;
							dicor_get_logtime(&date);
							EthernetLedOff();
							DimoLedOff();
							sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t 向DiMo发送数据失败，调用send出错，错误代码:%08X\r\n", date.HOUR,date.MINUTE,date.SECOND,RTCS_geterror(upload_buffer.sock));
							PrintLog(pDiCorLog->logbuf);
						} else {
							DEBUG_DIS(printf("\n"));
							 
						}
					}
				}
				else {
					if (BreakPointInfo.Fileexist == 0) {
						BreakPointInfo.Fileexist = 1;
						BreakPointInfo.fd_write_ptr = fopen("d:\\TempData\\TempData0000","a+");
						if (BreakPointInfo.fd_write_ptr == NULL) {
							printf("Error open file\r\n");
						}
						BreakPointInfo.fd_read_ptr = BreakPointInfo.fd_write_ptr;
						dicor_dis_timer();
						printf("can not connect DiMo, start save data in sd card\n");
						breakpointstart = 0;
						dicor_get_logtime(&date);
		
						sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t 连接DiMo失败，数据开始转存到SD卡中\r\n", date.HOUR,date.MINUTE,date.SECOND);
						PrintLog(pDiCorLog->logbuf);
			
					}
					printf("\r\nSave Data to SD Card\r\n");
					SaveData2SdCard(&upload_buffer.data0);
				}
				upload_buffer.state = CAN_WRITE;
				upload_buffer.write_index = 0;
				_mutex_unlock(&upload_buffer.mutex);
				dicor_rf_signal(NET_SEND_DATA_END);
			} 
			//如果有数据需要上传
			if (BreakPointInfo.Fileexist != 0) {
				//检查网络
				if (upload_buffer.eth_st == ETH_CABLE_IP_CON) { 
					if (!(ipcfg_get_link_active(0))) {
						upload_buffer.eth_st = ETH_INIT;
						shutdown(upload_buffer.sock,FLAG_ABORT_CONNECTION);
						DEBUG_DIS(printf("\n please plug cable")); 
						_time_delay(5);
					} else {
						if (breakpointstart == 0) {
							breakpointstart = 1;
							dicor_dis_timer();
							printf("Breakpoint Continuingly Start\n");	
							dicor_get_logtime(&date); 
							sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t 开始传送SD卡中的数据到DiMo，断点续传开始\r\n", date.HOUR,date.MINUTE,date.SECOND);
							PrintLog(pDiCorLog->logbuf); 
						}
						if (1 == BreakpointContinuingly(&Save_Data_Buff, &offset)) {
							//没有数据可发了
							offset = 0;
							bak_offset = 0;
							BreakPointInfo.Fileexist = 0;
							dicor_dis_timer();
							printf("Breakpoint Continuingly End\n");
							dicor_get_logtime(&date);
					
							sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t SD卡数据传送完毕，断点续传结束\r\n", date.HOUR,date.MINUTE,date.SECOND);
							PrintLog(pDiCorLog->logbuf);
					
						} else {
							if (SERVER_MODE == 2)	
								error = send(upload_buffer.sock_mux, (int_8_ptr)(&Save_Data_Buff.type), (uint_32)(Save_Data_Buff.num + DICOR_ID_LEN + 4 + 12), 0);
							error = send(upload_buffer.sock, (int_8_ptr)(&Save_Data_Buff.type), (uint_32)(Save_Data_Buff.num + DICOR_ID_LEN + 4 + 12), 0);
							if (error ==RTCS_ERROR) {
								//恢复，为下次重新发送做准备
								offset = bak_offset;
								fseek(BreakPointInfo.fd_read_ptr,offset,IO_SEEK_SET);
								shutdown(upload_buffer.sock,FLAG_ABORT_CONNECTION);
								DEBUG_DIS(printf("\n Error in send() with error code: %01x!", RTCS_geterror(upload_buffer.sock)));
								dicor_dis_timer();
								upload_buffer.eth_st = ETH_INIT;
								dicor_get_logtime(&date);
								sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t 向DiMo发送断点续传数据失败，调用send出错，错误代码:%08X\r\n", date.HOUR,date.MINUTE,date.SECOND,RTCS_geterror(upload_buffer.sock));
								PrintLog(pDiCorLog->logbuf);
							} else {
								bak_offset = offset;
								//DEBUG_DIS(printf("\n save to data center"));
							}
						}
					}
				} else {
					_time_delay(5);
				}
			}
			break;
		case RF_NWK_IDLE_NO_DIDI_WORKNG:

			break;
		case RF_NWK_IDLE_NO_LAN:
			NoRegTimes = 0;
			// _time_delay(1000*30);  // delay 30sec
#if DICOR_STATIC_REG_TAB
			//   _time_delay(1000*60*10);  // delay 10min
			;
#else
			_time_delay(1000 * 60 * 10);	// delay 10min
			NoRfTimes++;
			if (NoRfTimes > 2) {
				rfnwk_state = RF_NWK_DIS;
			}
			DEBUG_DIS(printf("\nNo RF network"));
#endif

			break;
		case RF_NWK_DIS:
#if (!DICOR_STATIC_REG_TAB)
			NoRfTimes = 0;
			NoRegTimes++;
			// _time_delay(1000*60*4);
			_time_delay(1000 * 60);
			if (NoRegTimes > 3) {
				rfnwk_state = RF_NWK_IDLE_NO_LAN;
			}
			RF_SetupNetwork();
#endif
			break;
		default:
			break;

		}
		//通知第三方设备任务数据已经被处理过了
		dicor_rf_signal(THIRD_DEVICE_DATA_START);
		_time_delay(1);
		Upload_Alive = 1;
	}
}




void upload_data_exit(void)
{
	DATE_STRUCT date;
	fclose(BreakPointInfo.fd_write_ptr);
	
	
//	dicor_kill_task("dicor_upload_data_task");
	
	dicor_get_logtime(&date);
	
	sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t 掉电或低电压关机\r\n", date.HOUR,date.MINUTE,date.SECOND);
	PrintLog(pDiCorLog->logbuf);
	//释放CPU资源
	_time_delay(800);
	//等待保存结束
	_lwsem_wait(&pDiCorLog->writesem);
	_lwsem_post(&pDiCorLog->writesem);
}



/* EOF */
