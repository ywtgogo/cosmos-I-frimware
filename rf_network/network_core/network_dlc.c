/**********************************************************************
* 
* 	Copyright (c) 2010 ;
* 	All Rights Reserved by convertergy
* 	FileName: network_dlc.c
* 	Version : 
* 	Date    : 
*
* Comments:
*      1) if device is coordniator,it can generate network beacon.
*      2) use CSMA_CA schem for accessing the shared RF channel.
*      3) handle guarented time slot (GTS)
*      4) creat relaible link between two peer to peer device. 
*      
*     super frame: beacon + contention access period  + contention free access period + unactive period 
*     Evey network has a unique PAN ID(1 byte), every device had a device ID(1 byte).
*     data from RFD to FFD, not realtime data
*     data from FFD to RFD, it is realtime data.
*************************************************************************/
//#include "network_phy.h"


#include "network_dlc.h"
#include "network_nwk.h"
//#include "network_frame.h"
#include "eeprom.h"


#define	MAX_RX_TIME		 (1000)		
#define MAX_TX_TIME      (20)          


UINT_8  MAC_ComBufIndex; 
UINT_8	MAC_ComBuf[EMN_PHY_PAYLOAD_LEN];
UINT_8	MAC_ComBuf2[EMN_PHY_PAYLOAD_LEN];

static EMN_MAC_STATE  MAC_State ;

static SYS_TIMER      EMN_MacTimer; 
static UINT_8         MAC_DisNetTime;  

//scan process
#define MAX_SCAN_TIMES   (200)
static UINT_8         ScanTimeoutNum =0 ;  //times that master ask slave
UINT_8                ScanNetAddr;
UINT_8                ScanSubnetAddr;
UINT_8                ScanDevAddr;
UINT_8                ScanDevNum; 
UINT_8	              ScanDevUID[4];
UINT_8                Scanning;



#define MAC_ScanAgain()  { \
                           EMN_SendMsg.dest_id = EMN_MAC_TASK;\
                           EMN_SendMsg.msg_id = EMN_SCAN_AGAIN;\
                           SYS_SendMsg(&EMN_SendMsg);\
                          }
//resend handle 
#define MAC_RESEND_TIME  (80)  //50
MAC_RESEND_BUF           MAC_ReSendBuf;
static UINT_8            MAC_ResendTimeoutCt;
#define MAC_START_RESEND_TIMER()  {\
                                    EMN_MacTimer.value =MAC_RESEND_TIME;\
                                     EMN_MacTimer.msgID = SYS_TIMEOUT_RESEND;\
                                     SYS_StartTimer(&EMN_MacTimer);\
                                   }
#define MAC_STOP_RESEND_TIMER()   SYS_StopTimer(&EMN_MacTimer)

void MAC_AddRouterItem(UINT_8  subnet);
UINT_8 SYS_GetRandom(void);
void MAC_MacDataReqHal(SYS_MESSAGE *msg);
void MAC_PhyDataInd(SYS_MESSAGE *msg);
void MAC_PhyTxCfm();
void EMN_MAC_DisNetwork(void);

/***********************************************
* 
* Function Name    : 
* Returned Value   : 
* Comments          :
*  
*
**************************************************/
void MAC_AddRouterItem(UINT_8  subnet)
{



}


UINT_8 SYS_GetRandom(void)
{
	
  return 20;	
	
}

/***********************************************
* 
* Function Name    : 
* Returned Value    : 
* Comments          :
*  
*
**************************************************/
static void MAC_MakeMasterScanFrm(UINT_8 com, 
                                               UINT_8 sub_com,UINT_8 *data)
{
  EMN_MAC_FRAME *mac_frm;  

 //Make frame
 mac_frm=(EMN_MAC_FRAME*)(&MAC_ComBuf[0]);
#if 0 
 mac_frm->mac_h.index=0;
 mac_frm->mac_h.last=1;
 mac_frm->mac_h.need_ack=0;
 mac_frm->mac_h.repeat=0;
 mac_frm->mac_h.type=2;
#else
MAC_ComBuf[0]=0x12;

#endif  
 mac_frm->scan_frm.com=com;
 mac_frm->scan_frm.sub_com= sub_com; 
 if(sub_com == CHALLENGE_UID)
 {
   mac_frm->scan_frm.data[0] = ScanNetAddr;
   mac_frm->scan_frm.data[1] = ScanSubnetAddr;
   mac_frm->scan_frm.data[2] = ScanDevAddr;
   MAC_ComBufIndex =6;			 
 }else if(sub_com == NETADDR_ALLOC)
 	{
 	  //UID and network address+subnetwork address +device address
	  mac_frm->scan_frm.data[0] = ScanNetAddr;
	  mac_frm->scan_frm.data[1] = ScanSubnetAddr;
	  mac_frm->scan_frm.data[2] = ScanDevAddr;
	  mac_frm->scan_frm.data[3] = data[0];
	  mac_frm->scan_frm.data[4] = data[1];
	  mac_frm->scan_frm.data[5] = data[2]; 	  
	  mac_frm->scan_frm.data[6] = data[3];
      MAC_ComBufIndex =10;			 
 }
 //send this frame
 EMN_SendMsg.dest_id = EMN_PHY_TASK;
 EMN_SendMsg.msg_id = EMN_PHY_TX_REQ;
 EMN_SendMsg.data[0] = EMN_TXBUF_MACCOM; 
 EMN_SendMsg.data[1] = 0x01;
 EMN_SendMsg.data[2] = 0xff;
 EMN_SendMsg.data[3] = 0xff;
 SYS_SendMsg((SYS_MESSAGE *)&EMN_SendMsg);
}


static void MasterScanProcess(SYS_MESSAGE *msg)
{

 EMN_MAC_FRAME *scan_rx_data;
 switch(MAC_State)
 {
   case   EMN_MAC_SCAN_MST0:
   	         SYS_StopTimer(&EMN_MacTimer);
		   	 if((msg->msg_id == EMN_MAC_SCAN_REQ)||(msg->msg_id == EMN_SCAN_AGAIN))
		   	 {
		   	   //send first requst
               DEBUG_DIS(printf("\nSCAN:1st req"));
               MAC_MakeMasterScanFrm(NETWORK_MASTER_SCAN, CHALLENGE_UID,0);
			   // start TX timer
			   EMN_MacTimer.value = MAX_TX_TIME;
			   EMN_MacTimer.msgID = SYS_TIMEOUT_MAC_TX;
			   SYS_StartTimer(&EMN_MacTimer);
		       MAC_State = EMN_MAC_SCAN_MST1; 	
			   ScanDevUID[0]=0x00;
   			   ScanDevUID[1]=0x00;
  			   ScanDevUID[2]=0x00;
			   ScanDevUID[3]=0x00;			   
		     }
		     break; 			
     case   EMN_MAC_SCAN_MST1:
		    switch(msg->msg_id)
		    {
			 case EMN_PHY_TX_CFM:	
			 	   SYS_StopTimer(&EMN_MacTimer); // stop TX timer					  					 
			 	   if(msg->data[0]==EMN_TXBUF_MACCOM)
	 			   {  
                     MAC_ComBufIndex =0;
					 if(msg->data[2]== EMN_OK)
					 {
					  // start RX timer
						EMN_MacTimer.value = MAX_RX_TIME;
						EMN_MacTimer.msgID = SYS_TIMEOUT_MAC_RX;
						SYS_StartTimer(&EMN_MacTimer); 
					 }else
						{
                           MAC_State = EMN_MAC_SCAN_MST0;
						   MAC_ScanAgain();
						}
			 	   	}
				   break;

			  case EMN_PHY_RX_IND:  
					 SYS_StopTimer(&EMN_MacTimer);                     
                     //release buffer
					 EMN_SendMsg.dest_id = EMN_PHY_TASK;
					 EMN_SendMsg.msg_id = EMN_PHY_RX_RES;
					 EMN_SendMsg.data[0]= msg->data[0];  
					 EMN_SendMsg.data[1]= msg->data[1];  
					 SYS_SendMsg(&EMN_SendMsg);
					 
                     scan_rx_data=(EMN_MAC_FRAME*)&(EMN_PHY_RxBuf.data[msg->data[0]][0]);
					 if((scan_rx_data->scan_frm.com== NETWORK_SLAVE_SCAN) &&
						 (scan_rx_data->scan_frm.sub_com== CHALLENGE_UID_ACK))	
						{	
						     
							// send 2rd request with received UID;and alllocated TID							
							ScanDevUID[0]=scan_rx_data->scan_frm.data[3];
							ScanDevUID[1]=scan_rx_data->scan_frm.data[4];
							ScanDevUID[2]=scan_rx_data->scan_frm.data[5];
							ScanDevUID[3]=scan_rx_data->scan_frm.data[6];
							DEBUG_DIS(printf("\nrev UID:%d-%d-%d-%d/n",ScanDevUID[0],ScanDevUID[1],ScanDevUID[2],ScanDevUID[3]));
							MAC_MakeMasterScanFrm(NETWORK_MASTER_SCAN, NETADDR_ALLOC,
							               &(ScanDevUID[0]));
							DEBUG_DIS(printf("\nSCAN:2nd req"));
							// start TX timer
							EMN_MacTimer.value = MAX_TX_TIME;
							EMN_MacTimer.msgID = SYS_TIMEOUT_MAC_TX;
							SYS_StartTimer(&EMN_MacTimer);
							//change state
							MAC_State = EMN_MAC_SCAN_MST3; 
							ScanTimeoutNum=0;
						}else{
								 MAC_State = EMN_MAC_SCAN_MST0;								 
								 MAC_ScanAgain();
							 }
                       
			         break;			   
			   case EMN_TIMEOUT :
			   	     SYS_StopTimer(&EMN_MacTimer);
					 if(msg->data[0] == SYS_TIMEOUT_MAC_RX)
					 {
						 MAC_State = EMN_MAC_SCAN_MST0;	
					     MAC_ScanAgain();

						 ScanTimeoutNum++;
						  // no device within the rang of this device's RF		
						 if(ScanTimeoutNum > MAX_SCAN_TIMES) 
						 {	
						 #if 0
                             //send scan finish message to NWK layer.                             
							 EMN_SendMsg.dest_id = EMN_NWK_TASK;
							 EMN_SendMsg.msg_id = EMN_MAC_SCAN_CFM;
							 EMN_SendMsg.data[0] = EMN_MAC_SCAN_FINISH; 
							 EMN_SendMsg.data[1] = ScanDevNum;
							 SYS_SendMsg(&EMN_SendMsg);
							 DEBUG_DIS(printf("\nSCAN:end"));
							 ScanTimeoutNum=0;
							 //change state
							 MAC_State = EMN_MAC_SCAN_M_END;
					     #else
						 ScanTimeoutNum=0;
						 MAC_State = EMN_MAC_SCAN_MST0;	
					     MAC_ScanAgain();

						 #endif 
						 }						 

					 }else if(msg->data[0] == SYS_TIMEOUT_MAC_TX)
					 	{// there is something wrong with RF module.
                           MAC_State = EMN_MAC_SCAN_MST0;  						   
						   MAC_ScanAgain();
					 	}
					break;
			default : break; 
		  }		
				  break;	
	case  EMN_MAC_SCAN_MST3:
		switch(msg->msg_id)
		{
			case EMN_PHY_RX_IND:
					SYS_StopTimer(&EMN_MacTimer); 
					EMN_SendMsg.dest_id = EMN_PHY_TASK;
					EMN_SendMsg.msg_id = EMN_PHY_RX_RES;
					EMN_SendMsg.data[0]= msg->data[0];	
					EMN_SendMsg.data[1]= msg->data[1];	
					SYS_SendMsg(&EMN_SendMsg);
					
                    scan_rx_data=(EMN_MAC_FRAME*)&(EMN_PHY_RxBuf.data[msg->data[0]][0]);
					if((scan_rx_data->scan_frm.com== NETWORK_SLAVE_SCAN) &&
					   (scan_rx_data->scan_frm.sub_com== NETADDR_ALLOC_ACK)&&
					    (ScanDevUID[0] == scan_rx_data->scan_frm.data[3])&&
					    (ScanDevUID[1] == scan_rx_data->scan_frm.data[4])&&
					    (ScanDevUID[2] == scan_rx_data->scan_frm.data[5])&&
					    (ScanDevUID[3] == scan_rx_data->scan_frm.data[6]))    //only with UID
					   {						   
						   // one device message  to NWK layer.
						   EMN_SendMsg.dest_id = EMN_NWK_TASK;
						   EMN_SendMsg.msg_id = EMN_MAC_SCAN_CFM;
						   EMN_SendMsg.data[0] = EMN_MAC_SCAN_ONE_DEV; 
						   SYS_SendMsg(&EMN_SendMsg);
						   DEBUG_DIS(printf("\nSCAN:one device"));
                           //change state						   
						   ScanTimeoutNum = 0;						   
						   ScanDevNum++;
						   ScanDevAddr++;
						   MAC_State = EMN_MAC_SCAN_MST0; 						   
						   MAC_ScanAgain();
					   }else{
								MAC_State = EMN_MAC_SCAN_MST0; 	
								DEBUG_DIS(printf("\nSCAN:UID error"));							
								MAC_ScanAgain();
						   }
					   
				 break;
			case EMN_PHY_TX_CFM:
				SYS_StopTimer(&EMN_MacTimer); // stop TX timer									  
				if(msg->data[2]== EMN_OK)
				{
				  // start RX timer,because the PHY change to receive mode automatically
				  EMN_MacTimer.value = MAX_RX_TIME;
				  EMN_MacTimer.msgID = SYS_TIMEOUT_MAC_RX;
				  SYS_StartTimer(&EMN_MacTimer); 
				}else
				  {
					MAC_State = EMN_MAC_SCAN_MST0;					
					MAC_ScanAgain();
				  }
				 break;
			case EMN_TIMEOUT :
				 SYS_StopTimer(&EMN_MacTimer);
				  if(msg->data[0] == SYS_TIMEOUT_MAC_TX||
				  	 msg->data[0] == SYS_TIMEOUT_MAC_RX)
				  {
					  MAC_State = EMN_MAC_SCAN_MST0;					  
					  MAC_ScanAgain();
				  }
				 break;
			default : break;				 
				 
		}

				  break;
			default: break;
	}

}

//this function only for EMT
static void MasterScanEndProcess(SYS_MESSAGE * msg)
{
  EMN_MAC_FRAME *mac_frm;  
  
  if(msg->msg_id ==EMN_PHY_RX_IND)//must this mssage must send response 
  {
	   //discard this frame
	  EMN_SendMsg.dest_id = EMN_PHY_TASK;
	  EMN_SendMsg.msg_id = EMN_PHY_RX_RES;
	  EMN_SendMsg.data[0]= msg->data[0];  
	  EMN_SendMsg.data[1]= msg->data[1];  
	  SYS_SendMsg(&EMN_SendMsg);
  }
  else if(msg->msg_id ==EMN_MAC_END_SCAN_REQ)
  {
    //Make frame
    mac_frm=(EMN_MAC_FRAME*)(&MAC_ComBuf[0]);
 //   MAC_ComBuf[0]=0x15;  //repeat page
      MAC_ComBuf[0]=0x11;  //no repeat page
    mac_frm->scan_frm.com=NETWORK_MASTER_SCAN;
    mac_frm->scan_frm.sub_com= NETWORK_JOIN; 
    //send this frame
    EMN_SendMsg.dest_id = EMN_PHY_TASK;
    EMN_SendMsg.msg_id = EMN_PHY_TX_REQ;
    EMN_SendMsg.data[0] = EMN_TXBUF_MACCOM; 
    EMN_SendMsg.data[1] = 0x01;
    EMN_SendMsg.data[2] = 0xff;
    EMN_SendMsg.data[3] = 0xff;
    SYS_SendMsg((SYS_MESSAGE *)&EMN_SendMsg);

	    //send this frame
    EMN_SendMsg.dest_id = EMN_PHY_TASK;
    EMN_SendMsg.msg_id = EMN_PHY_TX_REQ;
    EMN_SendMsg.data[0] = EMN_TXBUF_MACCOM; 
    EMN_SendMsg.data[1] = 0x01;
    EMN_SendMsg.data[2] = 0xff;
    EMN_SendMsg.data[3] = 0xff;
    SYS_SendMsg((SYS_MESSAGE *)&EMN_SendMsg);
	    //send this frame
    EMN_SendMsg.dest_id = EMN_PHY_TASK;
    EMN_SendMsg.msg_id = EMN_PHY_TX_REQ;
    EMN_SendMsg.data[0] = EMN_TXBUF_MACCOM; 
    EMN_SendMsg.data[1] = 0x01;
    EMN_SendMsg.data[2] = 0xff;
    EMN_SendMsg.data[3] = 0xff;
    SYS_SendMsg((SYS_MESSAGE *)&EMN_SendMsg);
  	
  }
  else if(msg->msg_id ==EMN_PHY_TX_CFM)
  {
    if(msg->data[2]== EMN_OK)
  	{
      //send this frame
      EMN_SendMsg.dest_id = EMN_NWK_TASK;
      EMN_SendMsg.msg_id = EMN_MAC_END_SCAN_CFM;
      SYS_SendMsg((SYS_MESSAGE *)&EMN_SendMsg);
      MAC_State = EMN_MAC_CON;
  		
  	}
  }
}

/************************************************************
* 
* Function Name    : 
* Returned Value    : 
* Comments          :
*  
*
**************************************************************/
static void MAC_MakeSlaveScanFrm(UINT_8 com, UINT_8 sub_com)
{
  EMN_MAC_FRAME *mac_frm;  
	
	//Make frame
mac_frm=(EMN_MAC_FRAME*)&MAC_ComBuf[0];

#if 0 
 mac_frm->mac_h.index=0;
 mac_frm->mac_h.last=1;
 mac_frm->mac_h.need_ack=0;
 mac_frm->mac_h.repeat=0;
 mac_frm->mac_h.type=2;
#else
 MAC_ComBuf[0]=0x12;
#endif

  mac_frm->scan_frm.com=com;
  mac_frm->scan_frm.sub_com= sub_com; 

 if(sub_com == CHALLENGE_UID_ACK||sub_com ==NETADDR_ALLOC_ACK)
   {
	  mac_frm->scan_frm.data[0]= ScanNetAddr;
 	  mac_frm->scan_frm.data[1]= ScanSubnetAddr;
	  mac_frm->scan_frm.data[2]= ScanDevAddr;
	  mac_frm->scan_frm.data[3]= pBaseConfig->uid[0];
	  mac_frm->scan_frm.data[4]= pBaseConfig->uid[1];
	  mac_frm->scan_frm.data[5]= pBaseConfig->uid[2];
	  mac_frm->scan_frm.data[6]= pBaseConfig->uid[3];
	  MAC_ComBufIndex =7;		
	  
	  EMN_SendMsg.dest_id = EMN_PHY_TASK;
	  EMN_SendMsg.msg_id = EMN_PHY_TX_REQ;
	  EMN_SendMsg.data[0] = EMN_TXBUF_MACCOM; 
	  EMN_SendMsg.data[1] = 0x01;
	  EMN_SendMsg.data[2] = 0Xff;
	  EMN_SendMsg.data[3] = 0Xff;
	  SYS_SendMsg(&EMN_SendMsg);
   } //send this frame
}	

static void SlaveScanProcess(SYS_MESSAGE *msg)
{
	EMN_MAC_FRAME *scan_rx_data;
	static UINT_8  start_cd=0;

 switch(MAC_State)
 {
   case  EMN_MAC_SCAN_SST0:
   	      SYS_StopTimer(&EMN_MacTimer);
   	      start_cd=0;
		  if( msg->msg_id==EMN_PHY_RX_IND)
		  {		scan_rx_data=(EMN_MAC_FRAME*)&(EMN_PHY_RxBuf.data[msg->data[0]][0]);
			    if((scan_rx_data->scan_frm.com == NETWORK_MASTER_SCAN) &&
			  	   (scan_rx_data->scan_frm.sub_com == CHALLENGE_UID))
			    {
				  	   if((scan_rx_data->scan_frm.data[2]> EMN_DevAddr) &&
				  	       (EMN_DevAddr!=0xff))  //OK
				  	   {
					      MAC_State = EMN_MAC_SCAN_S_END;
						  
				  	    }else
				  	      {
                              ScanNetAddr = scan_rx_data->scan_frm.data[0];
                              ScanSubnetAddr = scan_rx_data->scan_frm.data[1];
                              ScanDevAddr = scan_rx_data->scan_frm.data[2];							  
							  MAC_State = EMN_MAC_SCAN_SST1;
							  // start backoff timer
							  EMN_MacTimer.value =SYS_GetRandom();							  
							  EMN_MacTimer.msgID = SYS_TIMEOUT_MAC_TX_BACKOFF;
							  SYS_StartTimer(&EMN_MacTimer);
				  	      }
				}
				//discard this frame
				EMN_SendMsg.dest_id = EMN_PHY_TASK;
				EMN_SendMsg.msg_id = EMN_PHY_RX_RES;
				EMN_SendMsg.data[0]= msg->data[0];	
				EMN_SendMsg.data[1]= msg->data[1];	
				SYS_SendMsg(&EMN_SendMsg);
				
			}
			break; 
	case   EMN_MAC_SCAN_SST1:
		   	 switch(msg->msg_id)
		   	 {
                case EMN_PHY_RX_IND:
					  SYS_StopTimer(&EMN_MacTimer);	
		  		      scan_rx_data=(EMN_MAC_FRAME*)&(EMN_PHY_RxBuf.data[msg->data[0]][0]);
			          if(((scan_rx_data->scan_frm.com== NETWORK_MASTER_SCAN)|| 
			  	       (scan_rx_data->scan_frm.com== NETWORK_SLAVE_SCAN))&&(start_cd))  //from master
			          {       		           
						start_cd =0;
			            MAC_State = EMN_MAC_SCAN_SST0;			             

			          }	
					  //discard this frame
					  EMN_SendMsg.dest_id = EMN_PHY_TASK;
					  EMN_SendMsg.msg_id = EMN_PHY_RX_RES;
					  EMN_SendMsg.data[0]= msg->data[0];  
					  EMN_SendMsg.data[1]= msg->data[1];  
					  SYS_SendMsg(&EMN_SendMsg);
					  break;
				case EMN_PHY_TX_CFM:					
                     // stop TX timer
				     SYS_StopTimer(&EMN_MacTimer);					  	
                    if(msg->data[2]== EMN_OK) 
					 {
    					  // Switch state
    					MAC_State = EMN_MAC_SCAN_SST3;
                        start_cd =0;						  
    					  // start RX timer
    					EMN_MacTimer.value = MAX_RX_TIME;
    					EMN_MacTimer.msgID = SYS_TIMEOUT_MAC_RX;
    					SYS_StartTimer(&EMN_MacTimer); 
					  }
					  break;
				 case EMN_TIMEOUT :
					    if(msg->data[0]== SYS_TIMEOUT_MAC_TX_BACKOFF)
						{
                          // send the first reponse message
                          MAC_MakeSlaveScanFrm(NETWORK_SLAVE_SCAN, CHALLENGE_UID_ACK);

						  // start TX timer
						  EMN_MacTimer.value = MAX_TX_TIME;
						  EMN_MacTimer.msgID = SYS_TIMEOUT_MAC_TX;
						  SYS_StartTimer(&EMN_MacTimer);
						  start_cd =1;
					  	}else if(msg->data[0] == SYS_TIMEOUT_MAC_TX)
					  	  {// there is somthing wrong with RF module
                            MAC_State = EMN_MAC_SCAN_SST0;
							start_cd =0;
					  	  }
					   	break;
						
				 default: break;	   
						   
		   	 }
			 break;	
	case  EMN_MAC_SCAN_SST3:	
			switch(msg->msg_id)
			{
			  case EMN_PHY_RX_IND:   // receive 3rd from Master
			         // stop RX timer
					 SYS_StopTimer(&EMN_MacTimer); 					
					 scan_rx_data=(EMN_MAC_FRAME*)&(EMN_PHY_RxBuf.data[msg->data[0]][0]);
					 if((scan_rx_data->scan_frm.com== NETWORK_MASTER_SCAN) &&
					   (scan_rx_data->scan_frm.sub_com== NETADDR_ALLOC))  
					 {
						if((scan_rx_data->scan_frm.data[0]==ScanNetAddr)&&
						   (scan_rx_data->scan_frm.data[1] ==ScanSubnetAddr)&&
						   (scan_rx_data->scan_frm.data[2] ==ScanDevAddr)&&
						   (scan_rx_data->scan_frm.data[3]== pBaseConfig->uid[0])&&
						   (scan_rx_data->scan_frm.data[4] == pBaseConfig->uid[1])&&
						   (scan_rx_data->scan_frm.data[5] == pBaseConfig->uid[2])&&
						   (scan_rx_data->scan_frm.data[6] == pBaseConfig->uid[3]))
						{

						  //send the 3rd ack to master
                          MAC_MakeSlaveScanFrm(NETWORK_SLAVE_SCAN, NETADDR_ALLOC_ACK);
						  //start TX timer
						  EMN_MacTimer.value =MAX_TX_TIME;
						  EMN_MacTimer.msgID = SYS_TIMEOUT_MAC_TX;
						  SYS_StartTimer(&EMN_MacTimer);
						 }
						 else
						 	{
                              MAC_State = EMN_MAC_SCAN_SST0;	
						 	}
					 }else
					 	{
                            MAC_State = EMN_MAC_SCAN_SST0;	
					 	}
					 
					 EMN_SendMsg.dest_id = EMN_PHY_TASK;
					 EMN_SendMsg.msg_id = EMN_PHY_RX_RES;
					 EMN_SendMsg.data[0]= msg->data[0];  
					 EMN_SendMsg.data[1]= msg->data[1];  
					 SYS_SendMsg(&EMN_SendMsg);
						 break;
				    case EMN_PHY_TX_CFM:					
						    // stop TX timer
						   SYS_StopTimer(&EMN_MacTimer);	
						  if(msg->data[2]== EMN_OK)
						   {
							  // Switch state
							 MAC_State = EMN_MAC_SCAN_S_END;	
							}

						  break;						 
					case EMN_TIMEOUT :
						 if(msg->data[0]== SYS_TIMEOUT_MAC_RX||// no receive 3nd response from mast end
						 	msg->data[0]== SYS_TIMEOUT_MAC_TX) // wrong with PHY 
						  {
							 MAC_State = EMN_MAC_SCAN_SST0;
						  }
						    
						 break;
					default: break; 	 
						 
				}

				  break;
	default: return;
  }  
}

static void SlaveScanEndProcess(SYS_MESSAGE * msg)
{
  EMN_MAC_FRAME *scan_rx_data;
  
if(msg->msg_id ==EMN_PHY_RX_IND)//must this mssage must send response 
{
	 scan_rx_data=(EMN_MAC_FRAME*)&(EMN_PHY_RxBuf.data[msg->data[0]][0]);
	 if((scan_rx_data->scan_frm.com== NETWORK_MASTER_SCAN) &&
	   (scan_rx_data->scan_frm.sub_com== NETWORK_JOIN))  

	 {
	  EMN_DevType=EMN_DEVICE;
	  EMN_NetAddr=ScanNetAddr;
	  EMN_SubnetAddr=ScanSubnetAddr;
	  EMN_DevAddr=ScanDevAddr;
	  MAC_State = EMN_MAC_CON;
	  PHY_WriteRxAddr(EMN_NetAddr,EMN_SubnetAddr);

	  
	  //send NWK layer message	
	  EMN_SendMsg.dest_id = EMN_NWK_TASK;
	  EMN_SendMsg.msg_id = EMN_MAC_CON_NET_IND;
	  SYS_SendMsg(&EMN_SendMsg);
	 }
	   //discard this frame
	  EMN_SendMsg.dest_id = EMN_PHY_TASK;
	  EMN_SendMsg.msg_id = EMN_PHY_RX_RES;
	  EMN_SendMsg.data[0]= msg->data[0];  
	  EMN_SendMsg.data[1]= msg->data[1];  
	  SYS_SendMsg(&EMN_SendMsg);
}
  
}

extern _mem_size base_addr;
//from NWK layer,only with the number of phy frame
static void MAC_MacDataReqHal(SYS_MESSAGE *msg)
{
  UINT_8 i,j;
  NET_ADDR	next;
  EMN_MAC_FRAME  *mac_frame;
	EEPROMDATA_PTR p;
	p = (EEPROMDATA_PTR) base_addr;
	
  //modify the MAC header
  for(i=0;i<msg->data[1]; i++)
  {
    if(msg->data[0]==EMN_TXBUF_NWKTX)
    {
        mac_frame =(EMN_MAC_FRAME* )&NWK_TxBuf[i][0];
		mac_frame->mac_h.index = 0;
		mac_frame->mac_h.last = 0;
		mac_frame->mac_h.need_ack = 0;
		mac_frame->mac_h.repeat = 0;
		mac_frame->mac_h.type = 0;
    }else if(msg->data[0]==EMN_TXBUF_NWKCOM)
      {
        if(i>=1) SYS_Error(0x0b); 
		mac_frame =(EMN_MAC_FRAME* )&NWK_ComBuf[0]; 
			mac_frame->mac_h.index = 0;
			mac_frame->mac_h.last = 0;
			mac_frame->mac_h.need_ack = 0;
			mac_frame->mac_h.repeat = 0;
			mac_frame->mac_h.type = 0;
      }else
      	{
             SYS_Error(1);
      	}
    if(mac_frame->nwk_h.data[0] == EMN_NWK_ALLOC_ADDR_REQ)
    {
     	mac_frame->mac_h.type =0x03;//every router must upate itself router table 
    }else
      {
	    mac_frame->mac_h.type =0x00;
      }	
	mac_frame->mac_h.index =i;
	mac_frame->mac_h.repeat = 0;

	//insert router information 
	if(i==0)
	{
	  mac_frame->nwk_h.last = EMN_SubnetAddr;
     if(mac_frame->nwk_h.dest.subnet == EMN_SubnetAddr)
     {
         mac_frame->nwk_h.next = EMN_SubnetAddr;
         next = mac_frame->nwk_h.next;
     }else
        {
          //insert  the next subnet for this frame transmitter    
	      mac_frame->nwk_h.next = EMN_SubnetAddr;
		 //for(i=0;i<EMN_DOWN_TAB_SIZE;i++)
		 for(j=0;j<p->RouterTable[2];j++)
		 {
		   if((mac_frame->nwk_h.dest.subnet <= EMN_DownTab[j].max&&
			  mac_frame->nwk_h.dest.subnet >= EMN_DownTab[j].min)||
			  (mac_frame->nwk_h.dest.subnet == EMN_DownTab[j].addr))
		   {
			  mac_frame->nwk_h.next = EMN_DownTab[j].addr;	
			  next = mac_frame->nwk_h.next;
			  break;
		   }
		  }	
          if( mac_frame->nwk_h.next == EMN_SubnetAddr)  
		  	//{SYS_Error(0x0c);}//router table error   
		  	while (1)
		  	{
		  //		printf("router table error \n");   by younger 2013.7.1
				_time_delay(40);
			}
         }
	}	
	if(i ==(msg->data[1]-1))
	{
	  mac_frame->mac_h.last =1;
#if(DICOR_ACK_RESEND)	
      mac_frame->mac_h.need_ack =1;    
#else
      mac_frame->mac_h.need_ack =0;   //no ack, at now, we use no ack 
#endif
	  break;	  
	}
  }

//send mssage request to PHY layer.  
  EMN_SendMsg.dest_id = EMN_PHY_TASK;
  EMN_SendMsg.msg_id = EMN_PHY_TX_REQ;
  EMN_SendMsg.data[0]= msg->data[0]; 
  EMN_SendMsg.data[1]= msg->data[1];
  EMN_SendMsg.data[2] = EMN_NetAddr;
  EMN_SendMsg.data[3] = next;  //Tx address  
  SYS_SendMsg(&EMN_SendMsg);
  

}

//this function is called after setup network
//from PHY layer
void MAC_PhyTxCfm(void)
{
	// start TX timer, waite for ack from receiver
	EMN_MacTimer.value = MAC_RESEND_TIME;
	EMN_MacTimer.msgID = SYS_TIMEOUT_RESEND;
	SYS_StartTimer(&EMN_MacTimer); 

}
/***********************************************
* 
* Returned Value : 
* Comments        :
*
*************************************************/

static MAC_AckFrameHal(EMN_MAC_FRAME *mac_frm)
{
UINT_8* data;
data =(UINT_8 *)mac_frm;

if((data[1]==EMN_NetAddr)&&(data[2]==EMN_SubnetAddr)&&
	  (data[3]==EMN_DevAddr))	  
{
   if(data[4]==EMN_OK)
  {
       // stop resend timer
       MAC_STOP_RESEND_TIMER();
	   DEBUG_DIS(printf("\nrev ack"));	 //send cfm to higher layer
		EMN_SendMsg.dest_id = EMN_NWK_TASK;
		EMN_SendMsg.msg_id = EMN_MAC_DATA_CFM;
		EMN_SendMsg.data[0]= MAC_ReSendBuf.which;  
		EMN_SendMsg.data[1]= MAC_ReSendBuf.frm_num;					  
		EMN_SendMsg.data[2]= data[4];
	
	   
#if 0	   
 	   //let nwk layer to release buffer
	  if(MAC_ReSendBuf.which== EMN_TXBUF_PHYRX)
	  {
		EMN_SendMsg.dest_id = EMN_NWK_TASK;
		EMN_SendMsg.msg_id = EMN_MAC_DATA_CFM;
		EMN_SendMsg.data[0]= MAC_ReSendBuf.frm_num;  // OK
		EMN_SendMsg.data[1]= EMN_OK;
		SYS_SendMsg(&EMN_SendMsg);
	  }
#endif 
	  //release resend buffer
	  MAC_ReSendBuf.frm_num =0;
	  MAC_ReSendBuf.subnet_addr =0;
  	  MAC_ReSendBuf.net_addr =0;
	  MAC_ReSendBuf.which =0;
	  MAC_ResendTimeoutCt=0;
  	}else
  	  {
        //resend the TX data
   	    DEBUG_DIS(printf("\nrev error ack"));
        MAC_ResendTimeoutCt++;
		if(MAC_ResendTimeoutCt<2)
		{
		  EMN_SendMsg.dest_id = EMN_PHY_TASK;
		  EMN_SendMsg.msg_id = EMN_PHY_TX_REQ;
		  EMN_SendMsg.data[0]= MAC_ReSendBuf.which;  
		  EMN_SendMsg.data[1]= MAC_ReSendBuf.frm_num;		
		  EMN_SendMsg.data[2] = MAC_ReSendBuf.net_addr;
		  EMN_SendMsg.data[3]= MAC_ReSendBuf.subnet_addr;
		  DEBUG_DIS(printf("\nresend"));
		  SYS_SendMsg(&EMN_SendMsg);      		  
		}else
			{
               MAC_ReSendBuf.frm_num =0;
	           MAC_ReSendBuf.subnet_addr =0;
  	           MAC_ReSendBuf.net_addr =0;
	           MAC_ReSendBuf.which =0;
	           MAC_ResendTimeoutCt=0;
			}
     }
		}
}
/***********************************************
* 
* Returned Value : 
* Comments        :
*           Receive data from the other subnet,and copy data to TXbuffer.
*************************************************/
static void MAC_RevOtherSubnetData(SYS_MESSAGE *msg)
{
	UINT_8 *from,*to;
	UINT_8 i,num;
   	
	//copy data to tx buffer
	to = &NWK_TxBuf[0][0];
	from =&EMN_PHY_RxBuf.data[msg->data[0]][0];  
	num= msg->data[1];
	while(num>0)
	{
	   for(i=0;i<EMN_PHY_PAYLOAD_LEN;i++)
	   {
	     *to=*from;
	     to++;
	     from++;
	   }
	   if(from> (&(EMN_PHY_RxBuf.data[EMN_PHY_RXBUF_COUNT-1][EMN_PHY_PAYLOAD_LEN-1])))
	   {
	      from =&(EMN_PHY_RxBuf.data[0][0]);
       }	
	   num--;
	}
	EMN_SendMsg.dest_id = EMN_NWK_TASK;
	EMN_SendMsg.msg_id = EMN_MAC_DATA_IND;
	EMN_SendMsg.data[0]= EMN_TXBUF_NWKTX;	
	EMN_SendMsg.data[1]= msg->data[1];  
	SYS_SendMsg(&EMN_SendMsg);
     
  
}


void MAC_PhyDataInd(SYS_MESSAGE *msg)
{
  EMN_MAC_FRAME *mac_frm;
  UINT_8        header,i,need_ack;
  UINT_8        *data;
  EEPROMDATA_PTR p;
  
  p = (EEPROMDATA_PTR) base_addr;
 // UINT_8        tmp;
  need_ack=0;
  mac_frm =(EMN_MAC_FRAME*)(&EMN_PHY_RxBuf.data[msg->data[0]][0]); 
  data= (&EMN_PHY_RxBuf.data[msg->data[0]][0]);
  header=EMN_PHY_RxBuf.data[msg->data[0]][0];
  //for(i=0;i<EMN_DOWN_TAB_SIZE;i++)
  for(i=0;i<p->RouterTable[2];i++)
  {
      if(EMN_DownTab[i].addr==mac_frm->nwk_h.last)
	  	break;
  }
  if((header==0x11)||(mac_frm->nwk_h.dest.subnet == EMN_SubnetAddr)&&(mac_frm->nwk_h.dest.dev == EMN_DevAddr))
  {
    i=0;	
  }
  //if((mac_frm->nwk_h.next == EMN_SubnetAddr)&&(i!=EMN_DOWN_TAB_SIZE))
  if((mac_frm->nwk_h.next == EMN_SubnetAddr)&&(i!=p->RouterTable[2]))
  {
  if(header&0x08)  need_ack=1;
  	
  header &=0x03;//type
  if(header==0x00)
  {
       //DEBUG_DIS(printf("\nrev data"));
	   if((mac_frm->nwk_h.dest.subnet == EMN_SubnetAddr)&&
	   	   (mac_frm->nwk_h.dest.dev == EMN_DevAddr)) 
	   {  
		 //  if(need_ack==1)
		   {
				 //send ack mac frame to the peer
				// DEBUG_DIS(printf("\ntx ack"));
				 MAC_ComBuf2[0] =  0x11; 
				 MAC_ComBuf2[1]=EMN_NetAddr;	 
				 MAC_ComBuf2[2]=mac_frm->nwk_h.last;
				 if(mac_frm->nwk_h.last== EMN_SubnetAddr)
				 {
					MAC_ComBuf2[3]=mac_frm->nwk_h.sour.dev;
				 }
				 else
				   {
					 MAC_ComBuf2[3]=0x00;  //send to cluster head
				   }	 
				 MAC_ComBuf2[4] =EMN_OK;	 
				 EMN_SendMsg.dest_id = EMN_PHY_TASK;
				 EMN_SendMsg.msg_id = EMN_PHY_TX_REQ;
				 EMN_SendMsg.data[0]= EMN_TXBUF_MACCOM2;  // from mac
				 EMN_SendMsg.data[1]= 1;	 
				 EMN_SendMsg.data[2] = EMN_NetAddr;
				 EMN_SendMsg.data[3]= mac_frm->nwk_h.last;
				 SYS_SendMsg(&EMN_SendMsg);
				 SYS_SendMsg(&EMN_SendMsg);

	     if(mac_frm->nwk_h.sour.subnet == EMN_SubnetAddr) // from itself subnet
	      {
	        //DEBUG_DIS(printf("\nValid child"));
			for(i=0;i<EMN_PHY_PAYLOAD_LEN;i++)
			{
			  MAC_ComBuf[i]=*data++;
			}
            EMN_SendMsg.dest_id = EMN_NWK_TASK;
            EMN_SendMsg.msg_id = EMN_MAC_DATA_IND;
            EMN_SendMsg.data[0]= EMN_TXBUF_MACCOM;  
            EMN_SendMsg.data[1]= 1;  
            SYS_SendMsg(&EMN_SendMsg);
	      }else //from the other subnet 
	      	{
	      	//   DEBUG_DIS(printf("\nValid the other"));
			 //  need_ack=1;
               MAC_RevOtherSubnetData(msg);
	      	}		  

			}
       }
  }else if(header==0x01)
  	{
	   MAC_AckFrameHal(mac_frm);
  	}
  	}
}
/*--------------------------------------*
 * 
 * Function Name    : 
 * Returned Value    : 
 * Comments          :
 *  
 *---------------------------------------*/
static void MAC_DisNetwork(void)
{
   //Make frame
   MAC_ComBuf[0]=0x12;
   MAC_ComBuf[1]=NETWORK_MASTER_NET_MAN;
   MAC_ComBuf[2]=NETWORK_DIS;

   //send this frame
   EMN_SendMsg.dest_id = EMN_PHY_TASK;
   EMN_SendMsg.msg_id = EMN_PHY_TX_REQ;
   EMN_SendMsg.data[0] = EMN_TXBUF_MACCOM; 
   EMN_SendMsg.data[1] = 0x01;
   EMN_SendMsg.data[2] = EMN_NetAddr;
   EMN_SendMsg.data[3] = EMN_SubnetAddr;
   SYS_SendMsg((SYS_MESSAGE *)&EMN_SendMsg);
   
   //start TX timer
   EMN_MacTimer.value =300;
   EMN_MacTimer.msgID = SYS_TIMEOUT_DIS_NET;
   SYS_StartTimer(&EMN_MacTimer);
}

/**************************************************
* 
* Function Name    : 
* Returned Value    : 
* Comments          :
*  
*
***************************************************/
void EMN_MAC_TaskInit(void)
{
    //init timer for this task using
  EMN_MacTimer.id = SYS_TIMER0;
  EMN_MacTimer.fastfun = NULL;
  EMN_MacTimer.taskID = EMN_MAC_TASK;
  EMN_MacTimer.value =0;
  EMN_MacTimer.msgID = 0;

   MAC_ResendTimeoutCt=0;

 //init state
 if(EMN_SubnetAddr == 0xFF || EMN_DevAddr == 0xFF ) 	
 {
   MAC_State = EMN_MAC_SCAN_SST0; 
   EMN_SubnetAddr = 0xff;
   EMN_DevAddr = 0xff;
   EMN_NetAddr=0xff;
 //  PHY_WriteRxAddr(0xff,0xff);// write scan network address
 }else
   {
     MAC_State = EMN_MAC_CON;
   }
 //init router table
 //EMN_UpTab=0x00;
 #if 0
 for(i=0;i<EMN_DOWN_TAB_SIZE;i++)
 {
    EMN_DownTab.addr=0x00;
	EMN_DownTab.max=0x00;
	EMN_DownTab.min=0x00;
 }
 #endif
 #if DICOR_STATIC_REG_TAB
 MAC_State=EMN_MAC_CON;
 #else
   //handle dis_network  
    MAC_DisNetTime=0;
    EMN_SendMsg.dest_id = EMN_MAC_TASK;
    EMN_SendMsg.msg_id = EMN_MAC_DIS_NETWORK_REQ;
    SYS_SendMsg(&EMN_SendMsg);    
#endif	
} 	

//after handling dis_network,then start to setup network
void EMN_MAC_DisNetwork(void)
{
  MAC_DisNetTime=0;
  EMN_SendMsg.dest_id = EMN_MAC_TASK;
  EMN_SendMsg.msg_id = EMN_MAC_DIS_NETWORK_REQ;
  SYS_SendMsg(&EMN_SendMsg);	
}
extern EMN_NWK_STATE  EMN_NWK_State;
void EMN_MAC_Task(SYS_MESSAGE *msg)
{
  switch(MAC_State)
  {
    case EMN_MAC_CON: 
		  switch(msg->msg_id)
		  {
		    case  EMN_MAC_DIS_NETWORK_REQ:
				  DEBUG_DIS(printf("\ndis_net:start"));
				  MAC_DisNetwork();				  
			 	  break;
         //from NWK layer message
			case EMN_MAC_SCAN_REQ:
				// if(msg->data[0]== 0x01)
				 {
				   MAC_State = EMN_MAC_SCAN_MST0;
				   PHY_WriteRxAddr(0xff,0xff);// write scan network address
				   MasterScanProcess(msg);
				 }
				  break;
			case EMN_MAC_DATA_REQ:  // must send EMN_MAC_DATA_CFM to high layer
			     //if(msg->data[2]==EMN_OK)
			     	{
			         MAC_MacDataReqHal(msg);
			     	}
                  break;
            case EMN_MAC_DATA_RES:
				  EMN_SendMsg.dest_id = EMN_PHY_TASK;
				  EMN_SendMsg.msg_id = EMN_PHY_RX_RES;
				  EMN_SendMsg.data[0]= msg->data[0];  
				  EMN_SendMsg.data[1]= msg->data[1];
				  SYS_SendMsg(&EMN_SendMsg); 		
				  break;
        //from internal MAC	
            case EMN_TIMEOUT :
					
			      if(msg->data[0] == SYS_TIMEOUT_RESEND)
				  {	
#if DICOR_ACK_RESEND
                   SYS_StopTimer(&EMN_MacTimer);
				   //resend the TX data
                    MAC_ResendTimeoutCt++;
                    if(MAC_ResendTimeoutCt>2)
                    {
   			           MAC_ResendTimeoutCt=0;
					   MAC_ReSendBuf.which = 0x00;
					   MAC_ReSendBuf.frm_num = 0x00;
					 //  DEBUG_DIS(printf("\nstop resend"));
                    }else if (MAC_ReSendBuf.which != 0x00)
                    	{                    	
        			    EMN_SendMsg.dest_id = EMN_PHY_TASK;
				        EMN_SendMsg.msg_id = EMN_PHY_TX_REQ;
				        EMN_SendMsg.data[0]= MAC_ReSendBuf.which;  
				        EMN_SendMsg.data[1]= MAC_ReSendBuf.frm_num;					
					    EMN_SendMsg.data[2] = MAC_ReSendBuf.net_addr;
				        EMN_SendMsg.data[3]= MAC_ReSendBuf.subnet_addr;
					    DEBUG_DIS(printf("\nresend"));
				        SYS_SendMsg(&EMN_SendMsg); 
                    	}
#endif 
                    ;
				 }else if(msg->data[0] == SYS_TIMEOUT_DIS_NET)  
				 	{
                       MAC_DisNetTime++;
					   if(MAC_DisNetTime>30)
					   {				   
					      EMN_SendMsg.dest_id = EMN_NWK_TASK;
				          EMN_SendMsg.msg_id = EMN_MAC_DIS_NETWORK_CFM;	
 					      EMN_NWK_State = EMN_NWK_CORD_DIS_SCAN;  //before network ready	
				          SYS_SendMsg(&EMN_SendMsg);
						  DEBUG_DIS(printf("\ndis_net:end"));
					   }else
					   	{
                            MAC_DisNetwork();		
					   	}
				 	}
		//from phy layer		
		          break;
            case EMN_PHY_RX_IND:  
#if 1           
                  MAC_PhyDataInd(msg);
                  EMN_SendMsg.dest_id = EMN_PHY_TASK;
                  EMN_SendMsg.msg_id = EMN_PHY_RX_RES;
                  EMN_SendMsg.data[0]= msg->data[0];  
                  EMN_SendMsg.data[1]= msg->data[1];  
                  SYS_SendMsg(&EMN_SendMsg);
#else           //for test                  
   				  EMN_SendMsg.dest_id = EMN_PHY_TASK;
				  EMN_SendMsg.msg_id = EMN_PHY_RX_RES;
				  EMN_SendMsg.data[0]= msg->data[0];  
				  EMN_SendMsg.data[1]= msg->data[1];
				  SYS_SendMsg(&EMN_SendMsg); 		
#endif 
				  break;
			case EMN_PHY_TX_CFM:
#if DICOR_ACK_RESEND
                  if((MAC_ReSendBuf.which != 0x00)&&
				  	(MAC_ReSendBuf.frm_num != 0x00))
				   {
				       MAC_PhyTxCfm();
					   //DEBUG_DIS(printf("\nstart resend"));
				   }else
				   	{                             // stop resend timer
                      MAC_STOP_RESEND_TIMER();
	                  //release resend buffer
	                  MAC_ResendTimeoutCt=0;
	                  MAC_ReSendBuf.frm_num = 0x00;
	                  MAC_ReSendBuf.subnet_addr = 0x00;
  	                  MAC_ReSendBuf.net_addr = 0x00;
	                  MAC_ReSendBuf.which = 0x00;
				   	}
				   	
#endif		

				  break;
			default : break; 
		  }
		  break;
   
	case EMN_MAC_SCAN_MST0:
	case EMN_MAC_SCAN_MST1:
//	case EMN_MAC_SCAN_MST2:
	case EMN_MAC_SCAN_MST3:
	case EMN_MAC_SCAN_MST4:
		   MasterScanProcess(msg);
		   break;
    case EMN_MAC_SCAN_M_END:
           MasterScanEndProcess(msg);
		   break;
	case EMN_MAC_SCAN_SST0:
	case EMN_MAC_SCAN_SST1:
//	case EMN_MAC_SCAN_SST2:
	case EMN_MAC_SCAN_SST3:
		   SlaveScanProcess(msg);
		   break;
    case EMN_MAC_SCAN_S_END:
	       SlaveScanEndProcess(msg);
		   break;
    default: break;
      
  }
}


/* EOF */

