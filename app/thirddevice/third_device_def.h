#ifndef __THIRD_DEVICE_DEF_H__
#define __THIRD_DEVICE_DEF_H__
/**********************************************************************

*
*   
*
*************************************************************************/


//dev_type 设备类型
#define	DEV_TYPE_ELECTRICITY_METER  		0x01
#define DEV_TYPE_INVERTER_DIAN				0x02
//#define DEV_TYPE_INVERTER_DIAN				0x03





//device_code 设备码
#define DEVICE_CODE_ACREL_PZ80				0x10
#define DEVICE_CODE_DAHUA_DDS879_96BDE		0x11
#define DEVICE_CODE_GATEWAY                 0x12
#define DEVICE_CODE_CONVERTERGY_DIAN		0x20











typedef int_8 (* register_third_device_func)(uint_16 addr,uint_16 num_node, uint_32 period);


typedef struct
{
	uint_8 dev_type;
	uint_8 dev_code;
    register_third_device_func register_func;
}Register_Third_Device;


extern Register_Third_Device Register_Third_Device_Table[];
#endif

/* EOF */
