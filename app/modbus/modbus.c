/*************************************************************
* 
* Copyright (c) convertergy ;
* All Rights Reserved
* FileName: modbus.c
* Version : 
* Date    : 2012/04/12
* ower    : Younger
*
* Comments:Modbus常用的几个操作命令及相关函数
*
*
***************************************************************/
#include <string.h>
#include "dicor.h"
#include "rs485.h"
#include "modbus.h"
#include "logging_public.h"
#include "thirddev_gateway.h"
#include "dicor_update.h"
#include "dicor_upload.h"
#include "Watchdog.h"
#include "third_device.h"
#include "modbus.h"
#include "led.h"
//#include "logging_public.h"
#include "thirddevice_poll.h"
#include "third_device_def.h"
#include  "stdlib.h"
#include  "mqx.h"
//static MCF5225_GPIO_STRUCT_PTR led_gpio_ptr;
//Modbus用到的CRC校验
//static
//#define UART1_CHANNEL "ittyb:"
//extern MQX_FILE_PTR uart1_dev ;
//#define RS485_CHANNEL "ittyb:"

//MQX_FILE_PTR rs485_dev = NULL;
extern uchar CoodAddr;
extern void EepromWriteCoodaddr(unsigned char *);
extern UINT_8 SERVER_MODE;
void dicor_dis_timer(void);

 unsigned short Modbus_calcrc16(unsigned char *buf, unsigned short len)
{
	unsigned char i = 0;
    unsigned short crc = 0xFFFF;
	unsigned short tlen = len;

    while (tlen--)
    {
		crc ^=  (unsigned short)(*buf++ & 0xFFFF);

        i = 8;
        do
        {
            if (crc & 0x0001)
            {
                crc = crc >> 1 ^ 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
		while(--i);
    }
    return crc;
}

void Read_Data_Command(uint_16 addr1,uint_8* data_buffer,uint_16 readstart, uint_16 readlen)
{
		uint_16 crc;
		uint_8 temp_Buf[150],i;
		extern void dicor_dis_timer(void);
	
		temp_Buf[0]=0x80;
     	temp_Buf[1] = CoodAddr;
		temp_Buf[2] = MODBUS_FUNCTION_READ_REGISTER;//0x04
		temp_Buf[3] =(uint_8)(addr1>>8);
        temp_Buf[4] =(uint_8)(addr1);
		temp_Buf[5] =(uint_8)(readstart>>8);
        temp_Buf[6] =(uint_8)(readstart);
        temp_Buf[7] =(uint_8)(readlen>>8);
        temp_Buf[8] =(uint_8)(readlen);
		crc = Modbus_calcrc16(temp_Buf, 9);
    	temp_Buf[9] = (uint_8)crc;
	    temp_Buf[10] = (uint_8)(crc>>8);

		dicor_dis_timer();
		printf("\n*********************************************************");
        printf("\nSend Magnus Read CMD( FuncCode:04) to device[0x%04x]",addr1);
        printf("\nWaiting the response of the device[0x%04x]",addr1);
     	printf("\nREQ :");
		for (i = 0; i < 11; i++)
		{
			printf("%02x ",temp_Buf[i] );
		}	        
		rs485_clear_buf();
		rs485_write(temp_Buf, 11);
		Dimo_ht_data.Dicor_UID[0]= pBaseConfig->uid[0];
		Dimo_ht_data.Dicor_UID[1]= pBaseConfig->uid[1];
		Dimo_ht_data.Dicor_UID[2]= pBaseConfig->uid[2];
		Dimo_ht_data.Dicor_UID[3]= pBaseConfig->uid[3];
		Dimo_ht_data.pack_obj=Dimo_response_data.pack_obj;
		Dimo_ht_data.pack_type=temp_Buf[0];
		Dimo_ht_data.state=temp_Buf[1];
		Dimo_ht_data.Data_Buffer[0]=temp_Buf[2];
		Dimo_ht_data.Data_Buffer[1]=temp_Buf[3];
		Dimo_ht_data.Data_Buffer[2]=temp_Buf[4];
		Dimo_ht_data.Data_Buffer[3]=temp_Buf[5];
		Dimo_ht_data.Data_Buffer[4]=temp_Buf[6];
		Dimo_ht_data.Data_Buffer[5]=temp_Buf[7];
		Dimo_ht_data.Data_Buffer[6]=temp_Buf[8];
		Dimo_ht_data.Data_Buffer[7]=temp_Buf[9];
		Dimo_ht_data.Data_Buffer[8]=temp_Buf[10];
		Dimo_ht_data.Data_Buffer[9]=temp_Buf[11];
		Dimo_ht_data.addr =addr1;

		if (SERVER_MODE == 2)  
			send(upload_buffer.sock_mux,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]), sizeof(DIMO_HT_DATA), 0);  
		send(upload_buffer.sock,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]), sizeof(DIMO_HT_DATA), 0);  
		//DETECT_MESAGE_TIME_OUT_Command(temp_Buf,11);		
		//_time_delay(30000);//30S等待数据返回
}

//0x46开始命令
void Modbus_SendStartCommand(uint_16 addr1,uint_8* start_buffer)
{
  	uint_16 crc;
  	uint_8 len,temp_Buf[150];
	
    temp_Buf[0]=0x80;
  	temp_Buf[1] = CoodAddr;
	temp_Buf[2] = GATEWAY_TYPE;//0x46;
    temp_Buf[3] =(uint_8)(addr1>>8);
    temp_Buf[4] =(uint_8)(addr1);
	crc = Modbus_calcrc16(temp_Buf, 5);
	temp_Buf[5] = (uint_8)crc;
	temp_Buf[6] = (uint_8)(crc>>8);
	_time_delay(500);

	dicor_dis_timer();
	printf("\n*********************************************************");
	printf("\nSend Sampling CMD Funcode 0x%02x to device[0x%04x]", GATEWAY_TYPE, addr1);	 	
	printf("\nWaiting the response of the device[0x%04x]",addr1);
	rs485_clear_buf();
	rs485_write(temp_Buf, 7);
  	DETECT_MESAGE_TIME_OUT_Command(temp_Buf, 7, 18, addr1);
}



//中断问数据，Modbus发送读取保持寄存器的命令
int Modbus_SendReadRegCommand(uint_16 addr1, uint_8* data_buffer, uint_16 readstart, uint_16 readlen)
{
	uint_16 crc;
	uint_8 len = 0;
  _mqx_int recv_len;
  uint_8* data_buffer1;
  
 // uint_16 recv_len;
	uint_8 i;
   int_8 result;
   //GATEWAY_DATA* pGatewayData;
  uint_32 	;
  	GATEWAY_DATA* pGatewayData;
	int_8 GatewayDataLen = sizeof(GATEWAY_DATA);
   int_8 EMN_SMC_DATA_ST_Len = sizeof(EMN_SMC_DATA_ST);
	EMN_SMC_DATA_ST* pSMCData;
  UINT_8 watchdogenable = 0;
  // Modbus_SendStartCommand(addr1,data_buffer);
   _time_delay(300);
    data_buffer1 = RS485_Data_Buffer;
		
	recv_len = rs485_read(data_buffer1);
   //recv_len=rs485_read(data_buffer);
  // _time_delay(500);
    data_buffer[0]=0x80;
	data_buffer[1]=CoodAddr;
	for(i=6;i>0;i--)
	{
	data_buffer[2+i]=data_buffer[1+i];
	}
	data_buffer[2] = MODBUS_FUNCTION_READ_REGISTER;


	crc = Modbus_calcrc16(data_buffer, 9);
	data_buffer[9] = (uint_8)crc;
	data_buffer[10] = (uint_8)(crc>>8);
		
	rs485_write(data_buffer, 11);
//	void GATEWAY_ReadData(void *ptr);
	_time_delay(1500);

	result = Modbus_RecvReadRegData(addr1, data_buffer1, (uint_8*)pGatewayData);
	
  	//	if (result == 0)
//	{	
		ThirdDevicPreDataFinish();
//	}
	pSMCData = (EMN_SMC_DATA_ST *)malloc(sizeof(EMN_SMC_DATA_ST));
     
    for(i=0;i<GATEWAY_REGISTER_READ_MAX_LENGHT;i++)
    {
     pGatewayData = (GATEWAY_DATA*)(data_buffer1 + 9+GatewayDataLen*i);
     // uint_16 temp1= 0;
    //  uint_16 temp2= 0;
    //  uint_16 temp3= 0;
    //  uint_16 temp4 =0;
  
    
    pSMCData->frm_type = 0x10;
    pSMCData->len =0x16;
   
    //pSMCData->subnet= data_buffer[9+GatewayDataLen*i];
   
    pSMCData->dev_id =  pGatewayData->id;
    //data_buffer[10+GatewayDataLen*i];
   
      //  temp1 = (uint_16)data_buffer[13+GatewayDataLen*i];
     
       // temp1 = temp1 << 8;
     
       // temp1 = temp1 & 0xFF00;
      //  temp1|= data_buffer[12+GatewayDataLen*i];

      
        pSMCData->Vin = pGatewayData->Vout;
   
     //  temp2 = (uint_16)data_buffer[17+GatewayDataLen*i];
     //  temp2 = temp2 << 8;
     //  temp2 = temp2 & 0xFF00 ;
     //  temp2|= data_buffer[16+GatewayDataLen*i];
      pSMCData->Iout=pGatewayData->Iout ;
    
      // temp3 = (uint_16)data_buffer[15+GatewayDataLen*i];
       //temp3 = temp3 << 8;
      // temp3 = temp3 & 0xFF00;
      // temp3|= data_buffer[14+GatewayDataLen*i];
       pSMCData->Vout = pGatewayData->Vin;
       //temp4 = (uint_16)data_buffer[18+GatewayDataLen*i];
      // temp4 = temp2 << 8;
      // temp4 = temp2 & 0xFF00 ;
      // temp4|= data_buffer[17+GatewayDataLen*i];
       
        pSMCData->Temp = pGatewayData->Temp;
     pSMCData->out_power= 0x0000;
    
    pSMCData->d = 0x0000; 
    pSMCData->Energy = 0x0000;
    pSMCData->a=0x0000;
    pSMCData->b=0x0000;
   
 memcpy((char *)(&(upload_buffer.data0.didi_data[ EMN_SMC_DATA_ST_Len * i])),pSMCData,EMN_SMC_DATA_ST_Len);
 
            
     }
     
      free(pSMCData);
    upload_buffer.state = CAN_READ;
	EMN_ChildNum = EMN_DEV_NUM;
	rfnwk_state = RF_NWK_IDLE;	
	upload_buffer.write_index =22* i; 
	
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

  //	if (result == 0)
//	{	
		ThirdDevicPreDataFinish();
//	}
       // ThirdDeviceWaitSendData();

  return 1;
  
}

//Modbus发送ready命令
void Modbus_SendReadyCommand(uint_16 addr1,uint_8* ready_buffer)
{
	uint_16 crc;
    ready_buffer[0]=0x80;
  	ready_buffer[1]=CoodAddr;
	ready_buffer[2] = 0x68;
	crc = Modbus_calcrc16(ready_buffer, 5);
	ready_buffer[5] = (uint_8)crc;
	ready_buffer[6] = (uint_8)(crc>>8);
		
	rs485_write(ready_buffer, 7);
	

}

void SET_ROUTER_TABLE_Command(uint_16 addr,uint_8* data_buffer)
{
 uint_16 crc;
 int_8 i;
 uint_8 len,temp_Buf[150];
 uint_16 b_len;
  	_mqx_int recv_len;
  	//	uint_8 tmp_buf[100];
  uint_8* rx_data_buffer;
  	b_len = recv_len;
  //	tmp_buf[2]=data_buffer[2];
  //	printf("b_len:%4d\n",b_len);
  	//	printf("发送命令\n");
			 _time_delay(500);
			//	printf("等待回应...\n");
			 rx_data_buffer = RS485_Data_Buffer;
		
	         b_len = rs485_read(rx_data_buffer);
 temp_Buf[0]=0x80;
 temp_Buf[1]=CoodAddr;	


 	for(i=98;i>0;i--)
	{
	temp_Buf[2+i]=data_buffer[1+i];
	}
	 temp_Buf[2]=0x51;
  crc = Modbus_calcrc16(temp_Buf, 101);
  temp_Buf[101] = (uint_8)crc;
  temp_Buf[102] = (uint_8)(crc>>8);
  rs485_write(temp_Buf, 103);
               Dimo_ht_data.Dicor_UID[0]= pBaseConfig->uid[0];
	           Dimo_ht_data.Dicor_UID[1]= pBaseConfig->uid[1];
	           Dimo_ht_data.Dicor_UID[2]= pBaseConfig->uid[2];
	           Dimo_ht_data.Dicor_UID[3]= pBaseConfig->uid[3];
	           Dimo_ht_data.pack_obj=Dimo_response_data.pack_obj;
	           Dimo_ht_data.pack_type=temp_Buf[0];
	           Dimo_ht_data.state=temp_Buf[1];
	         //error = 
	           Dimo_ht_data.Data_Buffer[0]=temp_Buf[2];
	           Dimo_ht_data.Data_Buffer[1]=temp_Buf[3];
	           Dimo_ht_data.Data_Buffer[2]=temp_Buf[4];
	           Dimo_ht_data.Data_Buffer[3]=temp_Buf[5];
	           Dimo_ht_data.Data_Buffer[4]=temp_Buf[6];
	           Dimo_ht_data.Data_Buffer[5]=temp_Buf[7];
	           Dimo_ht_data.Data_Buffer[6]=temp_Buf[8];
	           Dimo_ht_data.Data_Buffer[7]=temp_Buf[9];
	           Dimo_ht_data.Data_Buffer[8]=temp_Buf[10];
	           Dimo_ht_data.Data_Buffer[9]=temp_Buf[11];
	           Dimo_ht_data.addr =addr;
	         if (SERVER_MODE == 2)  
	         	send(upload_buffer.sock_mux,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]), sizeof(DIMO_HT_DATA), 0);   
	         send(upload_buffer.sock,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]), sizeof(DIMO_HT_DATA), 0); 
  			len=103;
 			DETECT_MESAGE_TIME_OUT_Command(temp_Buf,len,1,addr);   
}

//ENTER_IAP_MODE_Command
int_32 Modbus_ENTER_IAP_MODE_Command(uint_16 addr_f,uint_16 addr_r,uint_8* data_buffer)
{
		uint_16 crc;
		uint_8 i;
		uint_8 len,temp_Buf[9]; 
	    temp_Buf[0]=0x80;
  	    temp_Buf[1]=CoodAddr;
	    temp_Buf[6]= (uint_8)addr_r;
	    temp_Buf[5]= (uint_8)(addr_r>>8);
	    temp_Buf[4]=(uint_8)addr_f;
	    temp_Buf[3]=(uint_8)(addr_f>>8);
	   	temp_Buf[2]=0x31;
	       
		crc = Modbus_calcrc16(temp_Buf, 7);
		temp_Buf[7] = (uint_8)crc;
	    temp_Buf[8] = (uint_8)(crc>>8);
	 	len=9;
	 	printf("\nSend:");
	 	for (i = 0; i < len; i++)
	 	{
	 		printf("%x ",temp_Buf[i] );
	 	} 
 		if (DETECT_ISP_CMD(temp_Buf, 9, 10, 2) == -1)
 		{
 			return -1;
 		}
}

//ERASE_FLASH_Command
int_32 Modbus_ERASE_FLASH_Command(uint_16 addr_f,uint_16 addr_r,uint_8* data_buffer)
{
	uint_16 crc;
	uint_8  temp_Buf[150];
	temp_Buf[0]=0x80;
	temp_Buf[1]=CoodAddr;
	temp_Buf[6]= (uint_8)addr_r;
	temp_Buf[5]= (uint_8)(addr_r>>8);
	temp_Buf[4]=(uint_8)addr_f;
	temp_Buf[3]=(uint_8)(addr_f>>8);
	temp_Buf[2]=0x32;

	crc = Modbus_calcrc16(temp_Buf, 7);
	temp_Buf[7] = (uint_8)crc;
	temp_Buf[8] = (uint_8)(crc>>8); 
	if (DETECT_ISP_CMD(temp_Buf, 9, 20, 2) == -1)
	{
		return -1;
	}
}

//WRITE_FLASH_Command
int_32 Modbus_WRITE_FLASH_Command(uint_16 addr_f,uint_16 addr_r,uint_8* data_buffer,uint_8 readlen)
{
    uint_32 result;
     uint_16 crc;
     uint_8 i,temp_Buf[150];
  
    for(i=0;i<readlen-3;i++)
	{
		temp_Buf[3+i]=data_buffer[4+i];
	}
	temp_Buf[1]=CoodAddr;
	temp_Buf[0]=0x80;
	temp_Buf[2]=0x33;
    crc = Modbus_calcrc16(temp_Buf, readlen);
	temp_Buf[readlen] = (uint_8)crc;
    temp_Buf[readlen+1] = (uint_8)(crc>>8);
    
    //temp_Buf[readlen+2] = '\0';
    //rs485_write(temp_Buf, readlen+3);
    //printf ("\n______tenmp_Buf:%d______\n", readlen+1+1);
    //rs485_write(temp_Buf, readlen+3);
	result = DETECT_ISP_CMD(temp_Buf, readlen+1+1, 20, 2);//
	//printf("\n____return %d WRITE____\n", result);
	if (result==-1)
	{
		//printf("\n____return %d WRITE____\n", result);
		return -1;
	}
}

//END_IAP_Command
int_32 END_IAP_Command(uint_16 addr_f,uint_16 addr_r,uint_16 crc,uint_8* data_buffer)
{
	uint_16 crc_end;
	uint_8 i;
	uint_8 temp_Buf[150];
	temp_Buf[0]=0x80;
	temp_Buf[1]=CoodAddr;
	temp_Buf[6]= (uint_8)addr_r;
	temp_Buf[5]= (uint_8)(addr_r>>8);
	temp_Buf[4]=(uint_8)addr_f;
	temp_Buf[3]=(uint_8)(addr_f>>8);
	temp_Buf[2]=0x34;
	temp_Buf[7]=(uint_8)(crc>>8);  //0x11
	temp_Buf[8]=(uint_8)crc;       //0x11
	crc_end = Modbus_calcrc16(temp_Buf, 9);
	temp_Buf[9] = (uint_8)crc_end;
	temp_Buf[10] = (uint_8)(crc_end>>8);
	printf("\nSend:");
 	for (i = 0; i < 11; i++)
 	{
 		printf("%x ",temp_Buf[i] );
 	} 	
	if (DETECT_ISP_CMD(temp_Buf, 11, 20, 2) == -1)
	{
		return -1;
	}
}

//VERSION
int_32 MAGNUS_LOAD_PREAMBLE_Command(uint_16 addr_f,uint_16 addr_r,uint_8* data_buffer,uint_16 version)
{
   uint_16 crc;
   uint_8 len,temp_Buf[150];
 // rs485_read(data_buffer);
    temp_Buf[0]=0x80;
  	temp_Buf[1]=CoodAddr;
    temp_Buf[2]=0x35;
	temp_Buf[3]=(uint_8)(addr_f>>8);
	temp_Buf[4]=(uint_8)addr_f;
	temp_Buf[5]=(uint_8)(addr_r>>8);
	temp_Buf[6]=(uint_8)addr_r;
	temp_Buf[7]=0xa5;
	temp_Buf[8]=0x5a;
	temp_Buf[9]=0x00;
	temp_Buf[10]=0x00;
	temp_Buf[11]=0xc0;
	temp_Buf[12]=0x00;
	temp_Buf[13]=(uint_8)(version>>8);
	temp_Buf[14]=(uint_8)version;
	temp_Buf[15]=0x00;
	temp_Buf[16]=0x01;
	temp_Buf[17]=0x00;
	temp_Buf[18]=Dimo_response_data.role;
	crc = Modbus_calcrc16(temp_Buf, 19);
	temp_Buf[19] = (uint_8)crc;
	temp_Buf[20] = (uint_8)(crc>>8);
	
	if (DETECT_ISP_CMD(temp_Buf, 21, 10, 2)==-1)
	{
		return -1;
	}
	
}

//SET_FFD_TABLE_Command
void SET_FFD_TABLE_Command(uint_16 addr,uint_8* data_buffer)
{
	
     uint_16 crc;
	 uint_8 len =0,temp_Buf[150];//Lewis:最大子节点数+冗余，足够长
	 int_8 i;
	 
	 temp_Buf[0]=0x80;
  	 temp_Buf[1]=CoodAddr;
     temp_Buf[2]=0x66;
     
	 len =data_buffer[8];//len

     _time_delay(500);
     
   	for(i=len+7;i>0;i--)
	{
		temp_Buf[2+i]=data_buffer[1+i];
	}

        temp_Buf[9]=len;
		crc = Modbus_calcrc16(temp_Buf, len+10);//Lewis
		temp_Buf[len+10] = (uint_8)crc;
	    temp_Buf[len+11] = (uint_8)(crc>>8);
	   	rs485_write(temp_Buf, len+12);
	   	       Dimo_ht_data.Dicor_UID[0]= pBaseConfig->uid[0];
	           Dimo_ht_data.Dicor_UID[1]= pBaseConfig->uid[1];
	           Dimo_ht_data.Dicor_UID[2]= pBaseConfig->uid[2];
	           Dimo_ht_data.Dicor_UID[3]= pBaseConfig->uid[3];
	           Dimo_ht_data.pack_obj=Dimo_response_data.pack_obj;
	           Dimo_ht_data.pack_type=temp_Buf[0];
	           Dimo_ht_data.state=temp_Buf[1];
	         //error = 
	           Dimo_ht_data.Data_Buffer[0]=temp_Buf[2];
	           Dimo_ht_data.Data_Buffer[1]=temp_Buf[3];
	           Dimo_ht_data.Data_Buffer[2]=temp_Buf[4];
	           Dimo_ht_data.Data_Buffer[3]=temp_Buf[5];
	           Dimo_ht_data.Data_Buffer[4]=temp_Buf[6];
	           Dimo_ht_data.Data_Buffer[5]=temp_Buf[7];
	           Dimo_ht_data.Data_Buffer[6]=temp_Buf[8];
	           Dimo_ht_data.Data_Buffer[7]=temp_Buf[9];
	           Dimo_ht_data.Data_Buffer[8]=temp_Buf[10];
	           Dimo_ht_data.Data_Buffer[9]=temp_Buf[11];
	           Dimo_ht_data.addr =addr;
	         if (SERVER_MODE == 2)  
	         	send(upload_buffer.sock_mux,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]),sizeof(DIMO_HT_DATA), 0);
	         send(upload_buffer.sock,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]), sizeof(DIMO_HT_DATA), 0); 
	   	 _time_delay(1000);
 DETECT_MESAGE_TIME_OUT_Command(temp_Buf,len+12,1,addr);  
	   	
}


void SET_MAGNUS_ROLE_Command(uint_16 addr,uint_8* data_buffer)
{
		uint_16 crc;
 	uint_8 temp_buf[9];//Lewis：独立的设置一个Buf，临时的，避免在原数据Buf中操作。否则buf不能被下一个函数调用了！！！
    temp_buf[0]=0x80;
  	temp_buf[1]=CoodAddr;
  	temp_buf[6]=data_buffer[5];
  	temp_buf[5]=data_buffer[4];
  	temp_buf[4]=data_buffer[3];

	temp_buf[3]=data_buffer[2];
 	temp_buf[2]=0x47;
 	
 	crc = Modbus_calcrc16(temp_buf, 5);
	temp_buf[7] = (uint_8)crc;
	temp_buf[8] = (uint_8)(crc>>8);
	 _time_delay(200);
	rs485_write(temp_buf, 9);
		
 DETECT_MESAGE_TIME_OUT_Command(temp_buf,9,3,addr);
}  


//MODIFY_FFD_NODE_NUM_Command
void MODIFY_FFD_NODE_NUM_Command(uint_16 addr,uint_8* data_buffer)
{
  uint_16 crc;
  uint_8 len;
  int_8 i;
  len = 6;
    data_buffer[0]=0x80;
  	data_buffer[1]=CoodAddr;
  	for(i=0;i<len+4;i++)
	{
	data_buffer[3+i]=data_buffer[2+i];
	}	
	data_buffer[2]=0x06;
	crc = Modbus_calcrc16(data_buffer, 5);
	data_buffer[5] = (uint_8)crc;
	data_buffer[6] = (uint_8)(crc>>8);
	rs485_write(data_buffer, 7);
}

//FFD_BIND_RFD_Command
void FFD_BIND_RFD_Command(uint_16 addr,uint_8* data_buffer)
{
 	uint_16 crc;
 	uint_8 temp_Buf[7];//Lewis：独立的设置一个Buf，临时的，避免在原数据Buf中操作。否则buf不能被下一个函数调用了！！！
    temp_Buf[0]=0x80;
  	temp_Buf[1]=CoodAddr;
  	temp_Buf[4]=data_buffer[3];

	temp_Buf[3]=data_buffer[2];
 	temp_Buf[2]=0x45;
 	crc = Modbus_calcrc16(temp_Buf, 5);
	temp_Buf[5] = (uint_8)crc;
	temp_Buf[6] = (uint_8)(crc>>8);
	 _time_delay(200);
	rs485_write(temp_Buf, 7);
	           Dimo_ht_data.Dicor_UID[0]= pBaseConfig->uid[0];
	           Dimo_ht_data.Dicor_UID[1]= pBaseConfig->uid[1];
	           Dimo_ht_data.Dicor_UID[2]= pBaseConfig->uid[2];
	           Dimo_ht_data.Dicor_UID[3]= pBaseConfig->uid[3];
	           Dimo_ht_data.pack_obj=Dimo_response_data.pack_obj;
	           Dimo_ht_data.pack_type=temp_Buf[0];
	           Dimo_ht_data.state=temp_Buf[1];
	         //error = 
	           Dimo_ht_data.Data_Buffer[0]=temp_Buf[2];
	           Dimo_ht_data.Data_Buffer[1]=temp_Buf[3];
	           Dimo_ht_data.Data_Buffer[2]=temp_Buf[4];
	           Dimo_ht_data.Data_Buffer[3]=temp_Buf[5];
	           Dimo_ht_data.Data_Buffer[4]=temp_Buf[6];
	           Dimo_ht_data.Data_Buffer[5]=temp_Buf[7];
	           Dimo_ht_data.Data_Buffer[6]=temp_Buf[8];
	           Dimo_ht_data.Data_Buffer[7]=temp_Buf[9];
	           Dimo_ht_data.Data_Buffer[8]=temp_Buf[10];
	           Dimo_ht_data.Data_Buffer[9]=temp_Buf[11];
	           Dimo_ht_data.addr =addr;
	           if (SERVER_MODE == 2)
	           		send(upload_buffer.sock_mux,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]), sizeof(DIMO_HT_DATA), 0); 
	           send(upload_buffer.sock,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]), sizeof(DIMO_HT_DATA), 0); 
	_time_delay(4000);
		
 DETECT_MESAGE_TIME_OUT_Command(temp_Buf,7,15,addr);	
}

//FFD_START_COLLECT_Command
void FFD_START_COLLECT_Command(uint_16 addr,uint_8* data_buffer)
{
	

}



void SW_Command(uint_16 addr,uint_8* data_buffer)
{
  	uint_16 crc;
  	uint_8 len;
  	uint_8 temp_Buf[8];
  	temp_Buf[0]=0x80;
	temp_Buf[1]=CoodAddr;
  
  	temp_Buf[4]=data_buffer[3];
  	temp_Buf[3]=data_buffer[2];
  	temp_Buf[2]=GATEWAY_TYPE;//0x46;
  	crc = Modbus_calcrc16(temp_Buf, 5);
  	temp_Buf[5]=(uint_8)crc;
  	temp_Buf[6]=(uint_8)(crc>>8);
  	rs485_write(temp_Buf, 7);
  	           Dimo_ht_data.Dicor_UID[0]= pBaseConfig->uid[0];
	           Dimo_ht_data.Dicor_UID[1]= pBaseConfig->uid[1];
	           Dimo_ht_data.Dicor_UID[2]= pBaseConfig->uid[2];
	           Dimo_ht_data.Dicor_UID[3]= pBaseConfig->uid[3];
	           Dimo_ht_data.pack_obj=Dimo_response_data.pack_obj;
	           Dimo_ht_data.pack_type=temp_Buf[0];
	           Dimo_ht_data.state=temp_Buf[1];
	         //error = 
	           Dimo_ht_data.Data_Buffer[0]=temp_Buf[2];
	           Dimo_ht_data.Data_Buffer[1]=temp_Buf[3];
	           Dimo_ht_data.Data_Buffer[2]=temp_Buf[4];
	           Dimo_ht_data.Data_Buffer[3]=temp_Buf[5];
	           Dimo_ht_data.Data_Buffer[4]=temp_Buf[6];
	           Dimo_ht_data.Data_Buffer[5]=temp_Buf[7];
	           Dimo_ht_data.Data_Buffer[6]=temp_Buf[8];
	           Dimo_ht_data.Data_Buffer[7]=temp_Buf[9];
	           Dimo_ht_data.Data_Buffer[8]=temp_Buf[10];
	           Dimo_ht_data.Data_Buffer[9]=temp_Buf[11];
	           Dimo_ht_data.addr =addr;
	         if (SERVER_MODE == 2)  
	         	send(upload_buffer.sock_mux,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]), sizeof(DIMO_HT_DATA), 0);   
	         send(upload_buffer.sock,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]), sizeof(DIMO_HT_DATA), 0); 
  	_time_delay(5000);
  		len=7;
 DETECT_MESAGE_TIME_OUT_Command(temp_Buf,len,6,addr);	
}

void Write_Magnus_Parameter_CMD(uint_16 addr_f,uint_16 addr_r,uint_8* data_buffer)
{
	
	
     uint_16 crc;
	 uint_8 len =0,temp_Buf[150];//Lewis:最大子节点数+冗余，足够长
	 int_8 i;
//	extern void dicor_dis_timer(void);
	 temp_Buf[0]=0x80;
  	 temp_Buf[1]=data_buffer[1];
     temp_Buf[2]=0x10;
     
	 len =data_buffer[10];
     
   	for(i=0;i<len+9;i++)
	{
		temp_Buf[3+i]=data_buffer[2+i];
	}

        //temp_Buf[9]=len;
		crc = Modbus_calcrc16(temp_Buf, len+12);//Lewis
		temp_Buf[len+12] = (uint_8)crc;
	    temp_Buf[len+13] = (uint_8)(crc>>8);

	   	rs485_clear_buf();
	   	rs485_write(temp_Buf, len+14);
/*	   	
	   	       Dimo_ht_data.Dicor_UID[0]= pBaseConfig->uid[0];
	           Dimo_ht_data.Dicor_UID[1]= pBaseConfig->uid[1];
	           Dimo_ht_data.Dicor_UID[2]= pBaseConfig->uid[2];
	           Dimo_ht_data.Dicor_UID[3]= pBaseConfig->uid[3];
	           Dimo_ht_data.pack_obj=Dimo_response_data.pack_obj;
	           Dimo_ht_data.pack_type=temp_Buf[0];
	           Dimo_ht_data.state=temp_Buf[1];
	         //error = 
	           Dimo_ht_data.Data_Buffer[0]=temp_Buf[2];
	           Dimo_ht_data.Data_Buffer[1]=temp_Buf[3];
	           Dimo_ht_data.Data_Buffer[2]=temp_Buf[4];
	           Dimo_ht_data.Data_Buffer[3]=temp_Buf[5];
	           Dimo_ht_data.Data_Buffer[4]=temp_Buf[6];
	           Dimo_ht_data.Data_Buffer[5]=temp_Buf[7];
	           Dimo_ht_data.Data_Buffer[6]=temp_Buf[8];
	           Dimo_ht_data.Data_Buffer[7]=temp_Buf[9];
	           Dimo_ht_data.Data_Buffer[8]=temp_Buf[10];
	           Dimo_ht_data.Data_Buffer[9]=temp_Buf[11];
	           Dimo_ht_data.Data_Buffer[10]=temp_Buf[12];
	           Dimo_ht_data.Data_Buffer[11]=temp_Buf[13];
	           Dimo_ht_data.Data_Buffer[12]=temp_Buf[14];
	           Dimo_ht_data.Data_Buffer[13]=temp_Buf[15];
	           Dimo_ht_data.Data_Buffer[14]=0x00;
	           Dimo_ht_data.Data_Buffer[15]=0x00;
	           Dimo_ht_data.Data_Buffer[16]=0x00;
	           Dimo_ht_data.Data_Buffer[17]=0x00;
	           Dimo_ht_data.Data_Buffer[18]=0x00;
	           Dimo_ht_data.Data_Buffer[19]=0x00;
	           Dimo_ht_data.addr =addr_r;
*/
	dicor_dis_timer();	           
	printf("\n*********************************************************");	
	printf("\nSend Magnus Register Config CMD [0x10]");
	printf("\nGateway ID [0x%04x]", data_buffer[1]);
	printf("\nFFD ID [0x%04x]", data_buffer[3]);
	printf("\nRFD ID [0x%04x]", data_buffer[5]);
	printf("\nTrag Gateway ID [0x%04x]", data_buffer[12]);
	printf("\nTrag Gateway BD [0x%02x]", data_buffer[30]);
	printf("\nWaiting the response of the device[0x%04x]",data_buffer[1]);
/*	if (SERVER_MODE == 2)
		send(upload_buffer.sock_mux,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]), sizeof(DIMO_HT_DATA), 0); 
	send(upload_buffer.sock,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]), sizeof(DIMO_HT_DATA), 0); 
*/	_time_delay(1000);
	DETECT_MESAGE_TIME_OUT_Command(temp_Buf,len+12,10,addr_r);  
}

void Read_Magnus_Parameter_CMD(uint_16 addr_f,uint_16 addr_r,uint_8* data_buffer)
{
	uint_16 crc;
	 uint_8 len =0,temp_Buf[150];//Lewis:最大子节点数+冗余，足够长
	 int_8 i,result;
	 
	 uint_16 b_len;
  	_mqx_int recv_len;
  	//	uint_8 tmp_buf[100];
  uint_8* rx_data_buffer;
  	b_len = recv_len;
  //	tmp_buf[2]=data_buffer[2];
  	
  	rx_data_buffer = RS485_Data_Buffer;
	//	_time_delay(1000);
	 b_len = rs485_read(rx_data_buffer);
	 temp_Buf[0]=0x80;
  	 temp_Buf[1]=data_buffer[1];
     temp_Buf[2]=0x03;
     
	 //len =data_buffer[10];//len

    // _time_delay(500);
     
   	for(i=0;i<8;i++)
	{
		temp_Buf[3+i]=data_buffer[2+i];
	}

        //temp_Buf[9]=len;
		crc = Modbus_calcrc16(temp_Buf, 11);//Lewis
		temp_Buf[11] = (uint_8)crc;
	    temp_Buf[12] = (uint_8)(crc>>8);
	   	rs485_write(temp_Buf, 13);
	   	       Dimo_ht_data.Dicor_UID[0]= pBaseConfig->uid[0];
	           Dimo_ht_data.Dicor_UID[1]= pBaseConfig->uid[1];
	           Dimo_ht_data.Dicor_UID[2]= pBaseConfig->uid[2];
	           Dimo_ht_data.Dicor_UID[3]= pBaseConfig->uid[3];
	           Dimo_ht_data.pack_obj=Dimo_response_data.pack_obj;
	           Dimo_ht_data.pack_type=temp_Buf[0];
	           Dimo_ht_data.state=temp_Buf[1];
	         //error = 
	           Dimo_ht_data.Data_Buffer[0]=temp_Buf[2];
	           Dimo_ht_data.Data_Buffer[1]=temp_Buf[3];
	           Dimo_ht_data.Data_Buffer[2]=temp_Buf[4];
	           Dimo_ht_data.Data_Buffer[3]=temp_Buf[5];
	           Dimo_ht_data.Data_Buffer[4]=temp_Buf[6];
	           Dimo_ht_data.Data_Buffer[5]=temp_Buf[7];
	           Dimo_ht_data.Data_Buffer[6]=temp_Buf[8];
	           Dimo_ht_data.Data_Buffer[7]=temp_Buf[9];
	           Dimo_ht_data.Data_Buffer[8]=temp_Buf[10];
	           Dimo_ht_data.Data_Buffer[9]=temp_Buf[11];
	           Dimo_ht_data.Data_Buffer[10]=temp_Buf[12];
	           Dimo_ht_data.Data_Buffer[11]=0x00;
	           Dimo_ht_data.Data_Buffer[12]=0x00;
	           Dimo_ht_data.Data_Buffer[13]=0x00;
	           Dimo_ht_data.Data_Buffer[14]=0x00;
	           Dimo_ht_data.Data_Buffer[15]=0x00;
	           Dimo_ht_data.Data_Buffer[16]=0x00;
	           Dimo_ht_data.Data_Buffer[17]=0x00;
	           Dimo_ht_data.Data_Buffer[18]=0x00;
	           Dimo_ht_data.Data_Buffer[19]=0x00;
	           Dimo_ht_data.addr =addr_r;
	           if (SERVER_MODE == 2)
	           	send(upload_buffer.sock_mux,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]), sizeof(DIMO_HT_DATA), 0);
	           send(upload_buffer.sock,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]), sizeof(DIMO_HT_DATA), 0);
	        //_time_delay(1000);   
	   result=   Modbus_RecvReadParameter(addr_r,temp_Buf);
}

int_8 Modbus_RecvReadParameter(uint_16 addr,uint_8* data_buffer)
{
 	 uint_8 i;	
  uint_16 b_len;
  	_mqx_int recv_len;
  		uint_8 tmp_buf[100];
  uint_8* rx_data_buffer;
  	b_len = recv_len;
  	tmp_buf[2]=data_buffer[2];
  	
  	rx_data_buffer = RS485_Data_Buffer;
		_time_delay(1000);
	 b_len = rs485_read(rx_data_buffer);
  	
	if((tmp_buf[5]==0x0B)&&(rx_data_buffer[0]==0x00)&&(rx_data_buffer[2]==tmp_buf[2]))
	{
		printf("\ntimeout\n");
		goto END;
	}
	else if ((tmp_buf[5]!=0x0B)&&(tmp_buf[5]!=0x0E))
	{
	// malloc(b_len);
//	 memcpy(upload_buffer.data0.didi_data,data_buffer,b_len);
	 // free(data_buffer);
	  //upload_buffer.data0.type=0x15;
	//  upload_buffer.write_index=b_len;
		      //printf("receiveParameter successfully \n");
		      printf("\nThe device [0x%04x] replies CMD( FuncCode:0x%02x) successfully!\n",addr,tmp_buf[2]);
           	printf("RSP Frame:%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x\n",rx_data_buffer[0],rx_data_buffer[1],rx_data_buffer[2],rx_data_buffer[3],rx_data_buffer[4],rx_data_buffer[5],rx_data_buffer[6],rx_data_buffer[7],rx_data_buffer[8],rx_data_buffer[9],rx_data_buffer[10],rx_data_buffer[11],rx_data_buffer[12],rx_data_buffer[13],rx_data_buffer[14],rx_data_buffer[15],rx_data_buffer[16],rx_data_buffer[17],rx_data_buffer[18],rx_data_buffer[19],rx_data_buffer[20],rx_data_buffer[21],rx_data_buffer[22]);
           	printf("*******************************************************************\n");
		    	goto END;
	  	
	}
 END:
               Dimo_ht_data.Dicor_UID[0]= pBaseConfig->uid[0];
	           Dimo_ht_data.Dicor_UID[1]= pBaseConfig->uid[1];
	           Dimo_ht_data.Dicor_UID[2]= pBaseConfig->uid[2];
	           Dimo_ht_data.Dicor_UID[3]= pBaseConfig->uid[3];
	           Dimo_ht_data.pack_obj=Dimo_response_data.pack_obj;
	           Dimo_ht_data.pack_type=tmp_buf[2];
	           Dimo_ht_data.state=rx_data_buffer[5];
	         //error = 
	           Dimo_ht_data.Data_Buffer[0]=rx_data_buffer[0];
	           Dimo_ht_data.Data_Buffer[1]=rx_data_buffer[1];
	           Dimo_ht_data.Data_Buffer[2]=rx_data_buffer[2];
	           Dimo_ht_data.Data_Buffer[3]=rx_data_buffer[3];
	           Dimo_ht_data.Data_Buffer[4]=rx_data_buffer[4];
	           Dimo_ht_data.Data_Buffer[5]=rx_data_buffer[5];
	           Dimo_ht_data.Data_Buffer[6]=rx_data_buffer[6];
	           Dimo_ht_data.Data_Buffer[7]=rx_data_buffer[7];
	           Dimo_ht_data.Data_Buffer[8]=rx_data_buffer[8];
	           Dimo_ht_data.Data_Buffer[9]=rx_data_buffer[9];
	           Dimo_ht_data.Data_Buffer[10]=rx_data_buffer[10];
	           Dimo_ht_data.Data_Buffer[11]=rx_data_buffer[11];
	           Dimo_ht_data.Data_Buffer[12]=rx_data_buffer[12];
	           Dimo_ht_data.Data_Buffer[13]=rx_data_buffer[13];
	           Dimo_ht_data.Data_Buffer[14]=rx_data_buffer[14];
	           Dimo_ht_data.Data_Buffer[15]=rx_data_buffer[15];
	           Dimo_ht_data.Data_Buffer[16]=rx_data_buffer[16];
	           Dimo_ht_data.Data_Buffer[17]=rx_data_buffer[17];
	           Dimo_ht_data.Data_Buffer[18]=rx_data_buffer[18];
	           Dimo_ht_data.Data_Buffer[19]=rx_data_buffer[19];
	           Dimo_ht_data.Data_Buffer[20]=rx_data_buffer[20];
	           Dimo_ht_data.Data_Buffer[21]=rx_data_buffer[21];
	           Dimo_ht_data.Data_Buffer[22]=rx_data_buffer[22];
	           Dimo_ht_data.addr =addr;
	         if (SERVER_MODE == 2)  
	         	send(upload_buffer.sock_mux,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]), sizeof(DIMO_HT_DATA), 0);  
	         send(upload_buffer.sock,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]), sizeof(DIMO_HT_DATA), 0);

}
   
   
   
   
void FFD_STOP_COLLECT_Command(uint_16 addr,uint_8* data_buffer)
{
  	uint_16 crc;
 	data_buffer[2]=0x49;
 	data_buffer[0]=0x80;
  	data_buffer[1]=CoodAddr;
  
 	crc = Modbus_calcrc16(data_buffer, 3);
	data_buffer[3] = (uint_8)crc;
	data_buffer[4] = (uint_8)(crc>>8);
	rs485_write(data_buffer, 5);	
}


//OPTIMUS_SYNC_Command
void OPTIMUS_SYNC_Command(uint_16 addr,uint_8* data_buffer)
{
  	uint_16 crc, i;
  	uint_8 len,temp_Buf[150];
  	uint_16 b_len;
  	_mqx_int recv_len;
  	//	uint_8 tmp_buf[100];
  	uint_8* rx_data_buffer;
  	DATE_STRUCT date;
  
  	b_len = recv_len;
  	
  	rx_data_buffer = RS485_Data_Buffer;
	 b_len = rs485_read(rx_data_buffer);
    temp_Buf[0]=0x80;
  	temp_Buf[1]=CoodAddr;
	temp_Buf[2] = 0x43;
    temp_Buf[3] =(uint_8)(addr>>8);
    temp_Buf[4] =(uint_8)(addr);
    dicor_get_time(&date);
	temp_Buf[5] =(uint_8)(date.DAY);
    temp_Buf[6] =(uint_8)(date.HOUR);
    temp_Buf[7] =(uint_8)(date.MINUTE);
    temp_Buf[8] =(uint_8)(date.SECOND);
    temp_Buf[9] =(uint_8)(date.MILLISEC>>8);
    temp_Buf[10] =(uint_8)(date.MILLISEC);
	crc = Modbus_calcrc16(temp_Buf, 11);
	temp_Buf[11] = (uint_8)crc;
	temp_Buf[12] = (uint_8)(crc>>8);
	_time_delay(100);
	
	dicor_dis_timer();	 
	printf("\n*********************************************************");
	printf("\nSend SYNC CMD Funcode 0x43 to device[0x%04x]",addr);		
	printf("\nWaiting SYNC the response of the device[0x%04x]",addr);
	rs485_clear_buf();
	rs485_write(temp_Buf,13);
    printf("\nREQ :");
	for (i = 0; i < 13; i++)
	{
		printf("%02x ",temp_Buf[i] );
	}
/*		        
	Dimo_ht_data.Dicor_UID[0]= pBaseConfig->uid[0];
	Dimo_ht_data.Dicor_UID[1]= pBaseConfig->uid[1];
	Dimo_ht_data.Dicor_UID[2]= pBaseConfig->uid[2];
	Dimo_ht_data.Dicor_UID[3]= pBaseConfig->uid[3];
	Dimo_ht_data.pack_obj=Dimo_response_data.pack_obj;
	Dimo_ht_data.pack_type=temp_Buf[0];
	Dimo_ht_data.state=temp_Buf[1];
	Dimo_ht_data.Data_Buffer[0]=temp_Buf[2];
	Dimo_ht_data.Data_Buffer[1]=temp_Buf[3];
	Dimo_ht_data.Data_Buffer[2]=temp_Buf[4];
	Dimo_ht_data.Data_Buffer[3]=temp_Buf[5];
	Dimo_ht_data.Data_Buffer[4]=temp_Buf[6];
	Dimo_ht_data.Data_Buffer[5]=temp_Buf[7];
	Dimo_ht_data.Data_Buffer[6]=temp_Buf[8];
	Dimo_ht_data.Data_Buffer[7]=temp_Buf[9];
	Dimo_ht_data.Data_Buffer[8]=temp_Buf[10];
	Dimo_ht_data.Data_Buffer[9]=temp_Buf[11];
	Dimo_ht_data.addr =addr;
	if (SERVER_MODE == 2)           
		send(upload_buffer.sock_mux,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]), sizeof(DIMO_HT_DATA), 0);            
	send(upload_buffer.sock,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]), sizeof(DIMO_HT_DATA), 0); 
	_time_delay(400);
*/	
	len=13;
 	DETECT_MESAGE_TIME_OUT_Command(temp_Buf,len,3,addr);	
}

//turn on mosfet
void TURN_ON_Command(uint_16 addr,uint_8* data_buffer)
{
    uint_16 crc;
	 uint_8 len;
	uint_8 tmp_buf[7];//Lewis：同上问题
	tmp_buf[0]=0x80;
	tmp_buf[1]=CoodAddr;
  
  	//data_buffer[4]=addr;
  	//data_buffer[3]=(uint_8)(addr>>8);
  	
  	tmp_buf[4]=data_buffer[3];
  	tmp_buf[3]=data_buffer[2];
	
  	tmp_buf[2]=0x41;

	 crc = Modbus_calcrc16(tmp_buf,5);
	tmp_buf[5] = (uint_8)crc;
	tmp_buf[6] = (uint_8)(crc>>8);
    
	rs485_write(tmp_buf, 7);
	           Dimo_ht_data.Dicor_UID[0]= pBaseConfig->uid[0];
	           Dimo_ht_data.Dicor_UID[1]= pBaseConfig->uid[1];
	           Dimo_ht_data.Dicor_UID[2]= pBaseConfig->uid[2];
	           Dimo_ht_data.Dicor_UID[3]= pBaseConfig->uid[3];
	           Dimo_ht_data.pack_obj=Dimo_response_data.pack_obj;
	           Dimo_ht_data.pack_type=tmp_buf[0];
	           Dimo_ht_data.state=tmp_buf[1];
	         //error = 
	           Dimo_ht_data.Data_Buffer[0]=tmp_buf[2];
	           Dimo_ht_data.Data_Buffer[1]=tmp_buf[3];
	           Dimo_ht_data.Data_Buffer[2]=tmp_buf[4];
	           Dimo_ht_data.Data_Buffer[3]=tmp_buf[5];
	           Dimo_ht_data.Data_Buffer[4]=tmp_buf[6];
	           Dimo_ht_data.Data_Buffer[5]=tmp_buf[7];
	           Dimo_ht_data.Data_Buffer[6]=tmp_buf[8];
	           Dimo_ht_data.Data_Buffer[7]=tmp_buf[9];
	           Dimo_ht_data.Data_Buffer[8]=tmp_buf[10];
	           Dimo_ht_data.Data_Buffer[9]=tmp_buf[11];
	           Dimo_ht_data.addr =addr;
	         if (SERVER_MODE == 2)  
	         	send(upload_buffer.sock_mux,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]), sizeof(DIMO_HT_DATA), 0);   
	         send(upload_buffer.sock,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]), sizeof(DIMO_HT_DATA), 0); 
		_time_delay(500);// 1000
	len=7;
 DETECT_MESAGE_TIME_OUT_Command(tmp_buf,len,10,addr);	//2014.4.17 由10改为1
}

//MAGNUS_SHUTDOWN_Command
void MAGNUS_SHUTDOWN_Command(uint_16 addr,uint_8* data_buffer)
{
   	uint_16 crc;
	 uint_8 len;
	 uint_8 tmp_buf[7];
	tmp_buf[0]=0x80;
  	tmp_buf[1]=CoodAddr;
  
  	tmp_buf[4]=data_buffer[3];
  	tmp_buf[3]=data_buffer[2];
  	tmp_buf[2]=0x44;

	 crc = Modbus_calcrc16(tmp_buf,5);
	tmp_buf[5] = (uint_8)crc;
	tmp_buf[6] = (uint_8)(crc>>8);

	rs485_write(tmp_buf, 7);
	           Dimo_ht_data.Dicor_UID[0]= pBaseConfig->uid[0];
	           Dimo_ht_data.Dicor_UID[1]= pBaseConfig->uid[1];
	           Dimo_ht_data.Dicor_UID[2]= pBaseConfig->uid[2];
	           Dimo_ht_data.Dicor_UID[3]= pBaseConfig->uid[3];
	           Dimo_ht_data.pack_obj=Dimo_response_data.pack_obj;
	           Dimo_ht_data.pack_type=tmp_buf[0];
	           Dimo_ht_data.state=tmp_buf[1];
	         //error = 
	           Dimo_ht_data.Data_Buffer[0]=tmp_buf[2];
	           Dimo_ht_data.Data_Buffer[1]=tmp_buf[3];
	           Dimo_ht_data.Data_Buffer[2]=tmp_buf[4];
	           Dimo_ht_data.Data_Buffer[3]=tmp_buf[5];
	           Dimo_ht_data.Data_Buffer[4]=tmp_buf[6];
	           Dimo_ht_data.Data_Buffer[5]=tmp_buf[7];
	           Dimo_ht_data.Data_Buffer[6]=tmp_buf[8];
	           Dimo_ht_data.Data_Buffer[7]=tmp_buf[9];
	           Dimo_ht_data.Data_Buffer[8]=tmp_buf[10];
	           Dimo_ht_data.Data_Buffer[9]=tmp_buf[11];
	           Dimo_ht_data.addr =addr;
	         if (SERVER_MODE == 2)  
	         	send(upload_buffer.sock_mux,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]), sizeof(DIMO_HT_DATA), 0);   
	         send(upload_buffer.sock,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]), sizeof(DIMO_HT_DATA), 0); 
		_time_delay(500);//1000
	len=7;
 DETECT_MESAGE_TIME_OUT_Command(tmp_buf,len,10,addr);		//2014.4.17 由10改为1
}

int_32 SEND_INFO_DEL(uint_16 id, uint_8 cmd, uint_8 state)//(uint_8 *temp_Buf)
{
	

	Cosmos_to_tele.Dicor_UID[0]= pBaseConfig->uid[0];
	Cosmos_to_tele.Dicor_UID[1]= pBaseConfig->uid[1];
	Cosmos_to_tele.Dicor_UID[2]= pBaseConfig->uid[2];
	Cosmos_to_tele.Dicor_UID[3]= pBaseConfig->uid[3];
	Cosmos_to_tele.pack_obj=0x01;
	Cosmos_to_tele.pack_type=cmd;
	Cosmos_to_tele.state=state;
	_mem_zero(Cosmos_to_tele.Data_Buffer, sizeof(Cosmos_to_tele.Data_Buffer));
	Cosmos_to_tele.addr=id;
	if (SERVER_MODE == 2)  
		send(upload_buffer.sock_mux,(char_ptr)(&Cosmos_to_tele.Dicor_UID[0]), sizeof(Cosmos_to_tele), 0);   
	send(upload_buffer.sock,(char_ptr)(&Cosmos_to_tele.Dicor_UID[0]), sizeof(Cosmos_to_tele), 0); 
	
	printf("\n[0x%02x] info to tele", cmd);
}

int_32 DETECT_ISP_CMD(uint_8 *w_buf, uint_16 len_buf, uint_16 re_delay, uint_8 re_wtimes)
{
	uint_8 i;
	uint_8 *rx_data;
	uint_16 re_delay_times = re_delay*2;
	rx_data = RS485_Data_Buffer;
	
	for(i=0; i<=re_wtimes; i++)
	{	
		rs485_clear_buf();
		rs485_write(w_buf, len_buf);
		//printf ("\n______%d______\n", len_buf);
		//rs485_write(w_buf, strlen(w_buf));
		while(re_delay_times)
		{
			_mem_zero(rx_data, sizeof(rx_data));
			_time_delay(50);//100
			rs485_read(rx_data);
			//printf("%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\n",rx_data[0],rx_data[1],rx_data[2],rx_data[3],rx_data[4],rx_data[5],rx_data[6],rx_data[7],rx_data[8],rx_data[9],rx_data[10],rx_data[11]);
			if ((rx_data[0]==0x00)&&(rx_data[1]==w_buf[1]))
			{
				if ((rx_data[2]==w_buf[2])&&(rx_data[5] ==0x00))
				{
					printf("\n[0x%04x] replies CMD(0x%02x) succeed! STATE_Code:0x%02x",w_buf[4]|w_buf[3]<<8,w_buf[2],rx_data[5]);
					return 1;	
				}
				else if(rx_data[2]==(w_buf[2]|0x80))//if ((rx_data_buffer[0]==0x00)&&(rx_data_buffer[1]==data_buffer[1])&&(rx_data_buffer[5] ==0x0B)&&(rx_data_buffer[2]==(tmp_buf[2]+0x80)))
				{
					printf("\n[0x%04x] replies CMD(0x%02x) timeout! EXC_Code:0x%02x",w_buf[4]|w_buf[3]<<8,w_buf[2],rx_data[5]);
				}
				break;
			}
			re_delay_times--;
		}
		if((rx_data[1]==0x00)&&(rx_data[5] ==0x00))
		{
			printf("\n[0x%04x] replies CMD(0x%02x) No reply!",w_buf[4]|w_buf[3]<<8,w_buf[2]);
			printf("\n*********************************************************");
			rx_data[5]=0x01;
		}
		re_delay_times = re_delay*2;		
		if(i == re_wtimes)
		{
			SEND_INFO_DEL(w_buf[4]|w_buf[3]<<8, w_buf[2], rx_data[5]);
			//send_info_del(rx_data);
			return -1;
		}		
		printf("\nre_write_times = %d", i+1);
	}
}


int_8 DETECT_MESAGE_TIME_OUT_Command(uint_8* data_buffer,uint_8 len,uint_16 delay_time,uint_16 addr)
{
	uint_8 i;	
	uint_16 b_len;
	uint_16 retry ;//= 2;
	_mqx_int recv_len;
	uint_8 tmp_buf[100];
	uint_8 *rx_data_buffer;
	b_len = recv_len;
	tmp_buf[2]=data_buffer[2];
	retry = 30*delay_time;
	rx_data_buffer = RS485_Data_Buffer;
	
 	printf("\nREQ :");
 	for (i = 0; i < len; i++)
 	{
 		printf("%02x ",data_buffer[i] );
 	}
 	
	for(i=0 ; i<=RETRYTIMES; i++)
	{
		while(retry)
		{
			_mem_zero(rx_data_buffer, sizeof(rx_data_buffer));
			_time_delay(300);
			rs485_read(rx_data_buffer);
			//printf("%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\n",rx_data_buffer[0],rx_data_buffer[1],rx_data_buffer[2],rx_data_buffer[3],rx_data_buffer[4],rx_data_buffer[5],rx_data_buffer[6],rx_data_buffer[7],rx_data_buffer[8],rx_data_buffer[9],rx_data_buffer[10],rx_data_buffer[11]);
			if ((rx_data_buffer[0]==0x00)&&(rx_data_buffer[1]==data_buffer[1]))
			{
				if ((rx_data_buffer[2]==tmp_buf[2])&&(rx_data_buffer[5] ==0x00))
				{
					printf("\n[0x%04x] replies CMD(0x%02x) succeed! STATE_Code:0x%02x",addr,tmp_buf[2],rx_data_buffer[5]);
				}
				else if(rx_data_buffer[2]==(tmp_buf[2]|0x80))//if ((rx_data_buffer[0]==0x00)&&(rx_data_buffer[1]==data_buffer[1])&&(rx_data_buffer[5] ==0x0B)&&(rx_data_buffer[2]==(tmp_buf[2]+0x80)))
				{
					printf("\n[0x%04x] replies CMD(0x%02x) timeout! EXC_Code:0x%02x",addr,tmp_buf[2],rx_data_buffer[5]);
				}
				printf("\nRSP :%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x",rx_data_buffer[0],rx_data_buffer[1],rx_data_buffer[2],rx_data_buffer[3],rx_data_buffer[4],rx_data_buffer[5],rx_data_buffer[6],rx_data_buffer[7],rx_data_buffer[8],rx_data_buffer[9],rx_data_buffer[10],rx_data_buffer[11]);
				printf("\n*********************************************************\n");										
				break;
			}
			retry--;
		}
		if((rx_data_buffer[1]==0x00)&&(rx_data_buffer[5] ==0x00))
		{
			printf("\n[0x%04x] replies CMD(0x%02x) No reply!\n",addr,tmp_buf[2]);
			printf("\nRSP :%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x",rx_data_buffer[0],rx_data_buffer[1],rx_data_buffer[2],rx_data_buffer[3],rx_data_buffer[4],rx_data_buffer[5],rx_data_buffer[6],rx_data_buffer[7],rx_data_buffer[8],rx_data_buffer[9],rx_data_buffer[10],rx_data_buffer[11]);
			printf("\n*********************************************************\n");
			rx_data_buffer[5]=0x01;
		}
	}

	printf("\n");	 
	Dimo_ht_data.Dicor_UID[0]= pBaseConfig->uid[0];
	Dimo_ht_data.Dicor_UID[1]= pBaseConfig->uid[1];
	Dimo_ht_data.Dicor_UID[2]= pBaseConfig->uid[2];
	Dimo_ht_data.Dicor_UID[3]= pBaseConfig->uid[3];
	Dimo_ht_data.pack_obj=Dimo_response_data.pack_obj;
	Dimo_ht_data.pack_type=tmp_buf[2];
	Dimo_ht_data.state=rx_data_buffer[5];
	Dimo_ht_data.Data_Buffer[0]=rx_data_buffer[0];
	Dimo_ht_data.Data_Buffer[1]=rx_data_buffer[1];
	Dimo_ht_data.Data_Buffer[2]=rx_data_buffer[2];
	Dimo_ht_data.Data_Buffer[3]=rx_data_buffer[3];
	Dimo_ht_data.Data_Buffer[4]=rx_data_buffer[4];
	Dimo_ht_data.Data_Buffer[5]=rx_data_buffer[5];
	Dimo_ht_data.Data_Buffer[6]=rx_data_buffer[6];
	Dimo_ht_data.Data_Buffer[7]=rx_data_buffer[7];
	Dimo_ht_data.Data_Buffer[8]=rx_data_buffer[8];
	Dimo_ht_data.Data_Buffer[9]=rx_data_buffer[9];
    Dimo_ht_data.addr =addr;
     if (SERVER_MODE == 2)  
     	send(upload_buffer.sock_mux,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]), sizeof(DIMO_HT_DATA), 0);    
     send(upload_buffer.sock,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]), sizeof(DIMO_HT_DATA), 0);  
 }

//ENTER_IAP_MODE_Command
void OPT_Modbus_ENTER_IAP_MODE_Command(uint_16 addr_f,uint_16 addr_r,uint_8* data_buffer)
{
	
		uint_16 crc;
		uint_8 len;
	//	rs485_read(data_buffer);
		//data_buffer[0]=(uint_8)(addr_f>>8)=0x01;
	    data_buffer[0]=0x80;
  	    data_buffer[1]=CoodAddr;
	    data_buffer[6]= (uint_8)addr_r;
	    data_buffer[5]= (uint_8)(addr_r>>8);
	//	data_buffer[3]=(uint_8)(addr_r>>8)=0x02;
	    data_buffer[4]=(uint_8)addr_f;
	//	data_buffer[4]=(uint_8)addr_r=0x02;
	    data_buffer[3]=(uint_8)(addr_f>>8);
	   	data_buffer[2]=0x21;
	    
	    
		crc = Modbus_calcrc16(data_buffer, 7);
		data_buffer[7] = (uint_8)crc;
	    data_buffer[8] = (uint_8)(crc>>8);
		rs485_write(data_buffer, 9);
	 	len=9;
 DETECT_MESAGE_TIME_OUT_Command(data_buffer,len,1,addr_f); 
}

//ERASE_FLASH_Command
void OPT_Modbus_ERASE_FLASH_Command(uint_16 addr_f,uint_16 addr_r,uint_8* data_buffer)
{
		uint_16 crc;
		uint_8 len,temp_Buf[150];
	//	rs485_read(data_buffer);
		//data_buffer[0]=(uint_8)(addr_f>>8)=0x01;
	    temp_Buf[0]=0x80;
     	temp_Buf[1]=CoodAddr;
       temp_Buf[6]= (uint_8)addr_r;
	    temp_Buf[5]= (uint_8)(addr_r>>8);
	//	data_buffer[3]=(uint_8)(addr_r>>8)=0x02;
	    temp_Buf[4]=(uint_8)addr_f;
	//	data_buffer[4]=(uint_8)addr_r=0x02;
	    temp_Buf[3]=(uint_8)(addr_f>>8);
	    temp_Buf[2]=0x22;
	    
		crc = Modbus_calcrc16(temp_Buf, 7);
		temp_Buf[7] = (uint_8)crc;
	    temp_Buf[8] = (uint_8)(crc>>8);
		rs485_write(temp_Buf, 9);
		 	len=9;
 DETECT_MESAGE_TIME_OUT_Command(temp_Buf,len,1,addr_f); 
}

//WRITE_FLASH_Command
void OPT_Modbus_WRITE_FLASH_Command(uint_16 addr_f,uint_16 addr_r,uint_8* data_buffer,uint_8 readlen)
{
    
 uint_16 crc;
 uint_8 i,temp_Buf[150];
  
    for(i=0;i<readlen-1;i++)
	{
	temp_Buf[3+i]=data_buffer[2+i];
	}
//	data_buffer[3]=(uint_8)(ADDR_R>>8);     //data_buffer[4];
//	data_buffer[4]=(uint_8)ADDR_R;    //data_buffer[5];
	//dat_buffer[5]=0x00;
	 temp_Buf[0]=0x80;
	temp_Buf[1]=CoodAddr;
	temp_Buf[2]=0x23;
    crc = Modbus_calcrc16(temp_Buf, readlen+2);
	temp_Buf[readlen+2] = (uint_8)crc;
    temp_Buf[readlen+3] = (uint_8)(crc>>8);
	rs485_write(temp_Buf, readlen+4);
	 
 //DETECT_MESAGE_TIME_OUT_Command(temp_Buf,readlen+2,1);  
}

//END_IAP_Command
void OPT_END_IAP_Command(uint_16 addr_f,uint_16 addr_r,uint_16 crc,uint_8* data_buffer)
{
	uint_16 crc_end;
	data_buffer[0]=0x80;
	data_buffer[1]=CoodAddr;
	data_buffer[6]= (uint_8)addr_r;
	data_buffer[5]= (uint_8)(addr_r>>8);
	data_buffer[4]=(uint_8)addr_f;
	data_buffer[3]=(uint_8)(addr_f>>8);
	data_buffer[2]=0x24;
	data_buffer[7]=(uint_8)(crc>>8);  //0x11
	data_buffer[8]=(uint_8)crc;       //0x11
	crc_end = Modbus_calcrc16(data_buffer, 9);
	data_buffer[9] = (uint_8)crc_end;
	data_buffer[10] = (uint_8)(crc_end>>8);
	rs485_write(data_buffer, 11);
	DETECT_MESAGE_TIME_OUT_Command(data_buffer,11,1,addr_f); 	
}


//Modbus发送读取保持寄存器的命令后取回应数据，并解析数据
int_8 Modbus_RecvReadRegData(uint_16 addr2, uint_8* data_buffer, uint_8* dest)
{
	uint_8 retry = 30;
	_mqx_int recv_len;
	uint_16 crc;
	uint_8 crch, crcl,cc_flag;
	uint_16 len = 0;
	uint_8 result = 0;
	DATE_STRUCT date;
	uint_8 i;
	uint_16 b_len;
	uint_16 index = 0;
	MCF5225_GPIO_STRUCT_PTR rs485_gpio_ptr;	
	MQX_FILE_PTR rs485_dev = NULL;
	data_buffer = RS485_Data_Buffer;
	b_len = recv_len;	
	//b_len = rs485_read(data_buffer);
	//printf("%x,%x,%x,%x,%x,%x\n",data_buffer[0],data_buffer[1],data_buffer[2],data_buffer[3],data_buffer[4],data_buffer[5]);
/****************************************************/
	while(retry)
	{
		retry--;
		_mem_zero(data_buffer, sizeof(data_buffer));
		_time_delay(1000);
		b_len = rs485_read(data_buffer);	
		//printf("\n%x,%x,%x,%x,%x,%x",data_buffer[0],data_buffer[1],data_buffer[2],data_buffer[3],data_buffer[4],data_buffer[5]);
		if ((data_buffer[0]==0x00)&&(data_buffer[1]==CoodAddr)&&(data_buffer[2]==0x04||data_buffer[2]==0x84))
		{
			break;
		}	
	}
/***************************************************/

	if (b_len > 16)
	{
		index = 0;
		while ((data_buffer[index] != 0x00)&&(data_buffer[index+1] != CoodAddr))
		{
			index++;
			b_len--;
			if (b_len == 0)
			{
				sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t Modbus协议解析数据失败，没有找到数据帧头\r\n", date.HOUR,date.MINUTE,date.SECOND);
				sprintf(pDiCorLog->logbuf, "\r\n");
				return -1;
			}
			else
			 	return -1;
		}
				
		//校验数据
		if ((data_buffer[MODBUS_RECV_BYTE_ADDRESS_H+index]==0x00) &&(data_buffer[index+1] == CoodAddr)&&(data_buffer[MODBUS_RECV_BYTE_FUNCTION+index]==MODBUS_FUNCTION_READ_REGISTER))
		{
		    len = b_len-2;
			crc = Modbus_calcrc16(data_buffer+index, len);	
			crch = (uint_8)(crc>>8);
			crcl = (uint_8)crc;
			if ((crcl==data_buffer[len+index]) && (crch==data_buffer[len+1+index]))
			{
				//CRC校验成功
				//开始解析数据
				memcpy(dest, data_buffer+4+index, data_buffer[len-4 +index]);
			    printf("\n[0x%04X] replies CMD(0x04) succeed!\n", addr2);
			   	printf("RSP Frame:%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x\n",data_buffer[0],data_buffer[1],data_buffer[2],data_buffer[3],data_buffer[4],data_buffer[5],data_buffer[6],data_buffer[7],data_buffer[8],data_buffer[9],data_buffer[10],data_buffer[11]);
			    printf("*********************************************************\n");
                printf("Cosmos gather [0x%04X] parameters succeed!\n", addr2);
                printf("Send parameters to data center\n");
                printf("*********************************************************\n");
			   	RfLedOn();
			   	SetParameterData.flag=0;
			}
			else
			{   
				printf("Modbus 数据CRC校验错误！\n");
				printf("databuffer0:%x\n",data_buffer[0]);
				printf("databuffer1:%x\n",data_buffer[1]);
				printf("databuffer2:%x\n",data_buffer[2]);
	        	printf("databuffer3:%x\n",data_buffer[3]);
	           	printf("databuffer4:%x\n",data_buffer[4]);
				printf("databuffer5:%x\n",data_buffer[5]);
				printf("databuffer6:%x\n",data_buffer[6]);
		       	printf("databuffer7:%x\n",data_buffer[7]);
		       	printf("databuffer8:%x\n",data_buffer[8]);
		       	printf("databuffer9:%x\n",data_buffer[9]);
		       	printf("databuffer10:%x\n",data_buffer[10]);
		       	printf("databuffer11:%x\n",data_buffer[11]);
	       		printf("crcl:%x\n",crcl);
				printf("data_buffer[len+index]:%x\n",data_buffer[len+index]);
				printf("crch:%x\n",crch);
				printf("data_buffer[len+1+index]:%x\n",data_buffer[len+1+index]);
				dicor_get_logtime(&date);
		
				sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t Modbus 数据CRC校验错误！\r\n", date.HOUR,date.MINUTE,date.SECOND);
				_time_delay(1000*retry);
				return -1;				 
			}
		}
		else
		{
			dicor_get_logtime(&date);
          	printf("databuffer0:%x\n",data_buffer[0]);
			printf("databuffer1:%x\n",data_buffer[1]);
			printf("databuffer2:%x\n",data_buffer[2]);
	       	printf("databuffer3:%x\n",data_buffer[3]);
	       	printf("databuffer4:%x\n",data_buffer[4]);
			printf("databuffer5:%x\n",data_buffer[5]);
			printf("databuffer6:%x\n",data_buffer[6]);
	       	printf("databuffer7:%x\n",data_buffer[7]);
	       	printf("databuffer8:%x\n",data_buffer[8]);
	       	printf("databuffer9:%x\n",data_buffer[9]);
	       	printf("databuffer10:%x\n",data_buffer[10]);
	       	printf("databuffer11:%x\n",data_buffer[11]);
			printf("Modbus 数据地址校验不成功！\n");
			RfLedOff();
			_time_delay(1000*retry);
			return -1;		 
		 }
	}
	
	else
	{
    	RfLedOff();
    	if ((data_buffer[index+2]!=0x00) &&(data_buffer[index+1] == CoodAddr))
    	{
    		
    	    printf("\n[0x%04X]replies CMD(0x04) timeout!\n", addr2);
		   	printf("RSP Frame:%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x\n",data_buffer[0],data_buffer[1],data_buffer[2],data_buffer[3],data_buffer[4],data_buffer[5],data_buffer[6],data_buffer[7],data_buffer[8],data_buffer[9],data_buffer[10],data_buffer[11]);
		    printf("*********************************************************\n");
            printf("Cosmos gather the device[0x%04X] parameters timeout!\n", addr2);
            printf("Send 00 00 to data center\n");
            printf("*********************************************************\n");
    	}
        else 
		{
			printf("\n[0x%04X]replies CMD(04) no response!\n", addr2);
			printf("RSP Frame:%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x\n",data_buffer[0],data_buffer[1],data_buffer[2],data_buffer[3],data_buffer[4],data_buffer[5],data_buffer[6],data_buffer[7],data_buffer[8],data_buffer[9],data_buffer[10],data_buffer[11]);
			printf("*********************************************************\n");
			printf("Cosmos gather the device[0x%04X] parameters Unsuccessfully!\n", addr2);
			printf("Send 00 00 to data center\n");
			printf("*********************************************************\n");	
//modify by vitor
			SetParameterData.flag++;
			printf("No response*%x\n",SetParameterData.flag);			
		}
		if(SetParameterData.flag==4) 
		{
			printf("\nRESET CC2530\n");	
			SetParameterData.flag=0;
			fclose(rs485_dev);
			fflush(rs485_dev); 
			GPIO_PULLDOWN();
			_time_delay(30000);
			RESET_CC2530();
			_time_delay(30000);			
			Dicor_Reboot();			
			N_RESET_CC2530();
			_time_delay(30000);
			rs485_init();
		}
//modify by ywt			
		_time_delay(1000*retry);
		return -1;
	}
	_time_delay(1000*retry);
	return 0;
}

/* EOF */
