
/**********************************************************************/ /*!
*
* @file thirddev_dahua_dds979-96bde.c
*
* @author Younger
*
* @date 2012-04-16
*
* @version 0.0.0.0
*
* @brief DaHua DDS879-96BDE电表的相关操作，包括注册设备，读取数据等
* @也适用于苏州高途GSM-DE-A7VO-8-A1电表
***************************************************************************/
#include <mqx.h>
#include <string.h>
#include "thirddev_dahua_dds979-96bde.h"
#include "third_device.h"
#include "rs485.h"
#include "modbus.h"
#include "dicor_upload.h"
//#include "logging_public.h"
#include "thirddevice_poll.h"
#include "third_device_def.h"


//从Dahua DDS879_96BDE电表中读取数据
void Dahua_DDS879_96BDE_ReadData(void *ptr)
{
	_mqx_int len;
	int_8 result;
	THIRD_DEVICE_DATA_ST* pThirdData;
	DAHUA_DDS979_96BDE_DATA* pDDS979Data;
	uint_8 addr;
	uint_8* data_buffer;
	
   	uchar buffer[4]=
	{0x00,0x43,
	 0x40,0x41	
	};
	rs485_write(buffer,4);
	_time_delay(1000);




	pThirdData = (THIRD_DEVICE_DATA_ST*)ptr;
	data_buffer = RS485_Data_Buffer;
	pDDS979Data = (DAHUA_DDS979_96BDE_DATA*)pThirdData->pdata; 
	addr = pThirdData->device_id;
	
//	uint_8* ready_buffer;
	//uchar ReadyBuffer[4];

	
	ThirdDeviceWaitSendData();
	
	//先清串口缓冲区的数据
	len = rs485_read(data_buffer);
		
	Modbus_SendReadRegCommand(addr, data_buffer, DAHUA_DDS979_96BDE_REGISTER_START_ADDR, DAHUA_DDS979_96BDE_REGISTER_READ_MAX_LENGHT);		//前8个寄存器
	_time_delay(200);
	result = Modbus_RecvReadRegData(addr, data_buffer, (uint_8*)pDDS979Data);
	
	if (result == 0)
	{	
		ThirdDevicPreDataFinish();
	}
}


//注册Dahua DDS879_96BDE电表
//入口参数：addr:RS485总线地址 period:设备轮询周期，单位10ms
//出口参数：成功返回0，失败返回-1
int_8 register_device_Dahua_DDS879_96BDE(uint_16 addr,uint_16 num_node, uint_32 period)
{
	DAHUA_DDS979_96BDE_DATA* pDDSData;
	THIRD_DEVICE_DATA_ST* pThirdDeviceData;
	thirddev_poll_list_entry_t* pPoll_list_DDS;
	Third_Dev_Poll_Timer* pThird_Dev_Poll_Timer_DDS;
	//申请第三方设备统一标准结构的存储空间
	pThirdDeviceData = register_third_device(addr,num_node);
	if (pThirdDeviceData == NULL)
	{
		return -1;
	}
	
	//为该设备申请一个定时器
	pThird_Dev_Poll_Timer_DDS = register_third_device_timer(period);
	if (pThird_Dev_Poll_Timer_DDS == NULL)
	{
		return -1;
	}
	//将该设备申请一个轮询结构的存储空间
	pPoll_list_DDS = register_third_device_poll(Dahua_DDS879_96BDE_ReadData, (void*)pThirdDeviceData);
	if (pPoll_list_DDS == NULL)
	{
		return -1;
	}
	
	pDDSData = (DAHUA_DDS979_96BDE_DATA *) _mem_alloc_zero_from(_user_pool_id, sizeof(DAHUA_DDS979_96BDE_DATA));
	if (pDDSData == NULL)
	{
		printf("error when mem alloc pThirdDeviceData buf\r\n");	
		return -1;
	}
	memset(pDDSData, 0, sizeof(DAHUA_DDS979_96BDE_DATA));
	
	
	
	pThirdDeviceData->dev_type = DEV_TYPE_ELECTRICITY_METER;
	pThirdDeviceData->device_code = DEVICE_CODE_DAHUA_DDS879_96BDE;
	pThirdDeviceData->data_len = sizeof(DAHUA_DDS979_96BDE_DATA);
	pThirdDeviceData->pdata = (void*)pDDSData;
	
	//将该设备加入到轮询列表中
	InsetThirdDeviceToLink(pPoll_list_DDS, pThird_Dev_Poll_Timer_DDS);
	printf("register MODE-STATE device succeed\n");
	return 0;
}

