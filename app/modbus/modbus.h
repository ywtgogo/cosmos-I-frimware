/*************************************************************
* 
* Copyright (c) convertergy ;
* All Rights Reserved
* FileName: modbus.h
* Version : 
* Date    : 2012/04/12
* ower    : Younger
*
* Comments:
*
*
***************************************************************/


#ifndef __MODBUS_H__
#define __MODBUS_H__

//#define OPTIMUS 1

#ifdef	OPTIMUS
	#define	GATEWAY_TYPE						0x42	
#else
	#define GATEWAY_TYPE						0x46
#endif

#define		MODBUS_FUNCTION_READ_REGISTER		0x04
//#define     MODBUS_FUNCTION_READ_PORT_COLLECT_REGISTER  0x04
//#define     COODADDRDEFAULT                     0x01
#define		MODBUS_RECV_BYTE_ADDRESS_H			0
#define		MODBUS_RECV_BYTE_ADDRESS_L			1
#define		MODBUS_RECV_BYTE_FUNCTION			2
#define		MODBUS_RECV_BYTE_DATACOUNT1	 	    3
#define		MODBUS_RECV_BYTE_DATACOUNT2	 	    4
#define     RETRYTIMES                          0
void Modbus_SendStartCommand(uint_16 addr,uint_8* start_buffer);
void Modbus_SendReadyCommand(uint_16 addr,uint_8* ready_buffer);
//unsigned short Modbus_calcrc16(unsigned char *buf, unsigned short len);
void Read_Data_Command(uint_16 addr1,uint_8* data_buffer,uint_16 readstart, uint_16 readlen);
int Modbus_SendReadRegCommand(uint_16 addr1, uint_8* data_buffer, uint_16 readstart, uint_16 readlen);
int_8 Modbus_RecvReadRegData(uint_16 addr2, uint_8* data_buffer, uint_8* dest);
int_32 Modbus_ENTER_IAP_MODE_Command(uint_16 addr_f,uint_16 addr_r,uint_8* data_buffer);
int_32 Modbus_ERASE_FLASH_Command(uint_16 addr_f,uint_16 addr_r,uint_8* data_buffer);
int_32 Modbus_WRITE_FLASH_Command(uint_16 addr_f,uint_16 addr_r,uint_8* data_buffer,uint_8 readlen);
int_32 END_IAP_Command(uint_16 addr_f,uint_16 addr_r,uint_16 crc,uint_8* data_buffer);

void OPT_Modbus_ENTER_IAP_MODE_Command(uint_16 addr_f,uint_16 addr_r,uint_8* data_buffer);
void OPT_Modbus_ERASE_FLASH_Command(uint_16 addr_f,uint_16 addr_r,uint_8* data_buffer);
void OPT_Modbus_WRITE_FLASH_Command(uint_16 addr_f,uint_16 addr_r,uint_8* data_buffer,uint_8 readlen);
void OPT_END_IAP_Command(uint_16 addr_f,uint_16 addr_r,uint_16 crc,uint_8* data_buffer);

void Write_Magnus_Parameter_CMD(uint_16 addr_f,uint_16 addr_r,uint_8* data_buffer);
void Read_Magnus_Parameter_CMD(uint_16 addr_f,uint_16 addr_r,uint_8* data_buffer);
int_8 Modbus_RecvReadParameter(uint_16 addr,uint_8* data_buffer);

void SET_FFD_TABLE_Command(uint_16 addr,uint_8* data_buffer);
void SET_MAGNUS_ROLE_Command(uint_16 addr,uint_8* data_buffer);
void MODIFY_FFD_NODE_NUM_Command(uint_16 addr,uint_8* data_buffer);
void FFD_BIND_RFD_Command(uint_16 addr,uint_8* data_buffer);
void FFD_START_COLLECT_Command(uint_16 addr,uint_8* data_buffer);
void OPTIMUS_SYNC_Command(uint_16 addr,uint_8* data_buffer);
void MAGNUS_SHUTDOWN_Command(uint_16 addr,uint_8* data_buffer);
void FFD_STOP_COLLECT_Command(uint_16 addr,uint_8* data_buffer);
void SW_Command(uint_16 addr,uint_8* data_buffer);
void TURN_ON_Command(uint_16 addr,uint_8* data_buffer);
void SET_ROUTER_TABLE_Command(uint_16 addr,uint_8* data_buffer);
extern uint_8 cc_flag;
int_32 MAGNUS_LOAD_PREAMBLE_Command(uint_16 addr_f,uint_16 addr_r,uint_8* data_buffer,uint_16 version);
unsigned short Modbus_calcrc16(unsigned char *buf, unsigned short len); 
int_32 SEND_INFO_DEL(uint_16 id, uint_8 cmd, uint_8 state);
int_8 DETECT_MESAGE_TIME_OUT_Command(uint_8* data_buffer,uint_8 len,uint_16 delay_time,uint_16 addr);
int_8 DETECT_POLL_TIME_OUT_Command(uint_8* data_buffer,uint_8 len,uint_16 delay_time,uint_16 addr);
int_32 DETECT_ISP_CMD(uint_8 *w_buf, uint_16 len_buf, uint_16 re_delay, uint_8 re_wtimes);
//int_8 DETECT_MESAGE_TIME_OUT_Command_short(uint_8* data_buffer,uint_8 len);
#endif
/* EOF */
