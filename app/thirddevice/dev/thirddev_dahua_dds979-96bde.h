
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
* @brief DaHua DDS879-96BDE电表的相关数据结构，也适用于苏州高途GSM-DE-A7VO-8-A1
*
***************************************************************************/

#ifndef _THIRDDEV_DAHUA_DDS979_96BDE_H_
#define _THIRDDEV_DAHUA_DDS979_96BDE_H_


typedef struct 
{
	int_32 voltage;		//电压值
	int_32 current;		//电流值
	int_32 power;		//功率值
	int_32 Ep;			//电能值
}DAHUA_DDS979_96BDE_DATA;

//说明：
//实际一次电压值 = 电压寄存器读出值 / 10, 单位为V
//实际一次电流值 = 电流寄存器读出值 / 1000, 单位为A
//实际一次功率值 = 功率寄存器读出值 / 10000, 单位为kW
//实际一次电能值 = 电能寄存器读出值 / 100, 单位为Wh或kWh(与面板显示单位一致)



//***************************************************************************
//***************************DaHua DDS879-96BDE电表相关的结构和宏定义***********

#define DAHUA_DDS979_96BDE_REGISTER_START_ADDR			20
#define DAHUA_DDS979_96BDE_REGISTER_READ_MAX_LENGHT		8



//***************************************************************************


void Dahua_DDS879_96BDE_ReadData(void *ptr);
int_8 register_device_Dahua_DDS879_96BDE(uint_16 addr, uint_16 num_node,uint_32 period);

#endif
