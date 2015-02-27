
/**********************************************************************/ /*!
*
* @file thieddev_acel_pz80.c
*
* @author Younger
*
* @date 2012-04-16
*
* @version 0.0.0.0
*
* @brief Convertergy DiAn相关操作，包括注册设备，读取数据等
*
***************************************************************************/
#include <mqx.h>
#include <string.h>
#include "thirddev_convertergy_dian.h"
#include "third_device.h"
#include "rs485.h"
#include "modbus.h"
#include "dicor_upload.h"
//#include "logging_public.h"
#include "thirddevice_poll.h"
#include "third_device_def.h"


//从Convertergy DiAn中读取数据
void Convertergy_Dian_ReadData(void *ptr)
{
	_mqx_int len;
	int_8 result;
	THIRD_DEVICE_DATA_ST* pThirdData;
	CONVERTERGY_DIAN_DATA* pDianData;
	uint_16 addr;
	uint_8* data_buffer;
	
	pThirdData = (THIRD_DEVICE_DATA_ST*)ptr;
	data_buffer = RS485_Data_Buffer;
	pDianData = (CONVERTERGY_DIAN_DATA*)pThirdData->pdata; 
	addr = pThirdData->device_id;
	
	ThirdDeviceWaitSendData();
	
	//先清串口缓冲区的数据
	len = rs485_read(data_buffer);
		
	Modbus_SendReadRegCommand(addr, data_buffer, 0, 20);		//全部20个寄存器
	_time_delay(200);
	result = Modbus_RecvReadRegData(addr, data_buffer, (uint_8*)pDianData);
	
	if (result == 0)
	{	
		ThirdDevicPreDataFinish();
	}
}


//注册Convertergy DiAn
//入口参数：addr:RS485总线地址 period:设备轮询周期，单位10ms
//出口参数：成功返回0，失败返回-1
int_8 register_device_Convertergy_Dian(uint_16 addr,uint_16 num_node, uint_32 period)
{
	CONVERTERGY_DIAN_DATA* pConvertergyDianData;
	THIRD_DEVICE_DATA_ST* pThirdDeviceData;
	thirddev_poll_list_entry_t* pPoll_list_ConvertergyDian;
	Third_Dev_Poll_Timer* pThird_Dev_Poll_Timer_ConvertergyDian;
	//申请第三方设备统一标准结构的存储空间
	pThirdDeviceData = register_third_device(addr,num_node);
	if (pThirdDeviceData == NULL)
	{
		return -1;
	}
	
	//为该设备申请一个定时器
	pThird_Dev_Poll_Timer_ConvertergyDian = register_third_device_timer(period);
	if (pThird_Dev_Poll_Timer_ConvertergyDian == NULL)
	{
		return -1;
	}
	//将该设备申请一个轮询结构的存储空间
	pPoll_list_ConvertergyDian = register_third_device_poll(Convertergy_Dian_ReadData, (void*)pThirdDeviceData);
	if (pPoll_list_ConvertergyDian == NULL)
	{
		return -1;
	}
	
	pConvertergyDianData = (CONVERTERGY_DIAN_DATA *) _mem_alloc_zero_from(_user_pool_id, sizeof(CONVERTERGY_DIAN_DATA));
	if (pConvertergyDianData == NULL)
	{
		printf("error when mem alloc pThirdDeviceData buf\r\n");	
		return -1;
	}
	memset(pConvertergyDianData, 0, sizeof(CONVERTERGY_DIAN_DATA));
	
	
	
	pThirdDeviceData->dev_type = DEV_TYPE_INVERTER_DIAN;
	pThirdDeviceData->device_code = DEVICE_CODE_CONVERTERGY_DIAN;
	pThirdDeviceData->data_len = sizeof(CONVERTERGY_DIAN_DATA);
	pThirdDeviceData->pdata = (void*)pConvertergyDianData;
	
	//将该设备加入到轮询列表中
	InsetThirdDeviceToLink(pPoll_list_ConvertergyDian, pThird_Dev_Poll_Timer_ConvertergyDian);
	printf("register acrel pz80 device succeed\n");
	return 0;
}

