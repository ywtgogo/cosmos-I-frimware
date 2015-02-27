/**********************************************************************
* 
*	 Copyright (c) 2011 ;
* 	All Rights Reserved Convertergy
*	 FileName: dicor_network_apl.c
* 	Version : 
*	 Date     : 2010-11-16
* 	Owner   :  Peter Li
*
* Comments:
*************************************************************************/
#include <mqx.h>
#include <bsp.h>   
#include <Watchdog.h>
#include "rs485.h"

#include "dicor_network_apl.h"
#include "dicor_upload.h"
#ifdef DI_HARVESTER_V4
#include "unconnected_grid.h"
#endif 

#ifdef DI_HARVESTER_V3
#include "di-harvest.h"
#endif 
#include "lostlogging_public.h"

#define  WAITE_TIMEOUT_MAX  (120000L)
#define   TIMEOUT_NUM_DEVICE_UNAVAILBLE     (0)//2 
SYS_TIMER      EMN_AplTimer;
static UINT_8  GetDataSubNet,GetDataDev;
static UINT_8  GetParaSubNet,GetParaDev;


static UINT_8  GetTimes;
static UINT_8  SetTimes;
UINT_8  SetDiDiStatus;

ParaType SetParameterData;
ParaType NwkSetParaData;
UpdateType UpdataData;
UINT_8 Get_Data_Flag=0;
SET_PARA_TYPE  EMN_SetParaType;
UINT_8	DICOR_Status=DICOR_UPGRADE_INIT;

LWSEM_STRUCT GetSubDateSem;
UINT_8 GetSubDateFlag = 1;
LWSEM_STRUCT UpgradeStateSem;
extern INT_8 UpgradeState;


extern void dicor_get_lostlogfilename(DATE_STRUCT * date, char* name);
volatile UINT_32 LostLogTime = 0;
#define LOSTLOGSAVETIME	(500*60*5)


extern UINT_8 SetSubnet,SetDev,SetType;
extern void EMN_HAVST_Task(SYS_MESSAGE *msg);

//DIANDIDITABLE DiAnAndDiDiData;
extern const UINT_8  DianDidiTable[];
//extern DIDI_STATUS_TYPE DIDISTATUS_TABLE[];
//DIANDIDITABLE_PTR pDiAnDiDiTable=(DIANDIDITABLE_PTR)&DianDidiTable;
static UINT_8 CurDiDiIndex=0;

//for get information about subnet
static UINT_16  CurRegTabPos; 
static UINT_8  CurRegTabIndx;
UINT_32  APL_waitetime;
static UINT_8 EMN_APL_GetNextSubnet(void);

//extern const UINT_8 SubnetDepth[];
extern UINT_8* SubnetDepth;
extern LWSEM_STRUCT NetworkRfSem;

extern UINT_8 Didi_status ;
ParaType ParaData;
ParaType *SetParaData_p,SetParaData;
GetParaType GetParaData;

//UINT_8 Para[3]={4,2,3};

static void EMN_APL_SetPara(UINT_8 Subnet,UINT_8 dev,ParaType *data);
static UINT_32  EMN_APL_GetTimeoutValue(UINT_8 sub_addr);
extern void dicor_get_logtime(DATE_STRUCT * date);

UINT_8 ChkSaveLog(DATE_STRUCT* date)
{
	dicor_get_logtime(date);
	if ((date->HOUR>=6) && (date->HOUR<18))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
	

/*------------------------------------------------------------
 *
 *     the implementaion about network
 *
 *------------------------------------------------------------*/
extern void UpLoadbuffer_init(void);
void dicor_rf_nwk_task (UINT_32 initial_data)
{
	UpLoadbuffer_init(); //注释测试
	/*rs485_init();
	while (1)
	{
		rs485_read();
	}*/

    dicor_waite_rf(RF_REG_START,0);	
	/*if(_mutex_lock(&upload_buffer.mutex) != MQX_OK) 
	{
	  DEBUG_DIS(printf("\n system error, Mutex lock failed.\n"));
	}*/
    SYS_MainFunction();
}

/*------------------------------------------------------------
 *
 *    get the timeout value for this data request,
 *
 *------------------------------------------------------------*/

static UINT_32  EMN_APL_GetTimeoutValue(UINT_8 sub_addr)
{
  UINT_8   index;
  UINT_16  position;
  UINT_16  value;
  if(sub_addr==0x00)
  {
     return (1000);
  }
  position=0;  	
  for(index=0;index<SYS_SUBNET_NUM;index++)
  {
     if(SubnetDepth[position]==sub_addr)
     {
       if(SubnetDepth[position+2]>7)
       {
          value = SubnetDepth[position+1]*1000*2+7*1000*2;
       }else
       	{
           value = SubnetDepth[position+1]*1000*2+SubnetDepth[position+2]*1000*2;
       	}
	   break;
     }
     position +=3;
  }
  if(index == SYS_SUBNET_NUM)
  {
       value =30*1000;
	  // DEBUG_DIS(printf("\ntimeout table error")); by younger 2013.7.1
  }
  return(value);


}
/*------------------------------------------------------------
 *
 *    set paramete to DiAn's DiDi.
 *
 *------------------------------------------------------------*/
static void EMN_APL_SetPara(UINT_8 Subnet,UINT_8 dev,ParaType *paravalue)
{
    UINT_8 i;
	EMN_SendMsg.data[0]=Subnet;
	EMN_SendMsg.data[1]=dev;//if broadcast is 0xff,or is dev			
	EMN_SendMsg.dest_id= EMN_NWK_TASK;
	EMN_SendMsg.msg_id =EMN_NWK_SET_DATA_REQ;
	NwkSetParaData.brd= paravalue->brd;
	NwkSetParaData.flag= paravalue->flag;
	NwkSetParaData.index=paravalue->index;
	NwkSetParaData.len=paravalue->len;
	NwkSetParaData.dev=paravalue->dev;
	for(i=0;i<NwkSetParaData.len;i++)
	   { 
		 NwkSetParaData.data[i]=paravalue->data[i];
	   }
	
	SYS_SendMsg(&EMN_SendMsg); 
//	_watchdog_start(60000);   
	_watchdog_start(60*60*1000);
	DEBUG_DIS(printf("\nset:Subnet%d,dev%d",Subnet,paravalue->dev));
	
	//start ack timer
	EMN_AplTimer.value = EMN_APL_GetTimeoutValue(Subnet);
	EMN_AplTimer.msgID = SYS_TIMEOUT_SET_DATA;
	SYS_StartTimer(&EMN_AplTimer);		   


}
/*------------------------------------------------------------
 *
 *    get the next address from register table,
 *
 *------------------------------------------------------------*/
static UINT_8   EMN_APL_GetNextSubnet(void)
{
	UINT_8 i;
	UINT_8 flag;
	_lwsem_wait(&GetSubDateSem);
	flag = GetSubDateFlag; 
	_lwsem_post(&GetSubDateSem);  
	if (!flag)
	{
		//改过来看看，记得恢复
	    //_watchdog_start(30000);
	    _watchdog_stop();
		return 1;
	}
  
  if(Get_Data_Flag)
  {
	EMN_SendMsg.dest_id = EMN_APL_TASK;
	EMN_SendMsg.msg_id = SetType;
	EMN_SendMsg.data[0]=SetSubnet;
	EMN_SendMsg.data[1]=SetDev;
	SYS_SendMsg(&EMN_SendMsg);
	return(1);
  }
 if(workmode == WORKMODE_DIHAVST4)
 {
  if((HAVST_State == DI_HAVST_START)||(HAVST_State == DI_HAVST_COLLECT_DATA))
  {
   //  DEBUG_DIS(printf("\nHAVST_State=%d",HAVST_State));
    CurRegTabIndx++;
    if(CurRegTabIndx >RegTable[7])
    {
      CurRegTabPos=8;
  	CurRegTabIndx=1;
    }
    CurRegTabPos+=4;
   
    if(RegTable[CurRegTabPos]== EMN_SubnetAddr)//dicor's subnet
    {
       GetDataSubNet=EMN_SubnetAddr;
       GetDataDev=RegTable[CurRegTabPos+1];
  	 CurRegTabPos+=2;
   
    }else{  
        // the other subnet   	   
        
        for(i=0;i<RegTable[7];i++)  //search for entire register table
  	  {
  //	    DEBUG_DIS(printf("\ncur pos:index%d,pos%d",CurRegTabIndx,CurRegTabPos));
  	    if(RegTable[CurRegTabPos]!= GetDataSubNet)
  	 	{
            GetDataSubNet=RegTable[CurRegTabPos];
  		  GetDataDev=0x00;
  		  CurRegTabPos+=2;
  		  if(CurRegTabIndx >=RegTable[7])
            { 
             CurRegTabPos=8;
  	       CurRegTabIndx=0;
            }
  		  break;
          } 
          CurRegTabPos+=6;
  		CurRegTabIndx++;
  		if(CurRegTabIndx >RegTable[7])
          {
            CurRegTabPos=12;
  	      CurRegTabIndx=1;		  
          }
  		
  		if(RegTable[CurRegTabPos]== 0x00)//dicor's subnet
  		{
  		   GetDataSubNet=0x00;
  		   GetDataDev=RegTable[CurRegTabPos+1];
  		   CurRegTabPos+=2;
  		   break;
  		}
       }
  	 if(i== RegTable[7])
       {   
           CurRegTabPos-=4;
  	     CurRegTabIndx--;	 	
  	 }	   
       
    }
	//DEBUG_DIS(printf("\nGetDataSubNet=%d,GetDataDev=%d\n",GetDataSubNet,GetDataDev));
	  if((1==GetDataSubNet)&&(HAVST_SUB0_DEV_ADDR==GetDataDev))
     {
         if(HAVST_State == DI_HAVST_START)
         {
            HAVST_State = DI_HAVST_COLLECT_DATA;
	//		DEBUG_DIS(printf("\n dihavst begin collect data\n"));
         }else if(HAVST_State == DI_HAVST_COLLECT_DATA)
  	     {
  	    //   DEBUG_DIS(printf("\n dihavst begin collect data\n"));
            HAVST_State = DI_HAVST_PARSE_DATA;
  		    EMN_SendMsg.dest_id=EMN_HAVST_TASK; 	  
  		    EMN_SendMsg.msg_id = EMN_HAVST_PARSE_INFO_CMD;
  		    SYS_SendMsg(&EMN_SendMsg);
			return (2); 
  	     }
    }
  }else  	{
     return (2); //stop sampling because of di-harvesterV3.0
  }
}
 else
 	{
 	
	//	DEBUG_DIS(printf("\nsubnet%d,dev%d",GetDataSubNet,GetDataDev));
	  CurRegTabIndx++;
	  if(CurRegTabIndx >RegTable[7])
	  {
		CurRegTabPos=8;
		CurRegTabIndx=1;
	  }
	  CurRegTabPos+=4;
	 
	  if(RegTable[CurRegTabPos]== EMN_SubnetAddr)//dicor's subnet
	  {
		 GetDataSubNet=EMN_SubnetAddr;
		 GetDataDev=RegTable[CurRegTabPos+1];
		 CurRegTabPos+=2;
	 
	  }else{  
		  // the other subnet		   
		  
		  for(i=0;i<RegTable[7];i++)  //search for entire register table
		  {
	//		DEBUG_DIS(printf("\ncur pos:index%d,pos%d",CurRegTabIndx,CurRegTabPos));
			if(RegTable[CurRegTabPos]!= GetDataSubNet)
			{
			  GetDataSubNet=RegTable[CurRegTabPos];
			  GetDataDev=0x00;
			  CurRegTabPos+=2;
			  if(CurRegTabIndx >=RegTable[7])
			  { 
			   CurRegTabPos=8;
			   CurRegTabIndx=0;
			  }
			  break;
			} 
			CurRegTabPos+=6;
			CurRegTabIndx++;
			if(CurRegTabIndx >RegTable[7])
			{
			  CurRegTabPos=12;
			  CurRegTabIndx=1;		  
			}
			
			if(RegTable[CurRegTabPos]== 0x00)//dicor's subnet
			{
			   GetDataSubNet=0x00;
			   GetDataDev=RegTable[CurRegTabPos+1];
			   CurRegTabPos+=2;
			   break;
			}
		 }
		 if(i== RegTable[7])
		 {	 
			 CurRegTabPos-=4;
			 CurRegTabIndx--;		
		 }	   
	  }
 	}
  return(0);
}
/*--------------------------------------------------------
 *
 *     the implementaion about network
 *
 *-------------------------------------------------------*/ 
void EMN_APL_Task(SYS_MESSAGE *msg)
{
  int i;
  DATE_STRUCT date;
 switch(msg->msg_id)
 {
   case EMN_TIMEOUT:
	   
   	     switch(msg->data[0])
   	     	{  
   	     	  case SYS_TIMEOUT_SET_DATA:
   	      	  { 
#if 1		
              if(workmode == WORKMODE_DIHAVST4)
              {
			  	if(EMN_SetParaType != HAVST_SEND_CMD)
			  	{
    			  	Get_Data_Flag=0;				
    				EMN_SendMsg.dest_id= EMN_APL_TASK; 	  
    				EMN_SendMsg.msg_id = EMN_NWK_START_GET_DATA;
    				SYS_SendMsg(&EMN_SendMsg);  			
    			//	SendAllDiDiCmd(0x01,0xff,0x01,0x01);
    	          //  _watchdog_start(60000);	
    	          _watchdog_start(60*60*1000);
			  	}else if (EMN_SetParaType == HAVST_SEND_CMD)
				{
					//_watchdog_start(60000); 
					_watchdog_start(60*60*1000);
					EMN_SendMsg.dest_id= EMN_HAVST_TASK;	   
					EMN_SendMsg.msg_id =EMN_HAVST_CONTROL_CMD_CFM;
					SYS_SendMsg(&EMN_SendMsg);

				}
              }
			  else
			  	{		  	
					Get_Data_Flag=0;
					EMN_SendMsg.dest_id= EMN_APL_TASK;	  
					EMN_SendMsg.msg_id = EMN_NWK_START_GET_DATA;
					SYS_SendMsg(&EMN_SendMsg);				   
					//_watchdog_start(60000); 
					_watchdog_start(60*60*1000);
			  	}
#endif				
			     break;			 
   	       	 }
		   case SYS_TIMEOUT_GET_PARA_DATA:
		   	{
			  if((EMN_SetParaType != HAVST_SEND_CMD)||(workmode != WORKMODE_DIHAVST4))
		  	  {
				Get_Data_Flag=0;
				EMN_SendMsg.dest_id= EMN_APL_TASK; 	  
				EMN_SendMsg.msg_id = EMN_NWK_START_GET_DATA;
				SYS_SendMsg(&EMN_SendMsg);  
				// _watchdog_start(60000);	
				_watchdog_start(60*60*1000);
			  }
				  break;
		   	 }
		   case SYS_TIMEOUT_GET_UPDATE_CMD:
		   	{
                 _lwsem_wait(&UpgradeStateSem);
		       UpgradeState =-1;
			   _lwsem_post(&UpgradeStateSem); 
				 break;
		   	}
		   case SYS_TIMEOUT_SET_CFM_STATUS:
		  	{ 			
			//	SendAllDiDiCmd(0x01,0xff,0x01,0x01);
			if(workmode == WORKMODE_DIHAVST4)
			{
			 if(EMN_SetParaType != HAVST_SEND_CMD)
			  {
			    EMN_SendMsg.dest_id= EMN_APL_TASK; 	  
				EMN_SendMsg.msg_id = EMN_NWK_START_GET_DATA;
				SYS_SendMsg(&EMN_SendMsg);    
			//	_watchdog_start(60000);	
				_watchdog_start(60*60*1000);
			  }
			 else if (EMN_SetParaType == HAVST_SEND_CMD)
				{
				//	_watchdog_start(60000); 
					_watchdog_start(60*60*1000);
					EMN_SendMsg.dest_id= EMN_HAVST_TASK;	   
					EMN_SendMsg.msg_id =EMN_HAVST_CONTROL_CMD_CFM;
					SYS_SendMsg(&EMN_SendMsg);

				}
			  }
			 else
			 	{
		 	 
					EMN_SendMsg.dest_id= EMN_APL_TASK; 	 
					EMN_SendMsg.msg_id = EMN_NWK_START_GET_DATA;
					SYS_SendMsg(&EMN_SendMsg);	 
				//	_watchdog_start(60000); 
					_watchdog_start(60*60*1000);
			 	}
			 break;
		  	}	
	      case SYS_TIMEOUT_GET_PARA_CFM_DATA:
		   	{				
				EMN_SendMsg.dest_id= EMN_APL_TASK; 	  
				EMN_SendMsg.msg_id = EMN_NWK_START_GET_DATA;
				SYS_SendMsg(&EMN_SendMsg);  
			//	_watchdog_start(60000);	
				_watchdog_start(60*60*1000);
				break;
		   	}
   	      case SYS_TIMEOUT_GET_CFM_DATA:
   	      	{ //DEBUG_DIS(printf("\n 2msg->data[0]=%d",msg->data[0]));
   	     //	DEBUG_DIS(printf("\n Set_Flag=%d Get_Flag=%d\n",Set_Flag,Get_Flag)); 
			  {		   			   
			   	   if(EMN_APL_GetNextSubnet()) break;								 
			   	   EMN_SendMsg.data[0]=GetDataSubNet;
			       EMN_SendMsg.data[1]=GetDataDev;
				   EMN_SendMsg.dest_id= EMN_NWK_TASK; 	  
				   EMN_SendMsg.msg_id = EMN_NWK_GET_SAMPLE_REQ;
				  DEBUG_DIS(printf("\nget:sub%d,dev%d",GetDataSubNet,GetDataDev));
				  //
				  if (ChkSaveLog(&date))//[6,18)丢包保存
				  {
				  	sprintf(pLostLog->logbuf, "g%02X%02X\r\n",GetDataSubNet,GetDataDev);
				  //	fwrite(LostLog.logbuf, 7, 1, LostLog.fd_ptr);
				  PrintLost(pLostLog->logbuf);
					if (LostLogTime >= LOSTLOGSAVETIME)
					{
						LostLogTime = 0;
						//保存时间
						sprintf(pLostLog->logbuf, "System Time: %02d:%02d:%02d\r\n",date.HOUR,date.MINUTE,date.SECOND);
				  		PrintLost(pLostLog->logbuf);
					}
				  }
				  SYS_SendMsg(&EMN_SendMsg);
				 // _watchdog_start(60000);	
				 _watchdog_start(60*60*1000);
				  GetTimes=0;
					  //start ack timer
				  EMN_AplTimer.value = EMN_APL_GetTimeoutValue(GetDataSubNet);
				 
			     EMN_AplTimer.msgID = SYS_TIMEOUT_GET_DATA;
			     SYS_StartTimer(&EMN_AplTimer); 	
				  
			   	}
			  
		     }	
			  break;	     
   	      case SYS_TIMEOUT_GET_DATA:
          {	  	
            if(GetTimes>=TIMEOUT_NUM_DEVICE_UNAVAILBLE)
            {
			  // DEBUG_DIS(printf("\n SYS_TIMEOUT_GET_DATA"));  
				   if(EMN_APL_GetNextSubnet()) break;
				  //  GetDevNum--;
					EMN_SendMsg.data[0]=GetDataSubNet;
					EMN_SendMsg.data[1]=GetDataDev;
					EMN_SendMsg.dest_id= EMN_NWK_TASK; 	  
					EMN_SendMsg.msg_id =EMN_NWK_GET_SAMPLE_REQ;
					DEBUG_DIS(printf("\nget:sub%d,dev%d",GetDataSubNet,GetDataDev));
					if (ChkSaveLog(&date))
					{
						sprintf(pLostLog->logbuf, "g%02X%02X\r\n",GetDataSubNet,GetDataDev);
				  	//	fwrite(LostLog.logbuf, 7, 1, LostLog.fd_ptr);
				  		PrintLost(pLostLog->logbuf);
						if (LostLogTime >= LOSTLOGSAVETIME)
						{
							LostLogTime = 0;
							sprintf(pLostLog->logbuf, "System Time: %02d:%02d:%02d\r\n",date.HOUR,date.MINUTE,date.SECOND);
				  			PrintLost(pLostLog->logbuf);
				  		/*	fwrite(LostLog.logbuf, 23, 1, LostLog.fd_ptr);
							fclose(LostLog.fd_ptr);
							 dicor_get_lostlogfilename(&date, LostLog.filename);
							 LostLog.fd_ptr = fopen(LostLog.filename, "a+");
							if (LostLog.fd_ptr == NULL )
							{
								printf("Error open file\r\n");
							}*/
						}
					}
					SYS_SendMsg(&EMN_SendMsg);
				  //_watchdog_start(60000);	
				  _watchdog_start(60*60*1000);
				  GetTimes=0;
				  //start ack timer
			  EMN_AplTimer.value = EMN_APL_GetTimeoutValue(GetDataSubNet);		  
			  EMN_AplTimer.msgID = SYS_TIMEOUT_GET_DATA;
			  SYS_StartTimer(&EMN_AplTimer); 				  		   
			 
	       }else
            	{   
		           EMN_SendMsg.dest_id= EMN_NWK_TASK;
		           EMN_SendMsg.msg_id =EMN_NWK_GET_SAMPLE_REQ;
		           EMN_SendMsg.data[0]=GetDataSubNet;
		           EMN_SendMsg.data[1]=GetDataDev;
				   SYS_SendMsg(&EMN_SendMsg);
				   //_watchdog_start(60000);
				   _watchdog_start(60*60*1000);
				   GetTimes++;
				    DEBUG_DIS(printf("\nget %dtimes:sub%d,dev%d",GetTimes+1,GetDataSubNet,GetDataDev));
				   
				   //start ack timer
				   EMN_AplTimer.value = EMN_APL_GetTimeoutValue(GetDataSubNet);			  
				   EMN_AplTimer.msgID = SYS_TIMEOUT_GET_DATA;
				   SYS_StartTimer(&EMN_AplTimer);  				   
            	}			         
   	      }break;
   	   }
		 break;
	case EMN_NWK_START_SET_DATA:
	     	{
		  //SetParaData_p=(ParaType*)(&msg->data[2]);	
		  SetParaData.brd=SetParameterData.brd;
		  SetParaData.index=SetParameterData.index;
		  SetParaData.len=SetParameterData.len;
		  SetParaData.dev=SetParameterData.dev;
		  for(i=0;i<SetParaData.len;i++)
		  	{
		  	  SetParaData.data[i]=SetParameterData.data[i];
		  	}
		  
		  SetTimes=0;	  
		  EMN_APL_SetPara(msg->data[0],msg->data[1],&SetParaData);		    		   
		  }
	    break;
	case EMN_NWK_START_GET_PARA_DATA:
		{
		     EMN_SendMsg.dest_id= EMN_NWK_TASK;
		     EMN_SendMsg.msg_id =EMN_NWK_GET_PARA_REQ;
			 EMN_SendMsg.data[0]=msg->data[0];
		     EMN_SendMsg.data[1]=msg->data[1];
			if(workmode == WORKMODE_DIHAVST4)
			{
		     GetParaSubNet=msg->data[0];
		     GetParaDev=msg->data[1];
			}
		     SYS_SendMsg(&EMN_SendMsg); 
			// _watchdog_start(60000);	
			_watchdog_start(60*60*1000);
 		     DEBUG_DIS(printf("\nget para:sub%d,dev%d",GetParaSubNet,GetParaDev));
		    EMN_AplTimer.value = EMN_APL_GetTimeoutValue(GetDataSubNet);
		   EMN_AplTimer.msgID = SYS_TIMEOUT_GET_PARA_DATA;
		   SYS_StartTimer(&EMN_AplTimer);
		}
		 break;
	case EMN_NWK_SEND_DATA_CHECK_CFM://check ok
	   {
	   }
	   break;
	case EMN_APL_START_UPGRADE_CFM:
		  switch(msg->data[2])
		  {		  
			case EMN_UPGRADE_CMD:
				SYS_StopTimer(&EMN_AplTimer); 
				if(msg->data[3])//error
				  {				  
					_lwsem_wait(&UpgradeStateSem);
			       UpgradeState = msg->data[3];
				   _lwsem_post(&UpgradeStateSem);
				     DEBUG_DIS(printf("\n rev return code=%d",UpgradeState));
				 }
				else
					{
					  _lwsem_wait(&UpgradeStateSem);
				       UpgradeState = 1;
					   _lwsem_post(&UpgradeStateSem);
					   DEBUG_DIS(printf("\n rev upgrade OK command"));
					}
			    break;
		}
	case EMN_APL_START_UPGRADE_CMD:
		{
		  if(msg->data[2]==EMN_UPGRADE_CMD)
		  	{
		  	   SYS_StopTimer(&EMN_AplTimer);
				EMN_SendMsg.dest_id= EMN_NWK_TASK;
				EMN_SendMsg.msg_id =EMN_NWK_UPGRADE_CMD_REQ;
				EMN_SendMsg.data[0]= msg->data[0];
				EMN_SendMsg.data[1]= msg->data[1];
				SYS_SendMsg(&EMN_SendMsg); 
				  
				if(msg->data[3] == EMN_UPGRADE_NOROUTER_CMD)
					{
						EMN_AplTimer.value = 2000;//EMN_APL_GetTimeoutValue(msg->data[0]);
						EMN_AplTimer.msgID = SYS_TIMEOUT_GET_UPDATE_CMD;
						SYS_StartTimer(&EMN_AplTimer);						
					}
					
		  	}		  
		}
		break;
    case EMN_NWK_START_GET_DATA:	// from EMN os
    	{	  
			   if(EMN_APL_GetNextSubnet()) break;				
				//DEBUG_DIS(printf("\n Begin to get data GetDevNum=%d\n",GetDevNum));
				EMN_SendMsg.dest_id= EMN_NWK_TASK;
				EMN_SendMsg.msg_id = EMN_NWK_GET_SAMPLE_REQ;//EMN_NWK_GET_PARA_REQ;//
				EMN_SendMsg.data[0]=GetDataSubNet;
				EMN_SendMsg.data[1]=GetDataDev;
				SYS_SendMsg(&EMN_SendMsg); 
			//	_watchdog_start(60000);	 
				_watchdog_start(60*60*1000);
				DEBUG_DIS(printf("\nget:sub%d,dev%d",GetDataSubNet,GetDataDev));
				if (ChkSaveLog(&date))
				{
					sprintf(pLostLog->logbuf, "g%02X%02X\r\n",GetDataSubNet,GetDataDev);
					//fwrite(LostLog.logbuf, 7, 1, LostLog.fd_ptr);
					PrintLost(pLostLog->logbuf);
					if (LostLogTime >= LOSTLOGSAVETIME)
					{
						LostLogTime = 0;
						sprintf(pLostLog->logbuf, "System Time: %02d:%02d:%02d\r\n",date.HOUR,date.MINUTE,date.SECOND);
				  		PrintLost(pLostLog->logbuf);
				  	/*	fwrite(LostLog.logbuf, 23, 1, LostLog.fd_ptr);
						fclose(LostLog.fd_ptr);
						 dicor_get_lostlogfilename(&date, LostLog.filename);
						 LostLog.fd_ptr = fopen(LostLog.filename, "a+");
						if (LostLog.fd_ptr == NULL )
						{
							printf("Error open file\r\n");
						}*/
					}
				}
		
			 GetTimes=0;
	
		     //start ack timer
		     EMN_AplTimer.value = EMN_APL_GetTimeoutValue(GetDataSubNet);			 
		     EMN_AplTimer.msgID = SYS_TIMEOUT_GET_DATA;
		     SYS_StartTimer(&EMN_AplTimer); 		   
    	} 
		   break;
	case EMN_NWK_SET_DATA_CFM:
		{
		if(workmode == WORKMODE_DIHAVST4)
		 {
	       if(EMN_SetParaType != HAVST_SEND_CMD)
		  	 {			
	       	}else if (EMN_SetParaType == HAVST_SEND_CMD)
			{

				SetTimes=0;
				SetParameterData.flag=1;
				Get_Data_Flag=0;
				// Get_Flag=0;
			//	_watchdog_start(60000); 
				_watchdog_start(60*60*1000);
				DEBUG_DIS(printf("\nrev SET:sub%d,dev%d",msg->data[0],msg->data[1]));
				EMN_SendMsg.dest_id= EMN_HAVST_TASK;	   
				EMN_SendMsg.msg_id =EMN_HAVST_CONTROL_CMD_CFM;
				SYS_SendMsg(&EMN_SendMsg);
			}
		 }else
		 	{
			SetTimes=0;
			SetParameterData.flag=1;
			Get_Data_Flag=0;
			// Get_Flag=0;
		//	_watchdog_start(60000);	
			_watchdog_start(60*60*1000);
			DEBUG_DIS(printf("\nrev SET:sub%d,dev%d",msg->data[0],msg->data[1]));
#if 1		   
			//get the next device data
			SYS_StopTimer(&EMN_AplTimer); 
			//start waite timer
			EMN_AplTimer.value = 300;//300;
			EMN_AplTimer.msgID = SYS_TIMEOUT_SET_CFM_STATUS;
			SYS_StartTimer(&EMN_AplTimer);
#endif 
		 	}
 
		}
	   break;
    case EMN_NWK_GET_PARA_CFM:
		{
		if((EMN_SetParaType != HAVST_SEND_CMD)||(workmode != WORKMODE_DIHAVST4))
	    { 
			Get_Data_Flag=0;
		}
			DEBUG_DIS(printf("\n EMN_NWK_GET_PARA_CFM"));
			//_watchdog_start(60000);
			_watchdog_start(60*60*1000);	
			SYS_StopTimer(&EMN_AplTimer); 
			//start waite timer
			EMN_AplTimer.value = 300;
			EMN_AplTimer.msgID = SYS_TIMEOUT_GET_PARA_CFM_DATA;
			SYS_StartTimer(&EMN_AplTimer);					   
    	}
		break;
    case EMN_NWK_GET_SAMPLE_CFM:
		{
		   GetTimes=0;
		   rfnwk_state = RF_NWK_IDLE;
		//   if((msg->data[0]!=GetDataSubNet)||(msg->data[1]!=GetDataDev))
		//   {
        //       DEBUG_DIS(printf("\nrev not from "));
		//   }
		   DEBUG_DIS(printf("\nrev:sub%d,dev%d",msg->data[0],msg->data[1]));
#if 1		   
           //get the next device data
		   if(msg->data[2])
		   {
		      EMN_SendMsg.dest_id= EMN_NWK_TASK;	   
		      EMN_SendMsg.msg_id =EMN_NWK_GET_SAMPLE_REQ;
		      //EMN_APL_GetNextSubnet();
		      EMN_SendMsg.data[0]=GetDataSubNet;
		      EMN_SendMsg.data[1]=GetDataDev;
		      SYS_SendMsg(&EMN_SendMsg);
			 // _watchdog_start(60000);	
			_watchdog_start(60*60*1000);
              _time_delay(300);
		      SYS_StopTimer(&EMN_AplTimer); 
		      //start waite timer
		      DEBUG_DIS(printf("\nContinue get:sub%d,dev%d",GetDataSubNet,GetDataDev));
			  if (ChkSaveLog(&date))
			  {
				  sprintf(pLostLog->logbuf, "c%02X%02X\r\n",GetDataSubNet,GetDataDev);
				  PrintLost(pLostLog->logbuf);
				  //fwrite(LostLog.logbuf, 7, 1, LostLog.fd_ptr);
			  }
		      EMN_AplTimer.value = EMN_APL_GetTimeoutValue(GetDataSubNet);
		      EMN_AplTimer.msgID = SYS_TIMEOUT_GET_DATA;
		      SYS_StartTimer(&EMN_AplTimer);		 			  
	
		   } else{
		      
			   SYS_StopTimer(&EMN_AplTimer); 
			   //start waite timer
			   EMN_AplTimer.value = 300;
			   EMN_AplTimer.msgID = SYS_TIMEOUT_GET_CFM_DATA;
			   SYS_StartTimer(&EMN_AplTimer);					   
           }

#endif 
    	}
		   break;
	case    EMN_NWK_END_SETUP_NWK:
//            upload_buffer.state=CAN_READ;   //for upload register table
			if(EMN_ChildNum==0)
			{
			  rfnwk_state = RF_NWK_DIS;
			}else
				{
                  rfnwk_state = RF_NWK_IDLE;
				}
//			dicor_rf_signal(RF_REG_END);			
		    break;
   default: break;		  
  }

}
void EMN_APL_SampleFromTop(void)
{
	GetDataSubNet=0x00;
	GetDataDev=0x01;
	CurRegTabPos=8;
	CurRegTabIndx=0;
}
void EMN_APL_TaskInit(void)
{

  //init timer for this task using
  EMN_AplTimer.id = SYS_TIMER3;
  EMN_AplTimer.fastfun = NULL;
  EMN_AplTimer.taskID = EMN_APL_TASK;
  EMN_AplTimer.value =0;
  EMN_AplTimer.msgID = 0;

  //ack timeout
  GetDataSubNet=0x00;
  GetDataDev=0x01;

  GetTimes=0; 
  CurRegTabPos=8;
  CurRegTabIndx=0;
  APL_waitetime=0;
  Get_Data_Flag=0;

 // SetParaData_p=&SetParaData;
  #if DICOR_STATIC_REG_TAB		
  EMN_DevAddr = 0x00;
  EMN_SubnetAddr = 0x00; 
  EMN_NetAddr =pBaseConfig->net_addr;	
  EMN_NetDepth=0;
  EMN_ChildNum=EMN_DEV_NUM;
  #endif
  
  //
  _lwsem_create(&GetSubDateSem, 1);
  _lwsem_create(&UpgradeStateSem, 1);

  //用于保存丢包情况
/*  dicor_get_logtime(&date);
  dicor_get_lostlogfilename(&date, LostLog.filename);
  LostLog.fd_ptr = fopen(LostLog.filename, "a+");
	if (LostLog.fd_ptr == NULL )
	{
		printf("Error open file\r\n");
	}*/
}

void rf_nwk_exit(void)
{
	//释放CPU资源
	_time_delay(100);
	//等待文件保存结束
	_lwsem_wait(&pLostLog->writesem);
	_lwsem_post(&pLostLog->writesem);
}


/* EOF */

