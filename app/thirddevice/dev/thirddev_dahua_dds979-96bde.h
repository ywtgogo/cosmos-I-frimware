
/**********************************************************************/ /*!
*
* @file thirddevice_dahua_dds979-96bde.h
*
* @author Younger
*
* @date 2012-04-17
*
* @version 0.0.0.0
*
* @brief DaHua DDS879-96BDE����������ݽṹ��Ҳ���������ݸ�;GSM-DE-A7VO-8-A1
*
***************************************************************************/

#ifndef _THIRDDEV_DAHUA_DDS979_96BDE_H_
#define _THIRDDEV_DAHUA_DDS979_96BDE_H_


typedef struct 
{
	int_32 voltage;		//��ѹֵ
	int_32 current;		//����ֵ
	int_32 power;		//����ֵ
	int_32 Ep;			//����ֵ
}DAHUA_DDS979_96BDE_DATA;

//˵����
//ʵ��һ�ε�ѹֵ = ��ѹ�Ĵ�������ֵ / 10, ��λΪV
//ʵ��һ�ε���ֵ = �����Ĵ�������ֵ / 1000, ��λΪA
//ʵ��һ�ι���ֵ = ���ʼĴ�������ֵ / 10000, ��λΪkW
//ʵ��һ�ε���ֵ = ���ܼĴ�������ֵ / 100, ��λΪWh��kWh(�������ʾ��λһ��)



//***************************************************************************
//***************************DaHua DDS879-96BDE�����صĽṹ�ͺ궨��***********

#define DAHUA_DDS979_96BDE_REGISTER_START_ADDR			20
#define DAHUA_DDS979_96BDE_REGISTER_READ_MAX_LENGHT		8



//***************************************************************************


void Dahua_DDS879_96BDE_ReadData(void *ptr);
int_8 register_device_Dahua_DDS879_96BDE(uint_16 addr, uint_16 num_node,uint_32 period);

#endif
