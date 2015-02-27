#ifndef _network_frame_h_
#define _network_frame_h_
/**********************************************************************
* 
* 	Copyright (c) 2010  Convetergy;
* 	All Rights Reserved
* 	FileName: network_frame.h
*	Version : 
* 	Date    : 2010-11-17
* 	Ower   : peter li
*
* Comments:
*       The main configuration file for entire network implementation.
*   for this project, all definitions in this file is globle
*
*************************************************************************/
#include <mqx.h>
#include <bsp.h>
#include "dicor.h"
#include "eeprom.h"

#ifndef FALSE
  #define  FALSE  0                    /* Boolean value FALSE. FALSE is defined always as a zero value. */
#endif
#ifndef TRUE
  #define  TRUE   1                    /* Boolean value TRUE. TRUE is defined always as a non zero value. */
#endif
	
#ifndef NULL
  #define  NULL   0
#endif


#define EMN_DEBUG               1
#define EMN_EMT_ONLY            1
#define EMN_GET_SAMPLE_WITH_COMMAND         1
#define DI_HARVESTER_V4                    1


#if 0	
typedef signed char    INT_8;
typedef unsigned char  UINT_8;
typedef short 		   INT_16;
typedef unsigned short   UINT_16;
typedef long		   INT_32;
typedef unsigned long  UINT_32;

typedef    long  long       INT_64;
typedef unsigned long long  UINT_64;
#endif 
typedef int_8    INT_8;
typedef uint_8  UINT_8;
typedef  int_16 		   INT_16;
typedef  uint_16   UINT_16;
typedef  int_32		   INT_32;
typedef  uint_32    UINT_32;


typedef unsigned long long  UINT_64;


#define  EMN_OK      (0)
#define  EMN_ERROR   (1)

/*************************
 *  definitions of  timer message ID
  ************* ***************/
#define SYS_TIMEOUT_MAC_RX           (1)
#define SYS_TIMEOUT_MAC_TX           (2)
#define SYS_TIMEOUT_MAC_TX_BACKOFF   (3)
#define SYS_TIMEOUT_NWK_CHECK_REG    (4)   

#define SYS_TIMEOUT_PHY_TX_INTERVAL  (5)  // must wait for a while after send one PHY frame 
#define SYS_TIMEOUT_MAC_WAIT_ACK     (10)  // start it after the last PHY frame of MAC frame       

  
#define SYS_TIMEOUT_RESEND			 (11)  
#define SYS_TIMEOUT_GET_DATA		 (20)   
#define SYS_TIMEOUT_GET_CFM_DATA	 (30)
#define SYS_TIMEOUT_SET_DATA         (31)
#define SYS_TIMEOUT_SET_CFM_STATUS    (32)
#define SYS_TIMEOUT_GET_PARA_DATA	  (33)
#define SYS_TIMEOUT_GET_PARA_CFM_DATA (34)
#define SYS_TIMEOUT_GET_UPDATE_CMD     (35)
#define SYS_TIMEOUT_SEND_DATA_CHECK    (36)				

#define SYS_TIMEOUT_DIS_NET          (21)

#define SYS_TIMEOUT_HAVST_DIAN_CMD	  (50)
#define SYS_TIMEOUT_HAVST_DIDI_CMD    (51)


//MUST CHANGE THESE VALUES

#define EMN_DIDI_NUM       (EMN_DEV_NUM-EMN_DIAN_NUM)


#define EMN_GET_DEV_NUM     (SYS_SUBNET_NUM+EMN_DICOR_SUBDEV_NUM)

#define EMN_DIAN_NUM     (pDiAnDiDiTable->diancount[1])
#define EMN_DEV_NUM        (RegTable[7]) //(0xda)

#define SYS_SUBNET_NUM     (pBaseConfig->subnet_num)// (0x0d)
//#define DICOR_NET_ADDR      (0x22)
//#define EMN_DICOR_SUBDEV_NUM   (0x0)
#define EMN_DICOR_SUBDEV_NUM    (pBaseConfig->subdev_num)


#define SYS_TEST_CONTX      (12)//for test





/*************************
  *  definitions of  timer message ID
  ************* ***************/
#define EMN_TXBUF_NWKCOM      (1)
#define EMN_TXBUF_NWKTX       (2)
#define EMN_TXBUF_MACCOM      (3)
#define EMN_TXBUF_PHYRX       (4) 
#define EMN_TXBUF_MACCOM2      (5) 

 /************************
  *
  * definitions of  message ID
  *
  *********** ***************/
typedef enum
{
// higher layer send command to lower
 EMN_RF_RX_READY,
 
 //messgae between PYH and MAC
 EMN_PHY_TX_REQ,       //request
 EMN_PHY_TX_CFM,       //confirm

 //EMN_PHY_TX_CFM, //confirm
 EMN_PHY_RX_IND,     //indication
 EMN_PHY_RX_RES,     //response

// message between NWK and MAC/DLL
 EMN_MAC_SCAN_REQ,
 EMN_MAC_SCAN_CFM,

 //
 EMN_MAC_DIS_NETWORK_REQ,  //page this message serveral times, not need their repsonse
 EMN_MAC_DIS_NETWORK_CFM,

 EMN_MAC_CON_NET_IND,
 EMN_MAC_CON_NET_RES,

 EMN_SCAN_AGAIN,

// EMN_MAC_DATA_SET_REQ,
 EMN_MAC_DATA_REQ,
 EMN_MAC_DATA_CFM,

 EMN_MAC_DATA_IND,
 EMN_MAC_DATA_RES,
 
 EMN_MAC_END_SCAN_REQ,
 EMN_MAC_END_SCAN_CFM,

//Network layer
 EMN_NWK_CHECK_REG_REQ,
 EMN_NWK_CHECK_REG_CFM,


 EMN_NWK_FIND_NEWER_REQ, //with subnet addr+device addr
 EMN_NWK_FIND_NEWER_CFM,

 EMN_NWK_ALLOC_ADDR_REQ,
 EMN_NWK_ALLOC_ADDR_CFM,

 EMN_NWK_UPDATE_RUOTER_TAB_REQ,
 EMN_NWK_UPDATE_RUOTER_TAB_CFM,

 EMN_NWK_DISCONNECT_RQE,
 EMN_NWK_DISCONNECT_CFM,

//application message 
//no reliable data service
 EMN_NWK_PAGE_DATA_REQ, // get data from chirlren one by one without stop ,from  one ,
 EMN_NWK_PAGE_DATA_CFM, // 

//reliable data service 
 EMN_NWK_GET_SAMPLE_REQ,  
 EMN_NWK_GET_SAMPLE_CFM,




//command, for example, adjust the power parameter to locate the SMC. 
EMN_NWK_START_SET_DATA,//for set didi status 33
EMN_NWK_SET_DATA_REQ,  //33
 EMN_NWK_SET_DATA_CFM,

 EMN_NWK_GET_DATA_REQ,  
 EMN_NWK_GET_DATA_CFM,
	
 
 EMN_NWK_SET_STATUS_REQ,  
 EMN_NWK_SET_STATUS_CFM,
 
 EMN_NWK_START_GET_PARA_DATA,
 EMN_NWK_GET_PARA_REQ, 
 EMN_NWK_GET_PARA_CFM,

 EMN_NWK_END_SETUP_NWK,
 EMN_NWK_START_GET_DATA,

 EMN_APL_START_UPGRADE_CMD,
 EMN_APL_START_UPGRADE_CFM,

 EMN_NWK_UPGRADE_CMD_REQ,
 EMN_NWK_UPGRADE_CMD_CFM,
 
 EMN_NWK_UPGRADE_DATA_REQ,//48
 EMN_NWK_UPGRADE_DATA_CFM,


 EMN_NWK_UPGRADE_FINISH_REQ,
 EMN_NWK_UPGRADE_FINISH_CFM,


 
 EMN_NWK_SEND_DATA_CHECK_REQ,
 EMN_NWK_SEND_DATA_CHECK_CFM,		

 EMN_NWK_DICOR_CONTROL_REQ,
 EMN_NWK_DICOR_CONTROL_CFM,


//Di-harvester
 EMN_HAVST_PARSE_INFO_CMD,
 EMN_HAVST_CONTROL_CMD_REQ,
 EMN_HAVST_CONTROL_CMD_CFM,

 EMN_HAVST4_GET_SAMPLE_CMD_FORMAT1,  //one device 
 EMN_HAVST4_GET_SAMPLE_CMD_FORMAT2,  //some device    command number+dev address#n+dev command#n
 EMN_HAVST4_GET_SAMPLE_CMD_FORMAT3,  //all device        all device number+command field

 EMN_TIMEOUT,    
 EMN_PRIIMITIVE_MAX
}SYS_MESSAGE_ID; 


//#define    EMN_DOWN_TAB_SIZE       (4)


typedef struct{
UINT_8 max;
UINT_8 min;
UINT_8 addr;
}EMN_ROUTER_TAB_ITEM;
#define    EMN_DOWN_TAB_SIZE       (4)
#define    EMN_PARA_SIZE        (sizeof(EepromInRamType))
#define    EMN_UPGRADE_CMD       (1)
#define    EMN_UPGRADE_CMD_SIZE   (2)
#define    EMN_UPGRADE_DATA      (2)
#define    EMN_UPGRADE_FINISH    (3)
#define    EMN_UPGRADE_DATA_SIZE  (113)
#define    EMN_DATA_CHECK_CMD_SIZE (6)
#define    DICOR_UPGRADE_INIT             (1)
#define    DICOR_UPGRADE_SEND_CMD         (2)
#define    DICOR_UPGRADE_SEND_DATA        (3)

#define    EMN_UPGRADE_ROUTER_CMD       (1)
#define    EMN_UPGRADE_NOROUTER_CMD       (2)


          


typedef struct
{ 
  UINT_8  cmd; //if or not broadcast
  UINT_8  dev;
  UINT_32  startAddr;
  UINT_16  len;

  UINT_8  data[1024];
}UpdateType;

typedef struct
{
  UINT_8  brd; //if or not broadcast
  UINT_8  flag;
  UINT_8  index;
  UINT_8  len;
  UINT_8  dev;//set device's dev address
  UINT_8  data[14];
}ParaType;
typedef struct
{
 UINT_8 dev;
 UINT_8 flag;
 UINT_8 index;
 UINT_8 len;
 UINT_8 data[20];
}GetParaType;

#define BROADCAST_FLAG  1
#define NOBROADCAST_FLAG 0

typedef enum
 {
	 EMN_DEVICE,
	 EMN_HEADER,	   // several devices and one header can form one cluster, just as one subnetwork
	 EMN_ROUTER,       // only a header of cluster can act as router.
	 EMN_COORDINATOR,  // there is only one coordinator in one network, in general, EMT can act as this duty
	 EMN_UNKNOW      
 }EMN_DEV_TYPE;

// For EMT as for network coordinator, its address is 0x00;
// For one router, just as cluster header, its address is XXX,XXXXXX,000000B
typedef struct
{
  UINT_8	subnet; 
  UINT_8	dev;
 
}EMN_SUBNET_ADDR;

typedef struct
{
   UINT_8  net;
   UINT_8  subnet;  
   UINT_8  dev;
}EMN_NET_ADDR;
typedef UINT_8 DEV_ADDR;
typedef UINT_8 NET_ADDR;

#include "network_os.h"
// this part should be placed E2PROM
 extern DEV_ADDR	   EMN_DevAddr;
 extern NET_ADDR	   EMN_SubnetAddr; 
 extern EMN_DEV_TYPE   EMN_DevType;
 //extern const UINT_8   EMN_DeviceUID[4];
 extern BASECONFIGTABLE_PTR pBaseConfig;
 extern UINT_8         EMN_NetAddr;
 extern UINT_8         EMN_NetDepth;
 extern UINT_8         EMN_ChildNum;
 extern SYS_MESSAGE    EMN_SendMsg;
// extern EMN_ROUTER_TAB_ITEM   EMN_DownTab[EMN_DOWN_TAB_SIZE];
 extern EMN_ROUTER_TAB_ITEM *EMN_DownTab;
 extern UINT_8* SubnetDepth;  

 //extern UINT_8		          EMN_UpTab; 
 //extern  const  UINT_8   EMN_ScanAddr[4];

 #define  EMN_CONVERTERGY_NO     (0x18)
 #define  EMN_SCAN_FRM_LEN       (0x09)
 
 typedef enum 
 {
   NO_USED,
   SET_PARA,
   HAVST_SEND_CMD
 }SET_PARA_TYPE;
extern  void SYS_Error(UINT_8 m);
 
#endif


