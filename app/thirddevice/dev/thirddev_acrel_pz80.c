
/**********************************************************************/ /*!
*
* @file thieddev_acel_pz80.c
*
* @author Younger
*
* @date 2012-04-11
*
* @version 0.0.0.0
*
* @brief ACREL PZ80系列电表的相关操作，包括注册设备，读取数据等
*
***************************************************************************/
#include <mqx.h>
#include <string.h>
#include "thirddev_acrel_pz80.h"
#include "third_device.h"
#include "rs485.h"
#include "modbus.h"
#include "dicor_upload.h"
//#include "logging_public.h"
#include "thirddevice_poll.h"
#include "third_device_def.h"


//从Acrel PZ80系列电表中读取数据
void Acrel_PZ80_ReadData(void *ptr)
{
	_mqx_int len;
	int_8 result;
	THIRD_DEVICE_DATA_ST* pThirdData;
	ACREL_PZ80_DATA* pAcrelData;
	uint_16 addr;
//	uint_16 crc;
	uint_8* data_buffer;
	uint_8* ready_buffer;
	//uchar ReadyBuffer[4];
	uchar buffer[5]=
	{0x80,0x01,0x43,
	 0x30,0x49	
	};
	rs485_write(buffer,5);
	_time_delay(1000);



//	ThirdDeviceWaitSendData();
//	len = rs485_read(data_buffer);
//	Modbus_SendReadyCommand(addr,ready_buffer);
//	_time_delay(1000);
	
		
/*	

	readybuffer[0]= addr;
	readybuffer[1]= 0x68;
	crc = Modbus_calcrc16(readybuffer, len);
	readybuffer[2] = (uint_8)crc;
	readybuffer[3] = (uint_8)(crc>>8);
   	rs485_write(readybuffer,4);
	_time_delay(1000);
*/	
	pThirdData = (THIRD_DEVICE_DATA_ST*)ptr;
	data_buffer = RS485_Data_Buffer;
	pAcrelData = (ACREL_PZ80_DATA*)pThirdData->pdata; 
	addr = pThirdData->device_id;
	
	ThirdDeviceWaitSendData();
	//ready_buffer = ReadyBuffer;

	
	//先清串口缓冲区的数据
	len = rs485_read(data_buffer);
		
	Modbus_SendReadRegCommand(addr, data_buffer, ACREL_PZ80_REGISTER_START_ADDR, ACREL_PZ80_REGISTER_READ_MAX_LENGHT);		//全部20个寄存器
	_time_delay(200);
	result = Modbus_RecvReadRegData(addr, data_buffer, (uint_8*)pAcrelData);
	
	if (result == 0)
	{	
		ThirdDevicPreDataFinish();
	}
}


//注册Acrel PZ80系列电表
//入口参数：addr:RS485总线地址 period:设备轮询周期，单位10ms
//出口参数：成功返回0，失败返回-1
int_8 register_device_Acrel_PZ80(uint_16 addr,uint_16 num_node, uint_32 period)
{
	ACREL_PZ80_DATA* pAcrelPZ80Data;
	THIRD_DEVICE_DATA_ST* pThirdDeviceData;
	thirddev_poll_list_entry_t* pPoll_list_AcrelPZ80;
	Third_Dev_Poll_Timer* pThird_Dev_Poll_Timer_AcrelPZ80;
	//申请第三方设备统一标准结构的存储空间
	pThirdDeviceData = register_third_device(addr,num_node);
	if (pThirdDeviceData == NULL)
	{
		return -1;
	}
	
	//为该设备申请一个定时器
	pThird_Dev_Poll_Timer_AcrelPZ80 = register_third_device_timer(period);
	if (pThird_Dev_Poll_Timer_AcrelPZ80 == NULL)
	{
		return -1;
	}
	//将该设备申请一个轮询结构的存储空间
	pPoll_list_AcrelPZ80 = register_third_device_poll(Acrel_PZ80_ReadData, (void*)pThirdDeviceData);
	if (pPoll_list_AcrelPZ80 == NULL)
	{
		return -1;
	}
	
	pAcrelPZ80Data = (ACREL_PZ80_DATA *) _mem_alloc_zero_from(_user_pool_id, sizeof(ACREL_PZ80_DATA));
	if (pAcrelPZ80Data == NULL)
	{
		printf("error when mem alloc pThirdDeviceData buf\r\n");	
		return -1;
	}
	memset(pAcrelPZ80Data, 0, sizeof(ACREL_PZ80_DATA));
	
	
	
	pThirdDeviceData->dev_type = DEV_TYPE_ELECTRICITY_METER;
	pThirdDeviceData->device_code = DEVICE_CODE_ACREL_PZ80;
	pThirdDeviceData->data_len = sizeof(ACREL_PZ80_DATA);
	pThirdDeviceData->pdata = (void*)pAcrelPZ80Data;
	
	//将该设备加入到轮询列表中
	InsetThirdDeviceToLink(pPoll_list_AcrelPZ80, pThird_Dev_Poll_Timer_AcrelPZ80);
	printf("register acrel pz80 device succeed\n");
	return 0;
}

