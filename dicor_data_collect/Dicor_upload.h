#ifndef __dicor_upload_h__
#define __dicor_upload_h__
/**********************************************************************
* 
* Copyright (c) 2010 convertergy ;
* All Rights Reserved
* FileName: dicor_upload.h 
* Version : 
* Date    : 
*
* Comments:
*
*   This file contains definitions for the upload MG's data to data center
*
*************************************************************************/
#include <message.h>
#include "dicor_network_apl.h"
#include "dicor.h"
#include "mutex.h"


//#define  SERVER_DATA_CENTER              IPADDR(192,168,8,108) 
//#define  SERVER_DATA_CENTER              IPADDR(192,168,8,160) 
//#define  IPPORT_DATA_CENTER              (1018)
//#define  IPPORT_DATA_CENTER              (8081)

//#define  DICOR_UPLOAD_BUFFER_SIZE        (32*20)
#define  DICOR_UPLOAD_BUFFER_SIZE       (51*22)
#define MAXRFBUFFFILLSECONDS				70		//60S内RF数据缓冲区不满LED灯灭
#define  ADDR_F       0x0002
#define  ADDR_R       0x0003
//单个文件的最大文件大小
#define FILEMAXSIZE		2*1024*1024
//#define FILEMAXSIZE		1024



//signal definition

#define  RF_REG_END           (0x00000001)     
#define  RF_GET_DATA_END      (0x00000002)  
#define  NET_SEND_DATA_END    (0x00000004)   
#define  RF_REG_START         (0x00000008)
#define  RFNET_EVENT		  (0x00000008)	//add by dick

#define THIRD_DEVICE_DATA_END	(0x00000010)
#define THIRD_DEVICE_DATA_START	(0x00000020)



typedef struct collect_data1{
      uint_8    type;//0xaa register map table, 0x11:sample data frame
      uint_8    dicor_id[DICOR_ID_LEN];      
      uint_8    dumy[1];
	  uint_16   num; 	  
      uint_8    didi_data[DICOR_UPLOAD_BUFFER_SIZE]; 
}DICOR_PACKET1,_PTR_ DICOR_PACKET_PTR1;  

typedef struct collect_data2{
      uint_8    type;//0xaa register map table, 0x11:sample data frame
      uint_8    dicor_id[DICOR_ID_LEN];
	  uint_8    flag;//00:continue, 11last
	  uint_8    index;// index of packet
	  uint_8    num;    
}DICOR_REG_H,_PTR_ DICOR_REG_H_PTR;  

typedef enum 
{
   ETH_NO_INIT,
   ETH_INIT,   	
   ETH_CABLE,
   ETH_CABLE_IP,
   ETH_CABLE_IP_CON   
}ETHERNET_ST;


typedef enum 
{
   CAN_WRITE,
   WRITTING,   	
   CAN_READ,
   READING     
}UPLOAD_BUF_ST;

typedef struct collect_data
{
	   ETHERNET_ST    eth_st; 
   	   uint_32        write_index; 
       uint_32 		  sock;
       uint_32		  sock_mux;
	   UPLOAD_BUF_ST  state;  
	   MUTEX_STRUCT	  mutex;	   
       DICOR_PACKET1  data0;
} DIDI_DATA_BUFFER, _PTR_ DIDI_DATA_BUFFER_PTR;


//extern uchar  UP_recvData_Buffer[256];

typedef enum 
{
   RF_NWK_IDLE,
   RF_NWK_IDLE_NO_DIDI_WORKNG, //
   RF_NWK_IDLE_NO_LAN,
   RF_NWK_DIS,
   RF_NWK_REGING,
   RF_NWK_REGING1
}RF_NWK_STATE;
extern RF_NWK_STATE  rfnwk_state;
extern DIDI_DATA_BUFFER    upload_buffer;
//extern const UINT_8  RegTable[];

extern UINT_8* RegTable;
//data definitions from DiMo
typedef struct 
{
  UINT_8  Dicor_UID[4];
  UINT_8  pack_obj;
  UINT_8  pack_type;
  UINT_8  role;
 // UINT_16 status;
  UINT_8  Data_Buffer[128];
 // uint_8 *data_buffer;
} DIMO_RES_DATA, _PTR_  DIMO_RES_DATA_PTR;


typedef struct 
{
  UINT_8  Dicor_UID[4];
  UINT_16 pack_type;
  UINT_16 status;
} DIMO_HB_DATA, _PTR_  DIMO_HB_DATA_PTR;


typedef struct 
{
  UINT_8  Dicor_UID[4];
  UINT_8  pack_obj;
  UINT_8  pack_type;
  UINT_8  state;
  UINT_8  Data_Buffer[23];
  UINT_16 addr;
} DIMO_HT_DATA, _PTR_  DIMO_HT_DATA_PTR;


extern void  Downloadconfigfile(int_32 argc, char_ptr argv[]);

typedef struct 
{
	UINT_8 type;        //类型 , 0x40:DiCor
	UINT_8 len;         //表示整个结构的大小，例如DiAn的这个结构为len=16
	UINT_8 subnet;
	UINT_8 dev;
	UINT_8 sd_status;	//SD卡状态
	UINT_8 rf_status;	//RF状态
	

}EMN_DICOR_DATA_ST;

//from DiMo
#define DIMO_EVERTHING_OK                     (0X0000)
#define DIMO_DIMO_ERROR                       (0X0001)
#define DIMO_REG_TABLE_ERROR                  (0X0002)
#define DIMO_DB_MANGETMENT_ERROR              (0X0003)
#define DIMO_DICOR_DATA_PACKET_ERROR          (0X0004)
void dicor_get_time(DATE_STRUCT * date);
extern void dicor_waite_rf(uint_8 flag,uint_32 timeout);
extern void dicor_rf_signal(uint_32 flag);
extern int_32 dicor_kill_task(char* taskname);
extern int_32 dicor_create_task(char* taskname);
extern DIMO_RES_DATA Dimo_response_data;
//DIMO_RES_DATA Dimo_response_data;
extern DIMO_HB_DATA  Dimo_hb_data;  
extern DIMO_HT_DATA  Dimo_ht_data; 
extern DIMO_HT_DATA  Cosmos_to_tele;

extern _mem_pool_id _user_pool_id;

#endif
/* EOF */
