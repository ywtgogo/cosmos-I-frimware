/*********************************************************
* 
* 	Copyright (c) 2010 ;
* 	All Rights Reserved Convertegy
* 	FileName: network_frame.c
* 	Version : 
* 	Date    : 
*
* Comments:      
*
***********************************************************/
#include "network_frame.h"
#define  DICOR_STATIC_ROUTER_TAB  1 


// device info 
 EMN_DEV_TYPE	 EMN_DevType;
 DEV_ADDR        EMN_DevAddr;
 NET_ADDR        EMN_SubnetAddr; 
 NET_ADDR        EMN_NetAddr;
 //const	UINT_8	 EMN_DeviceUID[4]=DICOR_UID;
 //const  UINT_8   EMN_ScanAddr[4]={EMN_CONVERTERGY_NO,0xff,0xff,0x00};

 //
 UINT_8          EMN_NetDepth;
 UINT_8          EMN_ChildNum;
 
 //router table
 
 //router table

 
#if DICOR_STATIC_ROUTER_TAB 
#if (DIS_DIAN_NUM!=0)	

//register table
const UINT_8  DiAnAddrTable[]={ 0x00,0x95,
                                0x00,0x96,
                                0x00,0x97 
						   
							 };
#endif
//subnet address,its depth level,the number of its children 
/*const UINT_8 SubnetDepth[]={0x01,0x01,0x01,
                            0x02,0x02,0x00};*/
UINT_8* SubnetDepth;                            
#if 0
const UINT_8  RegTable[]={
						   0xaa,0x20,0x00,0x00,0x12,0x11,0x00,EMN_DEV_NUM,
						   0x00,0x00,0x00,0x02,0x01,0x00,
						   0x40,0x00,0x00,0x02,0x02,0x00
						 };  
#else

UINT_8* RegTable;

DIANDIDITABLE_PTR pDiAnDiDiTable;	//DiAn_DiDi直接的关系映射表
	/*
	const UINT_8 DianDidiTable[]={
	   0x00,0x02,0x01,0xff,0x00,0x00,0x00,0x02,
				 0x02,0xff,0x00,0x06,0x00,0x01,
				 0x01,0x00,0x01,0x01,0x02,0x00,
				 0x02,0x01,0x02,0x02,
	   
	  };*/
	


#endif 
//router table
EMN_ROUTER_TAB_ITEM* EMN_DownTab;
 /*EMN_ROUTER_TAB_ITEM  EMN_DownTab[EMN_DOWN_TAB_SIZE]={
 	{0x00,0xff,0xff},
 	{0x00,0xff,0xff},
 	{0x00,0xff,0xff},
 	{0x00,0xff,0xff}
 };*/
 //UINT_8               EMN_UpTab = 0x00; 
 
#else
// EMN_ROUTER_TAB_ITEM  EMN_DownTab[EMN_DOWN_TAB_SIZE];
// EMN_DIDI_DIAN_TAB EMN_DiDi_DiAnTab[EMN_DEV_NUM];

 UINT_8               EMN_UpTab; 
#endif 


 SYS_MESSAGE 	      EMN_SendMsg;
extern void dicor_dis_timer(void);

void SYS_Error(UINT_8 m)
{
#if 0
     dicor_dis_timer();
     DEBUG_DIS(printf("\n protocol error %d",m));
     while(1){};
     DEBUG_DIS(printf("\n protocol error %d",m));
#else  //
    MCF5225_CCM_STRUCT_PTR  ccr_reg_addr;
	dicor_dis_timer();
    DEBUG_DIS(printf("\n protocol error %d",m));
	_time_delay(500);
    ccr_reg_addr=(MCF5225_CCM_STRUCT_PTR)(&((VMCF5225_STRUCT_PTR)_PSP_GET_IPSBAR())->CCM);
    ccr_reg_addr->RCR |= 0x80;
#endif

}

/* EOF */

