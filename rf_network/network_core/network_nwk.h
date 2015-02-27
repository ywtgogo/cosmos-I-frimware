#ifndef _network_nwk_h_
#define _network_nwk_h_
/*****************************************************
* 
* 	Copyright (c) 2010  Convertergy;
* 	All Rights Reserved
* 	FileName:  network_nwk.h
* 	Version : 
* 	Date    : 
* 	Ower   : peter li
*
* Comments:
********************************************************/
#include "network_frame.h"
#include "network_phy.h"
#include "network_dlc.h"   

//router table operations
#define ROUTER_TABLE_ADD_ITEM   (1)
#define ROUTER_TABLE_REMOVE     (2)


#define  EMN_NWK_TXBUF_COUNT    (4)//the max number of mac frame is 8

#define RF_UPGRADE_BEGIN_INDEX   (12)


//for test lost packet
#define COUNT_LOSTPACKET  1

typedef enum 
{
#if EMN_EMT_ONLY
 EMN_NWK_CORD_DIS_INIT,
 EMN_NWK_CORD_DIS_CHECK,
 EMN_NWK_CORD_DIS_SCAN,
 EMN_NWK_CORD_CON_SCAN,
 EMN_NWK_CORD_CON

// disconnect state
#else
 EMN_NWK_DIS_UNKNOW,
 EMN_NWK_SCAN_HEADER,
 EMN_NWK_SCAN_DEVICE,

// connnect state
 EMN_NWK_CON_DEVICE, 
 EMN_NWK_CON_HEADER 

#endif 
}EMN_NWK_STATE;
#define EMN_UIN_BEGIN_BIT               (7)
#define EMN_UOUT_BEGIN_BIT              (11)
#define EMN_SUBNET_BEGIN_BIT            (2)
#define EMN_DEV_BEGIN_BIT               (3)


#define EMN_UIN_UOUT_D_VALUE            (1)
#define EMN_UIN_MT_UOUT                  (1)
#define EMN_UIN_LT_UOUT                  (2)
#define EMN_UIN_UOUT_OTHER               (3)

//register table management
#define EMN_HEADER_MAX_CHILDREN    (32)
#define EMN_COORD_MAX_CHILREN      (256)
#define EMN_NET_MAX_DEPTH          (4)
#define EMN_SUBNET_MAX             (20)
#if EMN_EMT_ONLY
typedef struct{
  NET_ADDR  subnet_addr;
  UINT_8    up_hop;
  UINT_8    num_child;
  UINT_8    depth;
  UINT_8    uid[4];
  UINT_16   child_addr;// the address of the first children 
}EMN_REG_1TAB_ST;
typedef struct{
  DEV_ADDR  dev_addr;
  UINT_8    uid[4];
  UINT_16   next;
}EMN_REG_2TAB_ST;

typedef struct{
  UINT_8 net_addr;
  UINT_8 dev_addr;
  UINT_8 uid[4];
}EMN_DEV_REG_INFO_ST;
#endif 
extern void EMN_NWK_Task(SYS_MESSAGE* msg);	
extern void EMN_NWK_TaskInit(void);	

extern  UINT_8   NWK_ComBufIndex;
extern  UINT_8   NWK_ComBuf[EMN_PHY_PAYLOAD_LEN];

//Tx buffer
extern UINT_8  NWK_TxBufIndex;
extern UINT_8  NWK_TxBufFrmNum;
extern UINT_8  NWK_TxBuf[EMN_NWK_TXBUF_COUNT][EMN_PHY_PAYLOAD_LEN];

extern UINT_8  RF_Data_CurIndex;
extern UINT_8  *RF_Data_From;
extern UINT_8  NWK_UPGRADE_TxBufFrmNum;
extern void NWK_WriteSampleData2(UINT_8 *data);
#endif

