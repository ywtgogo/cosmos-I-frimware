/**********************************************************************
* 
* 	Copyright (c) 2010 ;
*	 All Rights Reserved convertergy
* 	FileName: network_nwk.c
* 	Version : 
*	 Date    : 
*
* 	Comments: 1) In order to descrease system overhead, implements  application's function 
*                       in NWK layer. 
*             2) In this layer, for SMC and EMT, they are different, and the code in this file 
*                          includes their implementations.
*
*************************************************************************/
#include "network_nwk.h"
#include "dicor_network_apl.h"
#include "dicor_upload.h"
#include "lostlogging_public.h"  
#include <string.h>
#include "upgradedidi.h"
#ifdef DI_HARVESTER_V4
#include "unconnected_grid.h"
#endif   
#ifdef DI_HARVESTER_V3
#include "di-harvest.h"
#endif 
#include "breakpoint.h"
extern void HandleData( UINT_8 subnet, UINT_8 dev,UINT_16 power_data);

//extern const UINT_8 SubnetDepth[];//add by helen for test lost packet
extern UINT_8* SubnetDepth;
static void NWK_CordSetStatusReq(UINT_8 subnet,UINT_8 dev,ParaType *value);
void NWK_CordSetDataReq(EMN_SUBNET_ADDR addr,UINT_8 index,UINT_8 data);
void NWK_CordAllocAddrReq(UINT_8 subnet,UINT_8 dev,UINT_8 new_net);	  
void NWK_CordCheckRegReq(EMN_SUBNET_ADDR dev_addr,EMN_DEV_REG_INFO_ST *dev_infor);
void NWK_CordFindNewerReq(UINT_8 subnet,UINT_8 dev,UINT_8 scan_net,UINT_8 scan_subnet);
void NWK_WriteRegInfo(void);
void NWK_WriteSampleData(UINT_8 *data);


static UINT_8 Check_1nums( UINT_32 x );//add by helen for test lost packet
static void EMN_STATICS_LostPacket(void);
UINT_8 ChkSaveLog(DATE_STRUCT* date);


EMN_NWK_STATE  EMN_NWK_State;
static   SYS_TIMER      EMN_NwkTimer;

UINT_8 Last_Child;//add by helen for test lost packet
UINT_8  Last_SubAddr=0;
UINT_32 receive_flag=0;
UINT_8 childnum=0;
UINT_8 childcount=0;
UINT_8 flag_rev_ack=1;//whether or not receive one device ack flag
UINT_8 flag_rev_set_ack=1;//
UINT_8 Last_Devaddr=0xff;
UINT_8 Rev_Endflag=1;
UINT_8 LastSetSubnet,LastSetDev;
static UINT_16 MaxUin=0,MaxUout=0;
//NWK layer's message buffer I
 UINT_8   NWK_ComBufIndex;
 UINT_8   NWK_ComBuf[EMN_PHY_PAYLOAD_LEN];

//Tx buffer
 UINT_8  NWK_TxBufIndex;
 UINT_8  NWK_TxBufFrmNum=0;
 UINT_8  NWK_UPGRADE_TxBufFrmNum;

 UINT_8  NWK_TxBuf[EMN_NWK_TXBUF_COUNT][EMN_PHY_PAYLOAD_LEN];/*可能是实际的数据--Lewis*/

 
 UINT_8	RF_Data_CurIndex=0;
extern UINT_8	*RF_Data_From;
extern UINT_16 DiDiFlashStartAddr;
extern UINT_16 DiDiSendDataLen;
extern LWSEM_STRUCT UpgradeStateSem;
extern INT_8 UpgradeState;




 static UINT_8 NWK_CurSampleDev;
 extern UINT_32  APL_waitetime;
 #if 0
 // register database management
 UINT_16 EMN_RegDevSum;
 UINT_16 EMN_RegNetSum;
 UINT_8  EMT_CurSampleNum;
 UINT_8  EMT_CurPos;// 
 UINT_8  EMT_SdCard[512];
 //handle all data from all device 
 UINT_8  EMT_TmpData[512];
 UINT_16 EMT_TmpDataIndex;

 #endif
 UINT_8  EMN_NetworkOK;

static uint_8 Check_1nums( uint_32 x )
{
  uint_8 countx = 0;
  while( x )
  {
    countx ++;
    x = x & ( x-1 );
  }
  return countx;
} 

/*********************************************************************
* 
* Function Name    : 
* Returned Value    : 
* Comments          :
*  
*
**********************************************************************/

/***************************************************
* 
*  management of register table, it is stored in SD card
*
****************************************************/
void NWK_WriteRegInfo(void)
{
   upload_buffer.data0.didi_data[upload_buffer.write_index++]=ScanDevUID[0];
   upload_buffer.data0.didi_data[upload_buffer.write_index++]=ScanDevUID[1];
   upload_buffer.data0.didi_data[upload_buffer.write_index++]=ScanDevUID[2];
   upload_buffer.data0.didi_data[upload_buffer.write_index++]=ScanDevUID[3];   
   //because ID have increased
   upload_buffer.data0.didi_data[upload_buffer.write_index++]=ScanSubnetAddr;
   upload_buffer.data0.didi_data[upload_buffer.write_index++]=ScanDevAddr-1;   
}

extern SYS_TIMER      EMN_AplTimer;
static void NWK_ParaData(UINT_8 *data)
{
  UINT_8 len,i,index;
   GetParaData.flag=1;
  index =6;
  len = data[index+1];
  DEBUG_DIS(printf("\n len=%d\n",len));	 
  for(i=0;i<len-1;i++)
  	{
  	  GetParaData.data[i]=data[index+3+i];
  	}
}
static void NWK_MacDataCfmHal(SYS_MESSAGE *msg)
{
 UINT_8 Comand_id,frm_num,subnet,dev;
 UINT_8 *resendfrm;
 frm_num=msg->data[1];
//  printfint("\n ****msg->data[0]==%d*****\n",msg->data[0]);
 if(msg->data[0]==EMN_TXBUF_NWKTX)
 {
 	resendfrm = &NWK_TxBuf[frm_num-1][MAC_FRM_SUB_DATA0];
 	subnet=NWK_TxBuf[frm_num-1][MAC_FRM_SUB_SOUR_SUB];
 	dev=NWK_TxBuf[frm_num-1][MAC_FRM_SUB_SOUR_DEV];
	//Comand_id=
 }else if(msg->data[0]==EMN_TXBUF_MACCOM||msg->data[0]==EMN_TXBUF_NWKCOM)
  	{
  	  resendfrm = &(MAC_ComBuf[MAC_FRM_SUB_DATA0]);
  	  subnet=MAC_ComBuf[MAC_FRM_SUB_SOUR_SUB];
 	  dev=MAC_ComBuf[MAC_FRM_SUB_SOUR_DEV];
 	}else if(msg->data[0]==EMN_TXBUF_MACCOM2)
 		{
 		   resendfrm =&(MAC_ComBuf2[MAC_FRM_SUB_DATA0]);
 		   subnet=MAC_ComBuf2[MAC_FRM_SUB_SOUR_SUB];
 	       dev=MAC_ComBuf2[MAC_FRM_SUB_SOUR_DEV];
 		}
     Comand_id=(*resendfrm);
 if(Comand_id == EMN_NWK_UPGRADE_DATA_CFM)
 	{
 	   if(msg->data[0]==EMN_TXBUF_NWKTX)  //subnet data
	  { 
	  //   printfint("****EMN_TXBUF_NWKTX*****");
	     if(msg->data[2]==EMN_OK)//send next data
	     {
	        
	     }
		 else if(msg->data[2]==EMN_ERROR)//resend or send message to apl
	 	{
	 	    DEBUG_DIS(printf("\n msg->data[2]==EMN_ERROR\n"));	 
	 	}
	  }
 	}

     if(msg->data[0]==EMN_TXBUF_NWKTX)  //subnet data
	  {
	 //    printfint("****EMN_TXBUF_NWKTX*****");
	     if(msg->data[2]==EMN_OK)
	     {
	     }else
	     	{
	    //     printfint("****EMN_TXBUF_NWKTX send error*****");
	     	}

	  }else if(msg->data[0]==EMN_TXBUF_NWKCOM) //subnet command
	     {
	       if(msg->data[2]==EMN_ERROR)
         	{
        // 	 printfint("\n****set subdevice no response NWK_CurSampleDev=%d*****\n",NWK_CurSampleDev);
         	// return;
         	}	      
	     }
		
}


//static
 void NWK_WriteSampleData2(UINT_8 *data)
{
	UINT_8  index,i,len,m,position=0;
	UINT_8  frag_len;
	UINT_8  Last_subnet=0;
	UINT_32   buffer_startpoint;
	UINT_8  *tempdata;
	UINT_8  temp_subnet,temp_dev,temp_len;
	//data=&NWK_TxBuf[0][0];//get the first data
	len = data[7];
	index =10;  //for multiple frame handle
     //WriteTest();
		for(m=0;m<frag_len;m++)
		{
			upload_buffer.data0.didi_data[upload_buffer.write_index++]=data[index++];
			if(index%32==0)              { index++;}//discard frame header
			i++;
		}
		_mutex_unlock(&upload_buffer.mutex);
	//  if(_mutex_lock(&upload_buffer.mutex) != MQX_OK) 

// {

//      DEBUG_DIS(printf("\n system error, Mutex lock failed.\n"));

//  }
	for(i=2;i<len;i++)
	{
		tempdata = &data[index];
		if(!(data[index]==0x20||data[index]==0x10||data[index]==0x30||data[index]==0x31||data[index]==0x32||data[index]==0x33||data[index]==0x34||data[index]==0x21||data[index]==0x22||data[index]==0x23||data[index]==0x24||data[index]==0x11))
			break;
		if((index+1)%32==0)
		{
			temp_len = tempdata[1+1];
			temp_subnet = tempdata[2+1];
			temp_dev = tempdata[3+1];
		}
		else if((index+2)%32==0)
		{
			temp_len = tempdata[1];
			temp_subnet = tempdata[2+1];
			temp_dev = tempdata[3+1];
		}
		else if((index+3)%32==0)
		{
			temp_len = tempdata[1];
			temp_subnet = tempdata[2];
			temp_dev = tempdata[3+1];
		}
		else
		{
			temp_len = tempdata[1];
			temp_subnet = tempdata[2];
			temp_dev = tempdata[3];
		}
		frag_len = temp_len;
		#if COUNT_LOSTPACKET//add  by helen for test lost packet
		if(temp_subnet!=EMN_SubnetAddr)
		{
			receive_flag|=(1<<temp_dev);
			DEBUG_DIS(printf("\n temp_dev=%d.\n",temp_dev));
		}
		else
		{
			receive_flag=0;
		}
		#endif
#if (DIS_DIAN_NUM!=0)
		if(data[index]==0x20||data[index]==0x30)  //it is data from DiAn/DiAn_t
		{
			HandleData( data[index+2], data[index+3],(UINT_16)((UINT_16)data[index+12]<<8+(UINT_16)data[index+13]));
		}
#endif
		//    buffer is full
		if(upload_buffer.write_index>=(DICOR_UPLOAD_BUFFER_SIZE-frag_len))
		{
			upload_buffer.state=CAN_READ;
			SYS_StopTimer(&EMN_AplTimer);
/*        _mutex_unlock(&upload_buffer.mutex);    
//         dicor_rf_signal(RF_GET_DATA_END);
//         dicor_waite_rf(NET_SEND_DATA_END,0);
if(_mutex_lock(&upload_buffer.mutex) != MQX_OK)
{
DEBUG_DIS(printf("\n system error, Mutex lock failed.\n"));
}*/
			APL_waitetime=0;
			dicor_waite_rf(NET_SEND_DATA_END,0);
		}
		if(_mutex_lock(&upload_buffer.mutex) != MQX_OK)
		{
			DEBUG_DIS(printf("\n system error, Mutex lock failed.\n"));
		}
		if(workmode == WORKMODE_DIHAVST4)
		{
			buffer_startpoint=upload_buffer.write_index;
		}
	//	for(m=0;m<frag_len;m++)
	//	{
	//		upload_buffer.data0.didi_data[upload_buffer.write_index++]=data[index++];
	//		if(index%32==0)              { index++;}//discard frame header
	//		i++;
	//	}
	//	_mutex_unlock(&upload_buffer.mutex);
		if(workmode == WORKMODE_DIHAVST4)
		{
			if(HAVST_State == DI_HAVST_COLLECT_DATA)
			{
				HAVST_CollectPlantInfo(&(upload_buffer.data0.didi_data[buffer_startpoint]),(upload_buffer.write_index-buffer_startpoint));
			}
		}
	}
}


static void NWK_CordStartUpdateCmdReq(UINT_8 subnet,UINT_8 dev)
{   
      EMN_MAC_FRAME *frm_p; 	  
	    DEBUG_DIS(printf("\n subnet=%d dev=%d\n",subnet,dev));	 
		frm_p =(EMN_MAC_FRAME*)(&NWK_ComBuf[0]);
		frm_p->nwk_h.dest.subnet = subnet;
		if(subnet == EMN_SubnetAddr)
		{
		  frm_p->nwk_h.dest.dev = dev;
		}else
		  {
			frm_p->nwk_h.dest.dev = 0x00;
		  } 
		frm_p->nwk_h.sour.subnet= EMN_SubnetAddr;
		frm_p->nwk_h.sour.dev = 0x00;
		frm_p->nwk_h.len =EMN_UPGRADE_CMD_SIZE;
		frm_p->nwk_h.data[0] =EMN_NWK_UPGRADE_CMD_REQ;
		frm_p->nwk_h.data[1] =dev;
		EMN_SendMsg.dest_id = EMN_MAC_TASK;
		EMN_SendMsg.msg_id = EMN_MAC_DATA_REQ;
		EMN_SendMsg.data[0] = EMN_TXBUF_NWKCOM;//from NWK layer command
		EMN_SendMsg.data[1] = 1;//num
		SYS_SendMsg(&EMN_SendMsg);
		

}

#if 0
{
     Rf_data_from=NUll;
	 RF_data_cur_index;
	 state;

	 handle send_data_req:
	 	 load_rf_data_to_nwk_buffer();             children: get ready to receiver
                                                               send ok ...
            DLC_send_data_cfm:
              star send data
	   handle_cfm:                     
	  	  one case: resend();
		  the other case: contituse;
		                : finish data trasmiter, start check
			
		     NWK_send_data_cfm:

	      send_data_check_req:
	      send_data_check_cfm:


}
#endif 
static void NWK_CordGetParaReq(UINT_8 subnet,UINT_8 dev,GetParaType *value)
{
	EMN_MAC_FRAME *frm_p;	
	// UINT_8	data_p;
	
	  //Make  GET_SAMPLE frame, and send it to specified subnet's header
	   DEBUG_DIS(printf("\n subnet=%d dev=%d\n",subnet,dev));
	 frm_p =(EMN_MAC_FRAME*)(&NWK_ComBuf[0]);
	 frm_p->nwk_h.dest.subnet = subnet;
	 if(subnet == EMN_SubnetAddr)
	 {
	   frm_p->nwk_h.dest.dev = dev;
	 }else
	   {
		 frm_p->nwk_h.dest.dev = 0x00;
	   } 
	 DEBUG_DIS(printf("\n value->dev=%d\n",value->dev));
	 frm_p->nwk_h.sour.subnet= EMN_SubnetAddr;
	 frm_p->nwk_h.sour.dev = 0x00;
	 frm_p->nwk_h.len =5;
	 frm_p->nwk_h.data[0] =EMN_NWK_GET_PARA_REQ;
	 frm_p->nwk_h.data[1] =value->dev;
	 frm_p->nwk_h.data[2] =value->flag;
	 frm_p->nwk_h.data[3] =value->index;
	 frm_p->nwk_h.data[4] =value->len;
	 frm_p->nwk_h.data[5] =value->data[0];
	  DEBUG_DIS(printf("\n value->len=%d\n",value->len));
		
	  //send MAC_DATA_REQ to mac layer
	 EMN_SendMsg.dest_id = EMN_MAC_TASK;
	 EMN_SendMsg.msg_id = EMN_MAC_DATA_REQ;
	 EMN_SendMsg.data[0] = EMN_TXBUF_NWKCOM;//from NWK layer command
	 EMN_SendMsg.data[1] = 1;//num
	 SYS_SendMsg(&EMN_SendMsg); 

}

static void NWK_CordSetStatusReq(UINT_8 subnet,UINT_8 dev,ParaType *value)
{
  EMN_MAC_FRAME *frm_p;	
  UINT_8   i;
  
  //Make  SET_DATA frame, and send it to specified subnet's header
 frm_p =(EMN_MAC_FRAME*)(&NWK_ComBuf[0]);
 frm_p->nwk_h.dest.subnet = subnet;
 if(subnet == EMN_SubnetAddr)
 {
   frm_p->nwk_h.dest.dev = dev;
 }else
   {
     if(value->brd==0x01)
     	{
     	 frm_p->nwk_h.dest.dev = 0x00;
	//	  DEBUG_DIS(printf("\n value->brd==1\n"));
     	}
	 else 
	 	{
          frm_p->nwk_h.dest.dev = 0x00;
	//DEBUG_DIS(printf("\n value->brd==0\n"));		 		
	 	}
   } 
 
 frm_p->nwk_h.sour.subnet= EMN_SubnetAddr;
 frm_p->nwk_h.sour.dev = 0x00;
 frm_p->nwk_h.len =value->len+6;
 frm_p->nwk_h.data[0] = EMN_NWK_SET_DATA_REQ;
 frm_p->nwk_h.data[1] = value->brd;
 frm_p->nwk_h.data[2] = value->flag;
 frm_p->nwk_h.data[3] = value->index;
 frm_p->nwk_h.data[4] = value->len;
 frm_p->nwk_h.data[5] = value->dev;
 //DEBUG_DIS(printf("\n value->index=%d\n",frm_p->nwk_h.data[3])); 
// DEBUG_DIS(printf("\n value->len=%d\n",frm_p->nwk_h.data[4])); 
 for(i=0;i<value->len;i++)
	{
	   frm_p->nwk_h.data[6+i]= value->data[i];
	  // DEBUG_DIS(printf("\n frm_p->data[%d]=%d\n",i,value->data[i])); 
	}


  //send MAC_STATUS_REQ to mac layer
 EMN_SendMsg.dest_id = EMN_MAC_TASK;
 EMN_SendMsg.msg_id = EMN_MAC_DATA_REQ;
 EMN_SendMsg.data[0] = EMN_TXBUF_NWKCOM;//from NWK layer command
 EMN_SendMsg.data[1] = 1;//num
 SYS_SendMsg(&EMN_SendMsg);	
}

/*----------------------------------------
  The following functions are to implement collecting data
  from all SMC 
  -----------------------------------------*/
static void NWK_CordGetSampleReq(UINT_8 subnet,UINT_8 dev)
{
  
  EMN_MAC_FRAME *frm_p;	
// UINT_8   data_p;

  //Make  GET_SAMPLE frame, and send it to specified subnet's header
 frm_p =(EMN_MAC_FRAME*)(&NWK_ComBuf[0]);
 frm_p->nwk_h.dest.subnet = subnet;
 if(subnet == EMN_SubnetAddr)
 {
   frm_p->nwk_h.dest.dev = dev;
 }else
   {
     frm_p->nwk_h.dest.dev = 0x00;
   } 

 frm_p->nwk_h.sour.subnet= EMN_SubnetAddr;
 frm_p->nwk_h.sour.dev = 0x00;
 frm_p->nwk_h.len =1;
 frm_p->nwk_h.data[0] =EMN_NWK_GET_SAMPLE_REQ;
 //frm_p->nwk_h.data[0] = EMN_NWK_GET_SAMPLE_CFM;
if(workmode == WORKMODE_DIHAVST4)
 {
	#ifdef EMN_GET_SAMPLE_WITH_COMMAND	
	  NWK_CordMakeCmdPacket(subnet,&(frm_p->nwk_h.data[1]));
	#endif 
}

//for(i=2;i<frm_p->nwk_h.data[2];i++)
// DEBUG_DIS(printf("\n DidiInfo[%d].cmd=0x%x",i,frm_p->nwk_h.data[i+2]));

  //send MAC_DATA_REQ to mac layer
 EMN_SendMsg.dest_id = EMN_MAC_TASK;
 EMN_SendMsg.msg_id = EMN_MAC_DATA_REQ;
 EMN_SendMsg.data[0] = EMN_TXBUF_NWKCOM;//from NWK layer command
 EMN_SendMsg.data[1] = 1;//num
 SYS_SendMsg(&EMN_SendMsg);	
	
}


#if 0  //	debug it in the future
void NWK_CheckRegTable(SYS_MESSAGE *msg)
{
 EMN_MAC_FRAME *frm_p;

  switch(msg->msg_id)
  { 
    case  EMN_NWK_CHECK_REG_REQ: // from init function or higher layer
			 CurIndex= msg->data[0];
			 if(CurIndex < EMN_RegDevSum)
			 {
			  //note: TmpData[0][0] is for MAC header
			  frm_p =(EMN_MAC_FRAME*)(&NWK_ComBuf[0]);
			  frm_p->nwk_h.dest.subnet = EMN_RegTab[CurIndex].net_addr;
			  frm_p->nwk_h.dest.dev = EMN_RegTab[CurIndex].dev_addr;
			  frm_p->nwk_h.sour.subnet= 0x00;
			  frm_p->nwk_h.sour.dev =0x00;
 			  data_P = &(head->data);
			  *data_P++ = EMN_NWK_CHECK_REG_REQ;
			  *data_P++ = EMN_RegTab[CurIndex].uid[0];
			  *data_P++ = EMN_RegTab[CurIndex].uid[1];
			  *data_P++ = EMN_RegTab[CurIndex].uid[2];
			  *data_P++ = EMN_RegTab[CurIndex].uid[3];
			   head->flag.dir=0;	//down
			   head->flag.len = 5;
		  
					 EMN_SendMsg.dest_id = EMN_MAC_TASK;
					 EMN_SendMsg.msg_id = EMN_MAC_DATA_REQ;
					 EMN_SendMsg.data_p =(UINT_8*)(&TmpData[0][0]);
					 SYS_SendMsg(&EMN_SendMsg);
					}else{
						 // let coordinator to scan its RF range for new comer				 
						 EMN_SendMsg.dest_id = EMN_NWK_TASK;
						 EMN_SendMsg.msg_id = EMN_NWK_SCAN_RANGE_REQ;
						 EMN_SendMsg.data[0] =0x00;
						 EMN_SendMsg.data[1] =0x00;
						 EMN_NWK_State = EMN_NWK_CON_COORD; 
						 SYS_SendMsg(&EMN_SendMsg);
						}
				   break;
		  case	EMN_MAC_DATA_CFM:		
				   // start Check register timer
				  EMN_NwkTimer.value = MNK_CHECK_REG_TIME;
				  EMN_NwkTimer.msgID = SYS_TIMEOUT_NWK_CHECK_REG;
				  SYS_StartTimer(&EMN_NwkTimer);	 
				  break;
		  case	EMN_MAC_DATA_IND:
				 SYS_StartTimer(&EMN_NwkTimer);
				 head = (EMN_NWK_FRAME*)(&msg->data_p);
				 if(head->data == EMN_NWK_CHECK_REG_CFM)
				 {
					data_P = &head->data;
					if(*data_P == 0x01&& 
						head->sour.subnet == EMN_RegTab[CurIndex].net_addr&& 
						head->sour.dev == EMN_RegTab[CurIndex].dev_addr) //succees
					{
						; //it is here.
					}else
					  {
						EMN_RegTab[CurIndex].dev_addr =0x00; //delete this item
						EMN_RegTab[CurIndex].net_addr =0x00;
						EMN_RegTab[CurIndex].uid[0]= 0x00;
						EMN_RegTab[CurIndex].uid[1]= 0x00;
						EMN_RegTab[CurIndex].uid[2]= 0x00;
						EMN_RegTab[CurIndex].uid[3]= 0x00;						
					  }
					//check the next
					do{
						CurIndex++;
					}while(EMN_RegTab[CurIndex].dev_addr == 0x00&&
					   EMN_RegTab[CurIndex].net_addr == 0x00);	
					EMN_SendMsg.dest_id = EMN_NWK_TASK;
					EMN_SendMsg.msg_id = EMN_NWK_CHECK_REG_REQ;
					EMN_SendMsg.data[0] = CurIndex;
					SYS_SendMsg(&EMN_SendMsg);
				 }
	
			 break;
		case	EMN_TIMEOUT :
			if(msg.data[0] == SYS_TIMEOUT_NWK_CHECK_REG)
			{
			   EMN_SendMsg.dest_id = EMN_NWK_TASK;
			   EMN_SendMsg.msg_id = EMN_NWK_CHECK_REG_REQ;
			   EMN_SendMsg.data[0] = CurIndex++;
			   SYS_SendMsg(&EMN_SendMsg);
			}
		
		default :break; 	   
	  }
	
	}
#endif 
#if 0

void NWK_CheckRegCfm(EMN_NWK_FRAME *frame)
{

}
#endif 

//note: parameter dev
void NWK_CordFindNewerReq(UINT_8 subnet,UINT_8 dev,
                          UINT_8 scan_net,UINT_8 scan_subnet)
{	

 EMN_MAC_FRAME *frm_p;	
 if(subnet ==0x00)//scan itself
 {
   //send scan_req to mac layer directly
   ScanNetAddr=scan_net;
   ScanSubnetAddr=scan_subnet;
   ScanDevAddr=dev;
   ScanDevNum=0;
//   PHY_WriteRxAddr(0xff,0xff);

   EMN_SendMsg.dest_id = EMN_MAC_TASK;
   EMN_SendMsg.msg_id = EMN_MAC_SCAN_REQ;
   SYS_SendMsg(&EMN_SendMsg);  
    DEBUG_DIS(printf("\nstart scan"));
 }else{// send  this command to specified device
 
    //Make frame
   frm_p =(EMN_MAC_FRAME*)(&NWK_ComBuf[0]);
   frm_p->nwk_h.dest.subnet = subnet;
   frm_p->nwk_h.dest.dev = 0x00;
   frm_p->nwk_h.sour.subnet= EMN_SubnetAddr;
   frm_p->nwk_h.sour.dev = EMN_DevAddr;
   frm_p->nwk_h.data[0] = EMN_NWK_FIND_NEWER_REQ;
   frm_p->nwk_h.data[1]=dev;   
   frm_p->nwk_h.data[2]=scan_net;
   frm_p->nwk_h.data[3]=scan_subnet;
   EMN_SendMsg.dest_id = EMN_MAC_TASK;
   EMN_SendMsg.msg_id = EMN_MAC_DATA_REQ;
   EMN_SendMsg.data[0] = EMN_TXBUF_NWKCOM;//where
   EMN_SendMsg.data[1] = 1;//num
   SYS_SendMsg(&EMN_SendMsg);	
 
 }
 
}



//send one request every one subnet, In case that EMT has not received data from specified device
//for 3times, it can send this message to its header or up router to check if it is availble.
void NWK_CordCheckRegReq(EMN_SUBNET_ADDR dev_addr,
                                    EMN_DEV_REG_INFO_ST *dev_infor)
{
 EMN_MAC_FRAME *frm_p;	
 //UINT_8 *data_p;
  //Make frame
 frm_p =(EMN_MAC_FRAME*)(&NWK_ComBuf[0]);
 frm_p->nwk_h.dest.subnet = dev_addr.subnet;
 frm_p->nwk_h.dest.dev = 0x00;
 frm_p->nwk_h.sour.subnet= 0x00;
 frm_p->nwk_h.sour.dev = 0x00;

 //data_p = &(frm_p->nwk_h.data[0]);
 frm_p->nwk_h.data[0] = EMN_NWK_CHECK_REG_REQ;
 frm_p->nwk_h.data[1] = dev_infor->net_addr;
 frm_p->nwk_h.data[2]= dev_infor->dev_addr;
 frm_p->nwk_h.data[3]= dev_infor->uid[0];
 frm_p->nwk_h.data[4]= dev_infor->uid[1];
 frm_p->nwk_h.data[5]= dev_infor->uid[2];
 frm_p->nwk_h.data[6]= dev_infor->uid[3];
	
  //send MAC_DATA_REQ to mac layer
 EMN_SendMsg.dest_id = EMN_MAC_TASK;
 EMN_SendMsg.msg_id = EMN_MAC_DATA_REQ;
 EMN_SendMsg.data[0] = EMN_TXBUF_NWKCOM;//from NWK layer command
 EMN_SendMsg.data[1] = 1;//num
 SYS_SendMsg(&EMN_SendMsg);	
	
}

//it is for device
void NWK_CordAllocAddrReq(UINT_8 subnet,UINT_8 dev,
                                    UINT_8 new_net)
{

 EMN_MAC_FRAME *frm_p;	
 
  //Make frame
 frm_p =(EMN_MAC_FRAME*)(&NWK_ComBuf[0]);
 frm_p->nwk_h.dest.subnet = subnet;
 frm_p->nwk_h.dest.dev = 0x00;
 frm_p->nwk_h.sour.subnet= EMN_SubnetAddr;
 frm_p->nwk_h.sour.dev = EMN_DevAddr;

 frm_p->nwk_h.data[0]= EMN_NWK_ALLOC_ADDR_REQ;
 frm_p->nwk_h.data[1]= ROUTER_TABLE_ADD_ITEM;
 frm_p->nwk_h.data[2]= new_net;
 frm_p->nwk_h.data[3]= dev;

 frm_p->nwk_h.len =4;
	
 //send MAC_DATA_REQ to mac layer
 EMN_SendMsg.dest_id = EMN_MAC_TASK;
 EMN_SendMsg.msg_id = EMN_MAC_DATA_REQ;
 EMN_SendMsg.data[0] = EMN_TXBUF_NWKCOM;//from NWK layer command
 EMN_SendMsg.data[1] = 1;//num
 SYS_SendMsg(&EMN_SendMsg);	
	
}

//this function only can set one byte, and the index is onebyte
void NWK_CordSetDataReq(EMN_SUBNET_ADDR addr, 
	                             UINT_8 index,UINT_8 data)
{
 EMN_MAC_FRAME *frm_p;	

  //Make frame
 frm_p =(EMN_MAC_FRAME*)(&NWK_ComBuf[0]);
 frm_p->nwk_h.dest.subnet = addr.subnet;
 frm_p->nwk_h.dest.dev = addr.dev;
 frm_p->nwk_h.sour.subnet= 0x00;
 frm_p->nwk_h.sour.dev = 0x00;
 frm_p->nwk_h.len =1;
 frm_p->nwk_h.data[0] = EMN_NWK_SET_DATA_REQ;
 frm_p->nwk_h.data[1] = index;
 frm_p->nwk_h.data[2] = data;
 
 //send MAC_DATA_REQ to mac layer
 EMN_SendMsg.dest_id = EMN_MAC_TASK;
 EMN_SendMsg.msg_id = EMN_MAC_DATA_REQ;
 EMN_SendMsg.data[0] = EMN_TXBUF_NWKCOM;//from NWK layer command
 EMN_SendMsg.data[1] = 1;//num
 SYS_SendMsg(&EMN_SendMsg);	
	
}

//only send this request to the subnet header 
#if 0
void NWK_CordUpdateRouterTabReq(UINT_8 subnet,UINT_8 up_port,
	                                   EMN_ROUTER_TAB_ITEM *data, UINT_8 num)
{
 EMN_MAC_FRAME *frm_p;	
 UINT_8   *data_p,i;

  //Make frame
 frm_p =(EMN_MAC_FRAME*)(&NWK_ComBuf[0]);
 frm_p->nwk_h.dest.subnet = subnet;
 frm_p->nwk_h.dest.dev = 0x00;
 frm_p->nwk_h.sour.subnet= 0x00;
 frm_p->nwk_h.sour.dev = 0x00;
 data_p = &(frm_p->nwk_h.data[0]);
 *data_p++ = EMN_NWK_UPDATE_RUOTER_TAB_REQ;
 *data_p++ = up_port;
 *data_p++ = num;
 for(i=0;i<num; i++)
 {
   *data_p++ = data->max;
   *data_p++ = data->min;
   *data_p++ = data->addr;
   data++;
 } 
	
  //send MAC_DATA_REQ to mac layer
 EMN_SendMsg.dest_id = EMN_MAC_TASK;
 EMN_SendMsg.msg_id = EMN_MAC_DATA_REQ;
 EMN_SendMsg.data[0] = EMN_TXBUF_NWKCOM;//from NWK layer command
 EMN_SendMsg.data[1] = 1;//num
 SYS_SendMsg(&EMN_SendMsg);	
	
}
#endif 	
#if COUNT_LOSTPACKET //add  by helen for test lost packet

static void EMN_STATICS_LostPacket(void)
{
	UINT_8 i;
	DATE_STRUCT date;
	childcount = Check_1nums(receive_flag);
	
	//DEBUG_DIS(printf("\n childcount=%d childnum=%d\n",childcount,childnum));
	if((childnum==0)&&(childcount<1))
	{ 
	 DEBUG_DIS(printf("\n subaddr=%d lost %d child data.\n",Last_SubAddr,1));
	 if (ChkSaveLog(&date))//[6,18)丢包保存
	 {
	 	//
	 	sprintf(pLostLog->logbuf, "t%02d:%02d:%02d\t subaddr=%02X lost %d child data.\r\n", date.HOUR,date.MINUTE,date.SECOND,Last_SubAddr,1);
		PrintLost(pLostLog->logbuf);
		//fwrite(LostLog.logbuf, strlen(LostLog.logbuf), 1, LostLog.fd_ptr);
	 }
	} 					
	else if(childcount<childnum+1)
	{
	  DEBUG_DIS(printf("\n subaddr=%d lost %d child data.",Last_SubAddr,childnum+1-childcount));
	  DEBUG_DIS(printf(":"));
      for(i=0;i<(childnum+1);i++)
	 	{
	 	//  DEBUG_DIS(printf("\n receive_flag=%d",receive_flag));
	 	  if(!(receive_flag&(1<<i)))
	 	  	{
			 DEBUG_DIS(printf(" 0x%x ",i)); 
	 	  	}
	  	}

	  	if (ChkSaveLog(&date))//[6,18)丢包保存
		 {
		 	//
		 	
		 	sprintf(pLostLog->logbuf, "t%02d:%02d:%02d\t subaddr=%02X lost %d child data:", date.HOUR,date.MINUTE,date.SECOND,Last_SubAddr,childnum+1-childcount);
			PrintLost(pLostLog->logbuf);
			//fwrite(LostLog.logbuf, strlen(LostLog.logbuf), 1, LostLog.fd_ptr);

			for(i=0;i<(childnum+1);i++)
	 		{
		 	  if(!(receive_flag&(1<<i)))
		 	  	{
				 	sprintf(pLostLog->logbuf, " 0x%02X",i);
				 	PrintLost(pLostLog->logbuf);
					//fwrite(LostLog.logbuf, 5, 1, LostLog.fd_ptr);
		 	  	}
	  		}
			sprintf(pLostLog->logbuf, "\r\n");
			PrintLost(pLostLog->logbuf);
			//fwrite(LostLog.logbuf, 2, 1, LostLog.fd_ptr);
		 }
		
	  }		  
	receive_flag=0;
}
#endif
void EMN_NWK_TaskInit(void)
{
	 //init timer for this task using
 EMN_NwkTimer.id = SYS_TIMER1;
 EMN_NwkTimer.fastfun = NULL;
 EMN_NwkTimer.taskID = EMN_NWK_TASK;
 EMN_NwkTimer.value =0;
 EMN_NwkTimer.msgID = 0;

#if DICOR_STATIC_REG_TAB
 EMN_NWK_State = EMN_NWK_CORD_CON;  //before network ready	
 EMN_NetworkOK =0;
 
#else
 //scan its RF's range for new commer	 
 EMN_NWK_State = EMN_NWK_CORD_DIS_SCAN;  //before network ready	
 EMN_NetworkOK =0;
#endif  
}


//this task is for EMT	  

void EMN_NWK_Task(SYS_MESSAGE *msg)
{
 EMN_MAC_FRAME *rx_frm_p;
 UINT_8 *data_p,j; 
 UINT_8 position=0;
 //ParaType *temp;
 DATE_STRUCT date;
 
 switch(EMN_NWK_State)
 {
   case  EMN_NWK_CORD_CON:
	
	
         switch(msg->msg_id)
	     {
	      
	     
	       case EMN_NWK_SET_DATA_REQ:
	   	  NWK_CordSetStatusReq(msg->data[0],msg->data[1],&NwkSetParaData);
#if COUNT_LOSTPACKET 
          if((msg->data[0]!=LastSetSubnet)||(msg->data[1]!=LastSetDev))
          	{
          	  if(flag_rev_set_ack==0)			      	  
			   	{
			     DEBUG_DIS(printf("\n subaddr=%d lost NO %d child data.\n",LastSetSubnet,LastSetDev));
					
					if (ChkSaveLog(&date))//[6,18)丢包保存
						 {

						 	sprintf(pLostLog->logbuf, "t%02d:%02d:%02d\t subaddr=%02X lost NO. %02X child data.\r\n", date.HOUR,date.MINUTE,date.SECOND,LastSetSubnet,LastSetDev);
							PrintLost(pLostLog->logbuf);
							//fwrite(LostLog.logbuf, strlen(LostLog.logbuf), 1, LostLog.fd_ptr);
						 }
			  	}
			  else
			  	{
			  	 flag_rev_set_ack=0;
			  	}
          	}
          LastSetSubnet= msg->data[0];
          LastSetDev=msg->data[1];
#endif
			  break;
		  case EMN_NWK_GET_PARA_REQ:
		    NWK_CordGetParaReq(msg->data[0],msg->data[1],&GetParaData);
		  	   break;
		  case EMN_NWK_UPGRADE_CMD_REQ:
		  	NWK_CordStartUpdateCmdReq(msg->data[0],msg->data[1]);
		  	 break;		  	      	  
           case EMN_NWK_GET_SAMPLE_REQ:
		 
#if COUNT_LOSTPACKET               
		   if(SYS_SUBNET_NUM!=0)
			{
				for(j=0;j<SYS_SUBNET_NUM;j++)
				{		 
				 if(SubnetDepth[position]==Last_SubAddr)
					{
						childnum=SubnetDepth[position+2];
						break;
					}
				 else
					{
						 position+=3;
					}
				}															
			}	
		    if((Last_SubAddr==EMN_SubnetAddr)||((msg->data[0]==Last_SubAddr)&&(childnum==0)&&(SYS_SUBNET_NUM==1)))//dicor's children and one head with no child
	      	{
	      	  //  DEBUG_DIS(printf("flag_rev_ack %d \n",flag_rev_ack));
	      	  if(flag_rev_ack==0)			      	  
			   	{
			     DEBUG_DIS(printf("\n subaddr=%d lost NO %d child data.\n",Last_SubAddr,Last_Devaddr));
					if (ChkSaveLog(&date))//[6,18)丢包保存
				 {
				 	//
				 	sprintf(pLostLog->logbuf, "t%02d:%02d:%02d\t subaddr=%02X lost NO. %02X child data.\r\n", date.HOUR,date.MINUTE,date.SECOND,Last_SubAddr,Last_Devaddr);
					PrintLost(pLostLog->logbuf);
				//	fwrite(LostLog.logbuf, strlen(LostLog.logbuf), 1, LostLog.fd_ptr);
				
				 }

			  }
			  else
			  	{
			  	 flag_rev_ack=0;
			  	}
	      	}
			   else if(msg->data[0]!=Last_SubAddr)
			  	{
			  	// DEBUG_DIS(printf("this subAddr= %d\n ",msg->data[0]));
			  	 EMN_STATICS_LostPacket(); 
	   	        }
	   	        else if((msg->data[0]==Last_SubAddr)&&(childnum!=0)&&(SYS_SUBNET_NUM==1))//only have one header with many children
	   	     	{  
                 //  DEBUG_DIS(printf("\n Rev_Endflag=%d",Rev_Endflag));
				  if(Rev_Endflag==1)
	   	     	   {
	   	     	      EMN_STATICS_LostPacket();						
					}
				  else
				  	{
				  	  Rev_Endflag=1;//
				  	}
	   	        }
					childcount=0;
					childnum=0;
    				//receive_flag=0;//if is 0 ,continue get data error					
					flag_rev_ack=0;
				    Last_SubAddr=msg->data[0]; 
				    Last_Devaddr=msg->data[1];
#endif		   	  
				// DEBUG_DIS(printf("\n******EMN_NWK_GET_SAMPLE_REQ******\n"));
		   	     NWK_CordGetSampleReq(msg->data[0],msg->data[1]);
		   	      //EMN_SendMsg.msg_id =EMN_NWK_GET_SAMPLE_CFM;
		   	     break;  
		   case EMN_MAC_DATA_IND:
			 if(msg->data[0]==EMN_TXBUF_MACCOM)
	   	      {
				 rx_frm_p = (EMN_MAC_FRAME*)MAC_ComBuf;
		   	   }else if(msg->data[0]==EMN_TXBUF_NWKTX)	
	   	      	{
                   rx_frm_p = (EMN_MAC_FRAME*)(&NWK_TxBuf[0][0]);
	   	      	}
			   else
   	      		{
   	      		  break;
   	      		}
				  data_p = &(rx_frm_p->nwk_h.data[0]);
				  switch(*data_p)
				  {
				     case EMN_NWK_SET_DATA_CFM:	
#if COUNT_LOSTPACKET 
					 	   flag_rev_set_ack=1;
#endif					 	
						//	DEBUG_DIS(printf("\nEMN_NWK_SET_DATA_CFM=%d*data_p=%d",EMN_NWK_SET_DATA_CFM,*data_p));
							EMN_SendMsg.dest_id= EMN_APL_TASK;
                            EMN_SendMsg.msg_id =EMN_NWK_SET_DATA_CFM;
                            EMN_SendMsg.data[0]=rx_frm_p->nwk_h.sour.subnet;
                            EMN_SendMsg.data[1]=rx_frm_p->nwk_h.sour.dev;
                            SYS_SendMsg(&EMN_SendMsg); 
							break;
					 case EMN_NWK_GET_PARA_CFM:
					 	  DEBUG_DIS(printf("\EMN_NWK_GET_PARA_CFM msg->data[0]=%d",msg->data[0]));
					 	 if(msg->data[0]==EMN_TXBUF_MACCOM)
		   	                {    
		   	                    
						        // GetParaData.data=MAC_ComBuf;
				                 NWK_ParaData(MAC_ComBuf);
		   	                }else if(msg->data[0]==EMN_TXBUF_NWKTX)	
		   	      	        {
		   	      	       
						       //  GetParaData.data=&NWK_TxBuf[0][0];
                                 NWK_ParaData(&NWK_TxBuf[0][0]);
		   	      	        }else
		   	      		    {
		   	      		        break;
		   	      		      }		
					 	   EMN_SendMsg.dest_id= EMN_APL_TASK;
                            EMN_SendMsg.msg_id =EMN_NWK_GET_PARA_CFM;
                            EMN_SendMsg.data[0]=rx_frm_p->nwk_h.sour.subnet;
                            EMN_SendMsg.data[1]=rx_frm_p->nwk_h.sour.dev;
							SYS_SendMsg(&EMN_SendMsg);
					 	break;
					case EMN_NWK_UPGRADE_CMD_CFM:
						  EMN_SendMsg.data[0]=rx_frm_p->nwk_h.dest.subnet;
	                      EMN_SendMsg.data[1]=rx_frm_p->nwk_h.dest.dev;
						  EMN_SendMsg.dest_id= EMN_APL_TASK;
	                      EMN_SendMsg.msg_id = EMN_APL_START_UPGRADE_CFM;
						  EMN_SendMsg.data[2]= EMN_UPGRADE_CMD;
						  EMN_SendMsg.data[3]= rx_frm_p->nwk_h.data[1];
						  SYS_SendMsg(&EMN_SendMsg); 
						break;
					
					case EMN_NWK_SEND_DATA_CHECK_CFM:
						{
                          if(1)//check ok
                          	{
                          	  EMN_SendMsg.data[0]=rx_frm_p->nwk_h.sour.subnet;
                              EMN_SendMsg.data[1]=rx_frm_p->nwk_h.sour.dev;
							  EMN_SendMsg.dest_id= EMN_APL_TASK;
                              EMN_SendMsg.msg_id = EMN_NWK_SEND_DATA_CHECK_CFM;
							  SYS_SendMsg(&EMN_SendMsg);
                          	}
						  else //check error
						  	{
						  	  EMN_SendMsg.data[0]=rx_frm_p->nwk_h.sour.subnet;
                              EMN_SendMsg.data[1]=rx_frm_p->nwk_h.sour.dev;
							  EMN_SendMsg.dest_id= EMN_APL_TASK;
                              EMN_SendMsg.msg_id = EMN_APL_START_UPGRADE_CMD;
							  SYS_SendMsg(&EMN_SendMsg);
						  	}
							break;
						}
					 case EMN_NWK_GET_SAMPLE_CFM:
					 	    data_p++; 
			#if COUNT_LOSTPACKET  //add by helen for test lost packet
							flag_rev_ack=1;
							Rev_Endflag=0;
		    #endif
					 	    if(msg->data[0]==EMN_TXBUF_MACCOM)
		   	                {
				                NWK_WriteSampleData2(MAC_ComBuf);
		   	                }else if(msg->data[0]==EMN_TXBUF_NWKTX)	
		   	      	        {
                                NWK_WriteSampleData2(&NWK_TxBuf[0][0]);
		   	      	        }else
		   	      		    {
		   	      		        break;
		   	      		      }
					 	    
					 	    EMN_SendMsg.dest_id= EMN_APL_TASK;
                            EMN_SendMsg.msg_id =EMN_NWK_GET_SAMPLE_CFM;
                            EMN_SendMsg.data[0]=rx_frm_p->nwk_h.sour.subnet;
                            EMN_SendMsg.data[1]=rx_frm_p->nwk_h.sour.dev;
							if(((*data_p)&0xc0)==0xc0)
							{
							  EMN_SendMsg.data[2]=0x00;//all data
							  Rev_Endflag=1;//add by helen for test lost packet
							}else
								{
                                   EMN_SendMsg.data[2]=0x01;
 								}
                            SYS_SendMsg(&EMN_SendMsg); 
						    break;	
				   
					 case  EMN_NWK_GET_DATA_CFM:
					 	    break;
					 default: break;
				   }	
				   break;//end data_ind
			case EMN_TIMEOUT :
			/*	 if(msg->data[0] == SYS_TIMEOUT_RESEND)
				  {	

                   SYS_StopTimer(&EMN_MacTimer);
				   //resend the TX data
                    MAC_ResendTimeoutCt++;
                    if(MAC_ResendTimeoutCt>2)
                    {
   			           MAC_ResendTimeoutCt=0;
					   MAC_ReSendBuf.which = 0x00;
					   MAC_ReSendBuf.frm_num = 0x00;
					   DEBUG_DIS(printf("\nstop resend"));
                    }else if (MAC_ReSendBuf.which != 0x00)
                    	{                    	
        			    EMN_SendMsg.dest_id = EMN_PHY_TASK;
				        EMN_SendMsg.msg_id = EMN_PHY_TX_REQ;
				        EMN_SendMsg.data[0]= MAC_ReSendBuf.which;  
				        EMN_SendMsg.data[1]= MAC_ReSendBuf.frm_num;					
					    EMN_SendMsg.data[2] = MAC_ReSendBuf.net_addr;
				        EMN_SendMsg.data[3]= MAC_ReSendBuf.subnet_addr;
					   // DEBUG_DIS(printf("\nresend"));
				        SYS_SendMsg(&EMN_SendMsg); 
                    	}
				 	}*/
				break;
		    case EMN_MAC_DATA_CFM:
				
			//	NWK_MacDataCfmHal(msg);
				   break;
			default: break;
		   }//end mssage ID	
	   
			  break;  //EMN_NWK_CON_COORD:
  case EMN_NWK_CORD_DIS_SCAN: 	//itself scan it sRF range   
		  switch(msg->msg_id)
		  {
		  	case EMN_MAC_END_SCAN_CFM:
				  EMN_NWK_State = EMN_NWK_CORD_CON;
				  PHY_WriteRxAddr(EMN_NetAddr,EMN_SubnetAddr);
				  //send setup nwk 
				  EMN_SendMsg.dest_id= EMN_APL_TASK;
                  EMN_SendMsg.msg_id =EMN_NWK_END_SETUP_NWK;
                  SYS_SendMsg(&EMN_SendMsg); 

				   //scan its RF's range for new commer	 
#if 0				   
                  EMN_NWK_State = EMN_NWK_CORD_DIS_SCAN;  //before network ready	
                  ScanSubnetAddr=0x00;
                  ScanNetAddr=EMN_DeviceUID[3];
                  Scanning =1;
                  NWK_CordFindNewerReq(0x00,EMN_ChildNum+1,ScanNetAddr,ScanSubnetAddr);
#endif
		  	      break;
			case EMN_MAC_SCAN_CFM:				
			      if(msg->data[0] == EMN_MAC_SCAN_ONE_DEV)
				  {
					  NWK_WriteRegInfo();
				  }else if(msg->data[0] == EMN_MAC_SCAN_FINISH)
				    {
					  //because it is coordinator, only set value 
					  EMN_NetAddr=pBaseConfig->uid[3];
					  EMN_SubnetAddr=0x00;
					  EMN_DevAddr=0x00;
					  EMN_ChildNum += ScanDevNum;
					  Scanning=0;
					  if(EMN_ChildNum==0)
					  {
					    rfnwk_state= RF_NWK_DIS;
					  }else 
					  	{
						  rfnwk_state= RF_NWK_IDLE;

					  	}
					  EMN_SendMsg.dest_id = EMN_MAC_TASK;
			          EMN_SendMsg.msg_id = EMN_MAC_END_SCAN_REQ;
			          SYS_SendMsg(&EMN_SendMsg);			
					  
				    }
			  	  break;

			case EMN_MAC_DATA_IND:				
				   EMN_SendMsg.dest_id = EMN_MAC_TASK;
				   EMN_SendMsg.msg_id = EMN_MAC_DATA_RES;
				   EMN_SendMsg.data[0] = msg->data[0];
				   EMN_SendMsg.data[1] = msg->data[1];
				   SYS_SendMsg(&EMN_SendMsg);
				   break;
		   case  EMN_MAC_DIS_NETWORK_CFM:		   	       
				   ScanSubnetAddr=0x00;
				   ScanNetAddr=EMN_NetAddr;
				   Scanning =1;
				   _time_delay(1000);//waite for device init network protocl
				   NWK_CordFindNewerReq(0x00,EMN_ChildNum+1,ScanNetAddr,
				   	                    ScanSubnetAddr);
		   	      break;
			default : break;	
		  }//end mssage ID		 	
		  break;
		  
  case EMN_NWK_CORD_CON_SCAN:	  //let device in this network to scan their RF range  
				switch(msg->msg_id)
				{
				  case EMN_MAC_SCAN_CFM:
					  
					   if(msg->data[0] == EMN_MAC_SCAN_ONE_DEV)
					   {
						   NWK_WriteRegInfo()	;//write this info to storage
					   }else if(msg->data[0] == EMN_MAC_SCAN_FINISH)
						{
 					       NWK_WriteRegInfo();//write this info to storage

						}
						  break;
		  
				  case EMN_MAC_DATA_IND:// data from receiver buffer
						 rx_frm_p = (EMN_MAC_FRAME*)EMN_PHY_RxBuf.data[msg->data[0]];
						 data_p = &(rx_frm_p->nwk_h.data[0]);
						 switch(*data_p)
						 {
						   case EMN_NWK_ALLOC_ADDR_CFM: 				  
		  
								  break;  
						   case  EMN_NWK_FIND_NEWER_CFM:
								  NWK_WriteRegInfo();	
								  break;
						   default: break;
						 }					 
						 //release the receiver buffer in PHY layer
						 EMN_SendMsg.dest_id = EMN_MAC_TASK;
						 EMN_SendMsg.msg_id = EMN_MAC_DATA_RES;
						 EMN_SendMsg.data[0] = msg->data[0];
						 EMN_SendMsg.data[1] = msg->data[1];
						 SYS_SendMsg(&EMN_SendMsg);
						 break;//end data_ind
				  default : break;	  
				}//end mssage ID		  
				break; //end scan state
		  
    case EMN_NWK_CORD_DIS_CHECK:
		  switch(msg->msg_id)
		   {
		     case  EMN_NWK_CHECK_REG_REQ: 
			       break;  
		     default : break;	  
		  }		  
		
		  break;  // end state
	case EMN_NWK_CORD_DIS_INIT:
		  break;
       			  
	default : break;
 }
	
}


/* EOF */

