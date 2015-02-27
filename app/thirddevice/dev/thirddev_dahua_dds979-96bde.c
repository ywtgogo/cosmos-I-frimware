
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
* @brief DaHua DDS879-96BDE������ز���������ע���豸����ȡ���ݵ�
* @Ҳ���������ݸ�;GSM-DE-A7VO-8-A1���
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


//��Dahua DDS879_96BDE����ж�ȡ����
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
	
	//���崮�ڻ�����������
	len = rs485_read(data_buffer);
		
	Modbus_SendReadRegCommand(addr, data_buffer, DAHUA_DDS979_96BDE_REGISTER_START_ADDR, DAHUA_DDS979_96BDE_REGISTER_READ_MAX_LENGHT);		//ǰ8���Ĵ���
	_time_delay(200);
	result = Modbus_RecvReadRegData(addr, data_buffer, (uint_8*)pDDS979Data);
	
	if (result == 0)
	{	
		ThirdDevicPreDataFinish();
	}
}


//ע��Dahua DDS879_96BDE���
//��ڲ�����addr:RS485���ߵ�ַ period:�豸��ѯ���ڣ���λ10ms
//���ڲ������ɹ�����0��ʧ�ܷ���-1
int_8 register_device_Dahua_DDS879_96BDE(uint_16 addr,uint_16 num_node, uint_32 period)
{
	DAHUA_DDS979_96BDE_DATA* pDDSData;
	THIRD_DEVICE_DATA_ST* pThirdDeviceData;
	thirddev_poll_list_entry_t* pPoll_list_DDS;
	Third_Dev_Poll_Timer* pThird_Dev_Poll_Timer_DDS;
	//����������豸ͳһ��׼�ṹ�Ĵ洢�ռ�
	pThirdDeviceData = register_third_device(addr,num_node);
	if (pThirdDeviceData == NULL)
	{
		return -1;
	}
	
	//Ϊ���豸����һ����ʱ��
	pThird_Dev_Poll_Timer_DDS = register_third_device_timer(period);
	if (pThird_Dev_Poll_Timer_DDS == NULL)
	{
		return -1;
	}
	//�����豸����һ����ѯ�ṹ�Ĵ洢�ռ�
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
	
	//�����豸���뵽��ѯ�б���
	InsetThirdDeviceToLink(pPoll_list_DDS, pThird_Dev_Poll_Timer_DDS);
	printf("register MODE-STATE device succeed\n");
	return 0;
}

