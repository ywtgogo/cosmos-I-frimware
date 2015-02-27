#ifndef _network_phy_h_
#define _network_phy_h_
/***********************************************************
* 
* 	Copyright (c) 2010  Convertergy;
*	 All Rights Reserved
*	 FileName: network_phy.h
*	 Version : 
*	 Date    : 
* 	 Ower   : peter li
*
*      Comments:
*
**************************************************************/
#include "network_frame.h"

//#define PHY_TX_OK        (0)
//#define PHY_TX_ERROR     (1) 
//#define PHY_TX_CD        (2)

/****************************
  *
  *  definitions of PHY layer
  *
  ***************************/
//working  mode 
typedef enum{
  POWER_DOWN_MODE = 0x00,       // chip not power up 
  STANDBY_MODE=0x01,                 // hibernate mode
  RX_MODE,                      // receiver mode
  TX_MODE,                      // transmit mode
  CCA_MODE                     // carrier channel allocation 
}EMN_PHY_ST;


/**************************
  *  the interfaces of output
  **************************/
	
// For low overhead, system use queue for RX
#define  EMN_PHY_PAYLOAD_LEN     (32)
#define  EMN_PHY_RXBUF_COUNT     (10)
typedef struct 
{
  UINT_8 count;
  UINT_8 head;
  UINT_8 tail;
  UINT_8 data[EMN_PHY_RXBUF_COUNT][EMN_PHY_PAYLOAD_LEN];
} EMN_PHY_RX_BUF;
	
extern EMN_PHY_RX_BUF  EMN_PHY_RxBuf;
extern EMN_PHY_ST      EMN_PHY_State;

extern volatile UINT_8  RF_RxReady;  
extern UINT_8  RF_PhyGetDataEnd;

extern UINT_8  RF_RxOverflow;

extern void EMN_PHY_Task(SYS_MESSAGE *msg);	
extern void EMN_PHY_TaskInit(void);	
extern void PHY_WriteRxAddr(UINT_8 netaddr,UINT_8  sub_netaddr);
extern void RFSimpleTxPacket(uint_8* sendbuf,uint_8 sub, uint_8 netaddr);
extern void RFSimpleRxPacket(uint_8* recvbuf);
#endif

