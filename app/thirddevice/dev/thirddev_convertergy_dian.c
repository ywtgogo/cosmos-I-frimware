
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
* @brief Convertergy DiAn��ز���������ע���豸����ȡ���ݵ�
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


//��Convertergy DiAn�ж�ȡ����
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
	
	//���崮�ڻ�����������
	len = rs485_read(data_buffer);
		
	Modbus_SendReadRegCommand(addr, data_buffer, 0, 20);		//ȫ��20���Ĵ���
	_time_delay(200);
	result = Modbus_RecvReadRegData(addr, data_buffer, (uint_8*)pDianData);
	
	if (result == 0)
	{	
		ThirdDevicPreDataFinish();
	}
}


//ע��Convertergy DiAn
//��ڲ�����addr:RS485���ߵ�ַ period:�豸��ѯ���ڣ���λ10ms
//���ڲ������ɹ�����0��ʧ�ܷ���-1
int_8 register_device_Convertergy_Dian(uint_16 addr,uint_16 num_node, uint_32 period)
{
	CONVERTERGY_DIAN_DATA* pConvertergyDianData;
	THIRD_DEVICE_DATA_ST* pThirdDeviceData;
	thirddev_poll_list_entry_t* pPoll_list_ConvertergyDian;
	Third_Dev_Poll_Timer* pThird_Dev_Poll_Timer_ConvertergyDian;
	//����������豸ͳһ��׼�ṹ�Ĵ洢�ռ�
	pThirdDeviceData = register_third_device(addr,num_node);
	if (pThirdDeviceData == NULL)
	{
		return -1;
	}
	
	//Ϊ���豸����һ����ʱ��
	pThird_Dev_Poll_Timer_ConvertergyDian = register_third_device_timer(period);
	if (pThird_Dev_Poll_Timer_ConvertergyDian == NULL)
	{
		return -1;
	}
	//�����豸����һ����ѯ�ṹ�Ĵ洢�ռ�
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
	
	//�����豸���뵽��ѯ�б���
	InsetThirdDeviceToLink(pPoll_list_ConvertergyDian, pThird_Dev_Poll_Timer_ConvertergyDian);
	printf("register acrel pz80 device succeed\n");
	return 0;
}

