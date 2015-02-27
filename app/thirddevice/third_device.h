#ifndef __THIRD_DEVICE_H__
#define __THIRD_DEVICE_H__
/**********************************************************************
* 
* Copyright (c) 2010 convertergy ;
* All Rights Reserved
* FileName: third_device.h 
* Version : 
* Date    : 2012-3-31
* ower    : Alex
* Comments:
*
*   第三方设备数据结构
*
*************************************************************************/


#define TYPE_THIRD_DEVICE_DATA		 		0x50

//第三方设备与DiMo之间的接口定义
typedef struct 
{
	uint_8 type;				//类型
	uint_8 teminal_id[4];	//DiCor UID 对于我们来说是DiCor
	uint_8 dev_type;			//设备类型，1-电表，2-天气信息....gateway
	uint_8 device_code;	        //设备码
	uint_16 device_id;			//设备地址编号 范围1-254
	uint_16 num_node;				
	uint_16 data_len;			//第三方设备数据的长度
	uint_8 dump[2];
	void* pdata;			  
}THIRD_DEVICE_DATA_ST;


typedef struct
{
	uint_8 dev_type;
	uint_8 dev_code;
	uint_16 dev_id;
	uint_16 num_node;
	uint_32 period;
}THIRD_DEVICE_SDCARD_INFO;

typedef struct
{
	uint_8 count;
	uint_8 order;
	uint_8 dev_type;
	uint_8 dev_code;
	uint_16 dev_id;
	uint_16 num_node;
	uint_32 period;		
}THIRD_DEVICE_EEPROM_INFO;
//ThirdDevSdcardInfo或直接存入eeprom，取出来后验证


void ThirdDevicPreDataFinish(void);
void ThirdDeviceWaitSendData(void);
THIRD_DEVICE_DATA_ST* register_third_device(uint_16 addr1,uint_16 num_node);
void  dicor_third_device_task(uint_32 initial_data);
#endif

/* EOF */
