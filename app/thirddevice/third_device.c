/*************************************************************
* 
* Copyright (c) convertergy ;
* All Rights Reserved
* FileName: third_device.c
* Version : 
* Date    : 2012-04-11 
* ower    : Younger
*
* Comments:第三方设备的统一接口程序
*
*
***************************************************************/

#include <string.h>
#include <stdlib.h>
#include "dicor.h"
#include "third_device.h"
#include "dicor_upload.h"
#include "thirddevice_poll.h"
#include "rs485.h"
#include "thirddev_acrel_pz80.h"
#include "thirddev_gateway.h"
#include "utility.h"
#include "third_device_def.h"
#include <watchdog.h>


#define THIRDDEV_CFG_POLL_MAX	128

THIRD_DEVICE_DATA_ST* g_pThirdDeviceData;
extern _mem_pool_id _user_pool_id;


//int_8 Collect_Acrel_PZ80_All_Data(uint_8 addr, uint_8* data_buffer, ACREL_PZ80_DATA* pData);

//extern LWSEM_STRUCT ThirdDevDataSem;
//extern uint_8 ThirdDevDataFlag;
extern void dicor_rf_signal(uint_32 flag);
extern LWEVENT_STRUCT RF_NET_event;


 //uchar buffer[4] =
//	  {0x81,0x42,
//	  0xE1,0xD1};
//等待upload任务将数据处理掉
void ThirdDeviceWaitSendData(void)
{
	_lwevent_wait_ticks(&RF_NET_event, THIRD_DEVICE_DATA_START, TRUE, 0);
	_lwevent_clear(&RF_NET_event, THIRD_DEVICE_DATA_START);	
	_lwevent_clear(&RF_NET_event, THIRD_DEVICE_DATA_END);
}


//准备数据发给DiMo
void ThirdDevicPreDataFinish(void)
{
	//发送信号量
	dicor_rf_signal(THIRD_DEVICE_DATA_END);
}


//注册第三方设备
THIRD_DEVICE_DATA_ST* register_third_device(uint_16 addr1,uint_16 num_node)
{
	THIRD_DEVICE_DATA_ST* pThirdDeviceData;

	
	pThirdDeviceData = (THIRD_DEVICE_DATA_ST *) _mem_alloc_zero_from(_user_pool_id, sizeof(THIRD_DEVICE_DATA_ST));
	if (pThirdDeviceData == NULL)
	{
		printf("error when mem alloc pThirdDeviceData buf\r\n");	
		return NULL;
	}
	memset(pThirdDeviceData, 0, sizeof(THIRD_DEVICE_DATA_ST));
	
	pThirdDeviceData->type = TYPE_THIRD_DEVICE_DATA;
	pThirdDeviceData->teminal_id[0] = pBaseConfig->uid[0];
    pThirdDeviceData->teminal_id[1] = pBaseConfig->uid[1];
	pThirdDeviceData->teminal_id[2] = pBaseConfig->uid[2];
	pThirdDeviceData->teminal_id[3] = pBaseConfig->uid[3];
	pThirdDeviceData->device_id = addr1;
	pThirdDeviceData->num_node = num_node;
	return pThirdDeviceData;
	
}

//将SD卡中的所有第三方设备注册一下
static int_8 register_all_third_device(THIRD_DEVICE_SDCARD_INFO* pInfo, int_8 count)
{
	int_8 i;
	THIRD_DEVICE_SDCARD_INFO* p;
	Register_Third_Device* pRegDev;
	
	p = pInfo;
	for (i=0; i<count; i++)
	{
		//信息匹配，并调用函数
		pRegDev = Register_Third_Device_Table;
		while (pRegDev->dev_type != 0)
		{
			if (pRegDev->dev_type==p->dev_type && pRegDev->dev_code==p->dev_code)
			{
				break;
			}
			pRegDev++;
		}
		if (pRegDev->register_func != NULL)
		{
		
			if (pRegDev->register_func(p->dev_id, p->num_node,p->period) == 0)
			{
				printf("register third device addr=[%d] succeed\n", i);
			}
			else
			{
				printf("register third device addr=[%d] fail\n", i);
				return -1;
			}
		}
		else
		{
			printf("没有找到设备相匹配的注册函数\n", i);
			return -1;
		}
		p++;
		
	}
	return 0;
}

//解析第三方设备的配置文本文件
static int_8 AnalysisThirdDevConfigFile(THIRD_DEVICE_SDCARD_INFO* pInfo)
{
	char s[100];
	char t[40];
	FILE_PTR fd_ptr;
	char hex[3];
	uchar dev_type;
	uchar dev_code;
	uint_16 dev_id; 
	uint_16 num_node;
	uint_8  dev_id_h,dev_id_l;
	uint_8  num_node_h,num_node_l;
	uint_32 period;
	uchar count = 0;
	uchar end = 0;
	THIRD_DEVICE_SDCARD_INFO* p;
	p = pInfo;
	hex[2] = '\0';

	//先打开配置文件
	fd_ptr = fopen("d:\\dicorconfig\\thirddevicetable.txt", "r");
	if (fd_ptr == NULL) 
	{
		printf("第三方设备文件(thirddevicetable.txt)不存在\n");
		return -1;
	}
	printf("\n*******************************\n");
	printf("解析文件 thirddevicetable.txt\n");
	while (strcmp(s, "[BEGIN]") != 0) 
	{
		fgetline(fd_ptr, s, 90);	//[BEGIN]
		if (feof(fd_ptr))
		{
			printf("\n");
			printf("没有找到文件头[BEGIN]，请用ls,dir,type等命令检查文件是否正确！\n");
			fclose(fd_ptr);
			return -1;
		}
	}
	
	while (strcmp(s, "[END") != 0)
	{
		while (strcmp(s, "[DEV]") != 0)
		{
			if (strcmp(s, "[END]") == 0)
			{
				end = 1;
				break;
			}
			fgetline(fd_ptr, s, 90);	//
			if (feof(fd_ptr))
			{
				printf("\n");
				printf("没有找到[END]，请用ls,dir,type等命令检查文件书写格式是否正确！\n");
				fclose(fd_ptr);
				return -1;
			}
		}
		if (end == 1)
		{
			break;
		}
		fgetline(fd_ptr, s, 90);	//
		fgetline(fd_ptr, s, 90);
		printf("%s\n", s);
		if (cutstr(s, t, '='))
		{
			printf("文本数据书写格式错误\n");
			fclose(fd_ptr);
			return -1;
		}
		hex[0] = t[2];
		hex[1] = t[3];
		strupr(hex);
		dev_type = ahextoi(hex);
		fgetline(fd_ptr, s, 90);	//
		fgetline(fd_ptr, s, 90);
		printf("%s\n", s);
		if (cutstr(s, t, '='))
		{
			printf("文本数据书写格式错误\n");
			fclose(fd_ptr);
			return -1;
		}
		hex[0] = t[2];
		hex[1] = t[3];
		strupr(hex);
		dev_code = ahextoi(hex);
		fgetline(fd_ptr, s, 90);	//
		fgetline(fd_ptr, s, 90);
		printf("%s\n", s);
		if (cutstr(s, t, '='))
		{
			printf("文本数据书写格式错误\n");
			fclose(fd_ptr);
			return -1;
		}
		hex[0] = t[2];
		hex[1] = t[3];
		strupr(hex);
		dev_id_h = ahextoi(hex);
		
		
		
		hex[0] = t[4];
		hex[1] = t[5];
			strupr(hex);
		dev_id_l = ahextoi(hex);
	
		dev_id = (uint_16)dev_id_h;
     
        dev_id = dev_id << 8;
		dev_id = dev_id & 0xFF00;
		dev_id|= dev_id_l;
       // temp1 = temp1 & 0xFF00;
      //  temp1|= data_buffer[12+GatewayDataLen*i];
	
	    
	    
	    fgetline(fd_ptr, s, 90);	//
		fgetline(fd_ptr, s, 90);
		printf("%s\n", s);
		if (cutstr(s, t, '='))
		{
			printf("文本数据书写格式错误\n");
			fclose(fd_ptr);
			return -1;
		}
		hex[0] = t[2];
		hex[1] = t[3];
		strupr(hex);
		num_node_h = ahextoi(hex);
		
		hex[0] = t[4];
		hex[1] = t[5];
			strupr(hex);
		num_node_l = ahextoi(hex);
		
		num_node = (uint_16)num_node_h;
     
        num_node = num_node << 8;
		num_node = num_node & 0xFF00;
		num_node|= num_node_l;
	
	
	
		fgetline(fd_ptr, s, 90);	//
		fgetline(fd_ptr, s, 90);
		printf("%s\n", s);
		if (cutstr(s, t, '='))
		{
			printf("文本数据书写格式错误\n");
			fclose(fd_ptr);
			return -1;
		}
		period = atoi(t);
		p->dev_type = dev_type;
		p->dev_code = dev_code;
		p->dev_id = dev_id;
		p->num_node = num_node;
		p->period = period;
		count++;
		p++;
		if (count >= THIRDDEV_CFG_POLL_MAX)
		{
			printf("设备个数超过最大数量，最大允许%d个\n", THIRDDEV_CFG_POLL_MAX);
			fclose(fd_ptr);
			return -1;
		}
	}
	
			
	fclose(fd_ptr);	
	

	return count;
}

//从SD卡中加载第三方设备信息
static int_8 LoadThirdDevFormSDCard(void)
{
	int_8 count;
	THIRD_DEVICE_SDCARD_INFO* ThirdDevSdcardInfo;
	
	Init_ThirdDevLinkNode();
	ThirdDevSdcardInfo = (THIRD_DEVICE_SDCARD_INFO *) _mem_alloc_zero_from(_user_pool_id, sizeof(THIRD_DEVICE_SDCARD_INFO)*THIRDDEV_CFG_POLL_MAX);
	if (ThirdDevSdcardInfo == NULL)
	{
		printf("error when mem alloc ThirdDevSdcardInfo buf\r\n");	
		return -1;
	}
	memset(ThirdDevSdcardInfo, 0, sizeof(THIRD_DEVICE_SDCARD_INFO));
	
	count = AnalysisThirdDevConfigFile(ThirdDevSdcardInfo);
	if (count == -1)
	{
		printf("解析第三方设备的配置文本文件时发送错误\n");
		_mem_free(ThirdDevSdcardInfo);
		return -1;
	}
	else
	{
		printf("成功解析！找到设备个数%d\n", count);
	}
	if (register_all_third_device(ThirdDevSdcardInfo, count) == 0)
	{
		printf("注册所有的第三方设备成功\n");
	}
	else
	{
		printf("注册所有的第三方设备时发送错误\n");
	}
	
	_mem_free(ThirdDevSdcardInfo);
	return 0;
}



void  dicor_third_device_task(uint_32 initial_data)
{
	uint_16 addr1 = 0x0001;
	uint_8 count = 0;
	
	//_time_delay(1000*30);	
	//printf ("\n---third task ...---\n");
	
	if (_watchdog_create_component(BSP_TIMER_INTERRUPT_VECTOR,Wacthdog_Error)!= MQX_OK) 	
	{ 
		printf("third device task create watchdog failed !");
	}
	
	_watchdog_start(60*60*1000);
	

	rs485_init();
	_time_delay(1000);

	if (LoadThirdDevFormSDCard() == 0)
	{
		printf("从SD卡中加载第三方设备信息成功\n");
	}
	else
	{
		printf("从SD卡中加载第三方设备信息失败\n");
		while (1)
		{
			_time_delay(1000);
			_watchdog_start(60*60*1000);
		}
	}
	while (1)
	{
		Third_Device_Timer_Inc();		
		Third_Device_Poll();		
		_time_delay(10);
		_watchdog_start(60*60*1000);
	}
}

/* EOF */
