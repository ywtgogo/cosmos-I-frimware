/*************************************************************
* 
* Copyright (c) convertergy ;
* All Rights Reserved
* FileName: third_device_typetable.c
* Version : 
* Date    : 2012-04-12 
* ower    : Younger
*
* Comments:���е������豸�ĺ�������������
*
*
***************************************************************/

#include "dicor.h"
#include "third_device_def.h"
#include "thirddev_acrel_pz80.h"
#include "thirddev_dahua_dds979-96bde.h"
#include "thirddev_convertergy_dian.h"
#include "thirddev_gateway.h"

//���������豸��ע����غ����б�
Register_Third_Device Register_Third_Device_Table[] = 
{
	{DEV_TYPE_ELECTRICITY_METER, DEVICE_CODE_ACREL_PZ80, register_device_Acrel_PZ80},
	{DEV_TYPE_ELECTRICITY_METER, DEVICE_CODE_DAHUA_DDS879_96BDE, register_device_Dahua_DDS879_96BDE},
	{DEV_TYPE_ELECTRICITY_METER, DEVICE_CODE_GATEWAY,register_device_Gateway},
	{DEV_TYPE_INVERTER_DIAN, DEVICE_CODE_CONVERTERGY_DIAN, register_device_Convertergy_Dian},
	{0, 0, NULL},	
};

/* EOF */
