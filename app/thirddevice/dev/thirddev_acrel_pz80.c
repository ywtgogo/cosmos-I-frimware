
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
* @brief ACREL PZ80ϵ�е�����ز���������ע���豸����ȡ���ݵ�
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


//��Acrel PZ80ϵ�е���ж�ȡ����
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

	
	//���崮�ڻ�����������
	len = rs485_read(data_buffer);
		
	Modbus_SendReadRegCommand(addr, data_buffer, ACREL_PZ80_REGISTER_START_ADDR, ACREL_PZ80_REGISTER_READ_MAX_LENGHT);		//ȫ��20���Ĵ���
	_time_delay(200);
	result = Modbus_RecvReadRegData(addr, data_buffer, (uint_8*)pAcrelData);
	
	if (result == 0)
	{	
		ThirdDevicPreDataFinish();
	}
}


//ע��Acrel PZ80ϵ�е��
//��ڲ�����addr:RS485���ߵ�ַ period:�豸��ѯ���ڣ���λ10ms
//���ڲ������ɹ�����0��ʧ�ܷ���-1
int_8 register_device_Acrel_PZ80(uint_16 addr,uint_16 num_node, uint_32 period)
{
	ACREL_PZ80_DATA* pAcrelPZ80Data;
	THIRD_DEVICE_DATA_ST* pThirdDeviceData;
	thirddev_poll_list_entry_t* pPoll_list_AcrelPZ80;
	Third_Dev_Poll_Timer* pThird_Dev_Poll_Timer_AcrelPZ80;
	//����������豸ͳһ��׼�ṹ�Ĵ洢�ռ�
	pThirdDeviceData = register_third_device(addr,num_node);
	if (pThirdDeviceData == NULL)
	{
		return -1;
	}
	
	//Ϊ���豸����һ����ʱ��
	pThird_Dev_Poll_Timer_AcrelPZ80 = register_third_device_timer(period);
	if (pThird_Dev_Poll_Timer_AcrelPZ80 == NULL)
	{
		return -1;
	}
	//�����豸����һ����ѯ�ṹ�Ĵ洢�ռ�
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
	
	//�����豸���뵽��ѯ�б���
	InsetThirdDeviceToLink(pPoll_list_AcrelPZ80, pThird_Dev_Poll_Timer_AcrelPZ80);
	printf("register acrel pz80 device succeed\n");
	return 0;
}

