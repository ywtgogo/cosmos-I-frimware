#ifndef _network_dlc_h_
#define _network_dlc_h_
/**********************************************************************
* 
* 	Copyright (c) 2010  Convertery;
* 	All Rights Reserved
* 	FileName: network_dlc.h
* 	Version : 
* 	Date    : 2010-11-14
* 	Ower   : peter li
*
* Comments:  
*           
*
*************************************************************************/
#include "network_frame.h"
#include "network_phy.h"
// the return value of scan 
//#pragma pack(1)

#define EMN_MAC_SCAN_FINISH   (1)
#define EMN_MAC_SCAN_ONE_DEV  (2)

typedef struct 
{
//implement router protocol in MAC layer, so that this more simple
 UINT_8   index     :3;       // one mac frame, include max 4 phsical frame  
 
 UINT_8   last      :1;       //  it is not the last frame  
 UINT_8   need_ack  :1;       // this frame need to response with ack frame
 UINT_8   repeat    :1;	                             
 //UINT_8	  sec	:1;		 // security flag

 UINT_8	 type	    :2;		 //  00: Data frame,
	                         // 01: ack frame
	                         // 10:command frame for setup network
	                         //11:(update router table),//
} EMN_MAC_FRAME_HEAD;
	
typedef struct{
//   EMN_MAC_FRAME_HEAD mac_h;
   UINT_8  com;
   UINT_8  sub_com;
   UINT_8  data[8];
}EMN_MAC_SCAN_FRE;

typedef struct{
  // router mark
  NET_ADDR	last; // ensure that it relay the packet that is should be relay throught this router
  NET_ADDR	next;
  //data mark
  EMN_SUBNET_ADDR	sour;
  EMN_SUBNET_ADDR	dest;   
  
  UINT_8	len;
 UINT_8	data[24];
 // UINT_8	data[24];				 // the is from application layer
//  UINT_8	data[1];				 // the is from application layer
 }EMN_NWK_PACKET_HEAD ;

#define MAC_FRM_SUB_MACH           (0)
#define MAC_FRM_SUB_LAST           (1) 
#define MAC_FRM_SUB_NEXT           (2) 
#define MAC_FRM_SUB_SOUR_SUB       (3) 
#define MAC_FRM_SUB_SOUR_DEV       (4) 
#define MAC_FRM_SUB_DEST_SUB       (5) 
#define MAC_FRM_SUB_DEST_DEV       (6) 
#define MAC_FRM_SUB_LEN            (7) 
#define MAC_FRM_SUB_DATA0          (8) 
#define MAC_FRM_SUB_DATA1          (9)


typedef struct
{
   EMN_MAC_FRAME_HEAD mac_h;
   union{
	  EMN_NWK_PACKET_HEAD  nwk_h;
	  EMN_MAC_SCAN_FRE     scan_frm;
	  UINT_8 data[31]; 
   };
}EMN_MAC_FRAME;

typedef enum 
{
 // master scan state
   EMN_MAC_SCAN_MST0,
   EMN_MAC_SCAN_MST1,
  // EMN_MAC_SCAN_MST2,
   EMN_MAC_SCAN_MST3,
   EMN_MAC_SCAN_MST4,
   EMN_MAC_SCAN_M_END,// for the future improve
   // slave scan state   
   EMN_MAC_SCAN_SST0,
   EMN_MAC_SCAN_SST1,
//   EMN_MAC_SCAN_SST2,
   EMN_MAC_SCAN_SST3,
   EMN_MAC_SCAN_S_END,// for the future improve

   EMN_MAC_CON   //normal state
}EMN_MAC_STATE;
typedef enum{
	NETWORK_MASTER_SCAN,
	NETWORK_SLAVE_SCAN,
	NETWORK_MASTER_NET_MAN
}EMN_MAC_COMMAND;

typedef enum{
  CHALLENGE_UID,    		//mast
  CHALLENGE_UID_ACK, 		//slave
  NETADDR_ALLOC,     		//mast
  NETADDR_ALLOC_ACK,  		
  NETWORK_JOIN,
  NETWORK_DIS
}EMN_MAC_SUB_COMMAND;



/************************************************
  *
  *  All definitions about LLC sub-layer
  * 
  ************************************************/
typedef enum
{
 CONNECT_REQ,
 CONNECT_ACK,
 RELEASE_REQ,
 DATA_REQ,
 ID_COLLISION
}EMN_LLC_COMMAND;
typedef union
{
  EMN_LLC_COMMAND  type;

}EMN_LLC_PACKET;
// stored tx data, after received ack from peer, then release it 
// at this system, we only will stored PHY_TX_REQ message.
typedef struct{
 UINT_8  which;
 UINT_8  frm_num;
 UINT_8  net_addr;
 UINT_8  subnet_addr;
}MAC_RESEND_BUF;


extern UINT_8  MAC_ComBuf[EMN_PHY_PAYLOAD_LEN];
extern UINT_8  MAC_ComBuf2[EMN_PHY_PAYLOAD_LEN];
extern MAC_RESEND_BUF        MAC_ReSendBuf;


extern UINT_8      ScanNetAddr;
extern UINT_8      ScanSubnetAddr;
extern UINT_8      ScanDevAddr;
extern UINT_8      ScanDevNum;
extern UINT_8      Scanning;
extern UINT_8	   ScanDevUID[4];



extern void EMN_MAC_Task(SYS_MESSAGE *msg);
extern void EMN_MAC_TaskInit(void);

#endif

