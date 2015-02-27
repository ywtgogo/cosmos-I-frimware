
/**********************************************************************/ /*!
*
* @file thieddev_acel_pz80.c
*
* @author younger
*
* @date 2013-04-11
*
* @version 0.0.0.0
*
* @brief gateway的相关操作，包括注册设备，读取数据等
*
***************************************************************************/
#include <mqx.h>
#include <string.h>
#include "thirddev_gateway.h"
#include "third_device.h"
#include "rs485.h"
#include "modbus.h"
#include "dicor_upload.h"
//#include "logging_public.h"
#include "thirddevice_poll.h"
#include "third_device_def.h"
#include  "stdlib.h"
#include  "network_nwk.h"
#include   "breakpoint.h"
#include  "dicor.h"
#include   "Watchdog.h"
#include "logging_public.h"
//从gateway中读取数据,//轮询问数据
void GATEWAY_ReadData(void *ptr)
{
	_mqx_int len=0;
	int_8 result;
	THIRD_DEVICE_DATA_ST* pThirdData;
	GATEWAY_DATA* pGatewayData;
	uint_16 addr1,addr2;
	uint_16 num_node;
	uint_8* data_buffer;
	EMN_SMC_DATA_ST* pSMCData;
	uint_32 error;
	EMN_DICOR_DATA_ST DiCorRunStatus;
	int_8 i=0;
	UINT_8 watchdogenable = 0;
	static TIME_STRUCT SpaceTime;
	DATE_STRUCT date;
	int_8 GatewayDataLen = sizeof(GATEWAY_DATA);
	int_8 EMN_SMC_DATA_ST_Len = sizeof(EMN_SMC_DATA_ST);
	
	pThirdData = (THIRD_DEVICE_DATA_ST*)ptr;
    data_buffer = RS485_Data_Buffer;
	pGatewayData = (GATEWAY_DATA*)pThirdData->pdata; 
	addr2 = pThirdData->device_id;
	addr1 = addr2;
	num_node=pThirdData->num_node;

	OPTIMUS_SYNC_Command(addr1,data_buffer);

	Modbus_SendStartCommand(addr1,data_buffer);

	Read_Data_Command( addr1,data_buffer,GATEWAY_REGISTER_START_ADDR, num_node);		//全部20个寄存器
	result = Modbus_RecvReadRegData(addr2, data_buffer, (uint_8*)pGatewayData);  		//pGatewayData的数据格式
	if (result != 0)		//若0x04命令返回错误，将不上传下面的51个数据到服务器，而是继续执行下一次轮询
	{
		printf("\n0x04接收错误\n");
		return ;	
	}
	

	pThirdData = (THIRD_DEVICE_DATA_ST*)ptr;
	data_buffer = RS485_Data_Buffer;
	addr2 = pThirdData->device_id;
	addr1 = addr2;
	pSMCData = (EMN_SMC_DATA_ST *)malloc(sizeof(EMN_SMC_DATA_ST));
 
    for(i=0;i<51;i++)
    {
		pGatewayData = (GATEWAY_DATA*)(data_buffer + 9+GatewayDataLen*i);
		pSMCData->frm_type = 0x10;
		pSMCData->len =0x16;
		pSMCData->dev_id =  pGatewayData->id;
		pSMCData->Vin = pGatewayData->Vout;
		pSMCData->Iout=pGatewayData->Iout ;
		pSMCData->Vout = pGatewayData->Vin;
		pSMCData->Temp = pGatewayData->Temp;
		pSMCData->out_power= pGatewayData->Power;
		pSMCData->d = pGatewayData->Err ;   //= 0x0000; 
		pSMCData->Energy =pGatewayData->Energy;  //= 0x0000;
		pSMCData->a=0x0000;
		pSMCData->b=0x0000;
 		memcpy((char *)(&(upload_buffer.data0.didi_data[ EMN_SMC_DATA_ST_Len * i])),pSMCData,EMN_SMC_DATA_ST_Len);   
     }
     
	free(pSMCData);
	upload_buffer.state = CAN_READ;
	EMN_ChildNum = EMN_DEV_NUM;
	rfnwk_state = RF_NWK_IDLE;	
	upload_buffer.write_index =22* 51; 
    //add 7.9
    	if (upload_buffer.eth_st == ETH_CABLE_IP_CON)
				{
					if (!(ipcfg_get_link_active(0))) {
						upload_buffer.eth_st = ETH_INIT;
						shutdown(upload_buffer.sock,
						     FLAG_ABORT_CONNECTION);
						DEBUG_DIS(printf("\n please plug cable"));
					}
					if (watchdogenable == 0)
					{
						watchdogenable = 1;
						_watchdog_start(60*60*1000);
					}
				}
				
				else
				{
					if (watchdogenable == 1)
					{
						watchdogenable = 0;
						_watchdog_stop();
					}
				}   

  	if (result == 0)
	{	
		ThirdDevicPreDataFinish();
	}
}


//注册gateway表
//入口参数：addr:RS485总线地址 period:设备轮询周期，单位10ms
//出口参数：成功返回0，失败返回-1
int_8 register_device_Gateway(uint_16 addr1,uint_16 num_node, uint_32 period)
{
	GATEWAY_DATA* pgatewayData;
	THIRD_DEVICE_DATA_ST* pThirdDeviceData;
	thirddev_poll_list_entry_t* pPoll_list_gateway;
	Third_Dev_Poll_Timer* pThird_Dev_Poll_Timer_gateway;
	//申请第三方设备统一标准结构的存储空间
	pThirdDeviceData = register_third_device(addr1,num_node);
	if (pThirdDeviceData == NULL)
	{
		return -1;
	}
	
	//为该设备申请一个定时器
	pThird_Dev_Poll_Timer_gateway = register_third_device_timer(period);
	if (pThird_Dev_Poll_Timer_gateway == NULL)
	{
		return -1;
	}
	//将该设备申请一个轮询结构的存储空间
	pPoll_list_gateway = register_third_device_poll(GATEWAY_ReadData, (void*)pThirdDeviceData);
	if (pPoll_list_gateway == NULL)
	{
		return -1;
	}
	
	pgatewayData = (GATEWAY_DATA *) _mem_alloc_zero_from(_user_pool_id, sizeof(GATEWAY_DATA));
	if (pgatewayData == NULL)
	{
		printf("error when mem alloc pThirdDeviceData buf\r\n");	
		return -1;
	}
	memset(pgatewayData, 0, sizeof(GATEWAY_DATA));
	
	
	
	pThirdDeviceData->dev_type = DEV_TYPE_ELECTRICITY_METER;
	pThirdDeviceData->device_code = DEVICE_CODE_GATEWAY;
	pThirdDeviceData->data_len = sizeof(GATEWAY_DATA);
	pThirdDeviceData->pdata = (void*)pgatewayData;
	
	//将该设备加入到轮询列表中
	InsetThirdDeviceToLink(pPoll_list_gateway, pThird_Dev_Poll_Timer_gateway);
	printf("register gateway device succeed\n");
	return 0;
}

