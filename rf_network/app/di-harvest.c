/**********************************************************************
* 
*	 Copyright (c) 2011 ;
* 	 All Rights Reserved Convertergy
*	 FileName: di-harvester.c
* 	 Version : 
*	 Date     : 2011-10-18
* 	 Owner   :  Peter Li
*
* Comments:
*************************************************************************/
#include "network_frame.h"
#ifdef DI_HARVESTER_V3
#include <mqx.h>
#include <bsp.h>
#include "di-harvest.h"
#include <eeprom.h>

UINT_8 HAVST_State;
extern SET_PARA_TYPE  EMN_SetParaType;
static SYS_TIMER HavstTimer;
static UINT_8    CmdCurDianIndex=0;
static UINT_8    HavstHasDataToHandle;

extern DIANDIDITABLE_PTR pDiAnDiDiTable;
UINT_8 REG_DIDI_NUM;
static DIAN_INFO  DianInfo[REG_DIAN_NUM];
static DIDI_INFO  DidiInfo[REG_DIDI_NUM_EXCEPT];
extern void    EMN_APL_SampleFromTop(void);
extern void EMN_APL_SetParameter(UINT_8 subnet,UINT_8 dev,ParaType *para);


static void    NewRoundCollectData(void);
static void    ResetExpectState(UINT_8 dian_index);
static void    CalcuBusVol(UINT_8 dian_index,UINT_16 offset, UINT_16 didinum);
static void    SendDiAnCmd(UINT_8 subnet_addr,UINT_8 dev_addr,UINT_8 cmd, UINT_8 send_mode);
static void    SendAllDiDiCmd(UINT_8 subnet_addr,UINT_8 dev_addr,UINT_8 cmd,UINT_8 send_mode);
static void    GetDianIndexByDidi(UINT_8 subnet,UINT_8 dev,UINT_16 *dian_index,UINT_16 *didi_index,UINT_16 *didi_offset);
static void    GetDianIndexByDian(UINT_8 subnet,UINT_8 dev,UINT_16 *dian_index);
static UINT_16 GetSysDidiCount(void);
#define GetSysDianCount()     (((UINT_16)pDiAnDiDiTable->diancount[0])<<8)|pDiAnDiDiTable->diancount[1]



/*----------------------------------------------------------
 *
 * Function Name  : HAVST_CollectInfo(void)
 * Returned Value : void
 * Comments		 :
 *
 typedef struct{
 UINT_8   frm_type;	
 UINT_8   len;
 UINT_8  subnet; 
 UINT_8  dev;
 UINT_16  in_cur;
 UINT_16  in_Vol;
 UINT_16  out_cur;
 UINT_16  out_vol;
 UINT_16  out_power;
 UINT_16 d;   //bit11:   Normal   bit12: lowpower
}EMN_SMC_DATA_ST;
 *-----------------------------------------------------------*/
#define DEVICE_TYPE_DIDI   (0x10)
#define DEVICE_TYPE_DIAN   (0x20)
void HAVST_CollectPlantInfo(UINT_8 *data,UINT_16 data_num)
{
   UINT_16 i,didi_index,dian_index,didi_offset;
   if((HAVST_State==DI_HAVST_COLLECT_DATA)&& data_num)
   {
    HavstHasDataToHandle=1;
     for(i=0;i<data_num;)
     {
        //DiDi device 
        if(data[i]==DEVICE_TYPE_DIDI)
        {
          GetDianIndexByDidi(data[i+2],data[i+3],&dian_index,&didi_index,&didi_offset);
          if((!DianInfo[dian_index].init_end)&&((DianInfo[dian_index].sys_cur_st&0x0f)==DIDI_ALL_SWITCHING_ST))
    	  { 
        	  if((data[i+15]>>3)&0x01)  //state  1:normal  0:force direct
        	  {
        	     DianInfo[dian_index].didi_sample_st |= ((UINT_32)0x1)<<didi_index;
        	  }else
        	  	{
                  DianInfo[dian_index].didi_sample_st &= ~(((UINT_32)0x1)<<didi_index);
        	  	}
     	  }
          if(DianInfo[dian_index].init_end)
          {
             // GetDianIndexByDidi(data[i+2],data[i+3],&dian_index,&didi_index,&didi_offset);
              //for check DiDi state
        	  if((data[i+15]>>3)&0x01)  //state
        	  {
        	     DianInfo[dian_index].didi_sample_st |= ((UINT_32)0x1)<<didi_index;////?????
        	  }else
        	  	{
                  DianInfo[dian_index].didi_sample_st &= ~(((UINT_32)0x1)<<didi_index);
        	  	}
              //collect low power state
        	  if((data[i+15]>>4)&0x01)  //lowpower
        	  {
        	     DianInfo[dian_index].low_power_count++;
        	  }
               //for adjust VBUS
              if(((DianInfo[dian_index].sys_cur_st&0xf0)== DIDN_NO_MPPT_ST))
              {
                 DidiInfo[didi_index].vol_in= ((UINT_16)data[i+7]<<8)+ data[i+8];
    			 DidiInfo[didi_index].vol_out= ((UINT_16)data[i+11]<<8)+ data[i+12];
              }
          } 
		 // DiAn device  
        }else if(data[i]==DEVICE_TYPE_DIAN)
          {
             GetDianIndexByDian(data[i+2],data[i+3],&dian_index);
			 if(data[i+10]&0x01)	 //14:state bit
			 {
				DianInfo[dian_index].dian_sample_st= DIDN_NO_MPPT_ST;
			 }else{ 		 
				DianInfo[dian_index].dian_sample_st = DIDN_MPPT_ST; 				 
			 }
			 //when dian changes itself state from mppt to no mppt, use this value for DiAn's VBUS
//				 if((DianInfo[dian_index].sys_cur_st&0xf0)==DIDN_MPPT_ST)
			 {
              // DianInfo[dian_index].dian_bus_voltage=((UINT_16)data[i+4]<<8)+ data[i+5];
			 }  
						 
          }
		i+=data[i+1]; 
		DEBUG_DIS(printf("\n DianInfo[%x].didi_sample_st=%x\n",dian_index,DianInfo[dian_index].didi_sample_st));
     }
   }
}
 /*----------------------------------------------------------
  *
  * Function Name  : GetDianIndexByDidi
  * Returned Value : void
  * Comments	  :
  *
  *-----------------------------------------------------------*/
 static void GetDianIndexByDidi(UINT_8 subnet,UINT_8 dev,UINT_16 *dian_index,
 	                                   UINT_16 *didi_index,UINT_16 *didi_offset)
 {
     UINT_16 *didi_table_p,didi=0,i,didisum,diansum;
	 didi=((didi+subnet)<<8)+dev;
     didi_table_p=(UINT_16 *)(&(pDiAnDiDiTable->didiaddr[0]));
	 didisum=GetSysDidiCount();
	   
	 for(i=0;i<didisum;i++,didi_table_p++)
	 {
	   if(didi==*didi_table_p) 
	   	{ 
	   	  *didi_index = i;
	   	  break;
	   	}  
	   	
	 }

	 //for Dian index
	 diansum =GetSysDianCount();
	 for(i=0;i<diansum;i++)
	 {
       if((2*(*didi_index) >=(pDiAnDiDiTable->indextab[i].offset[0]<<8+pDiAnDiDiTable->indextab[i].offset[1]))&&
           (2*(*didi_index) <(pDiAnDiDiTable->indextab[i+1].offset[0]<<8+pDiAnDiDiTable->indextab[i+1].offset[1])))	   	
       	{
           	   	 *dian_index = i;
				 *didi_offset=2*(*didi_index)-(pDiAnDiDiTable->indextab[i].offset[0]<<8+pDiAnDiDiTable->indextab[i].offset[1]);
	   	         break;
       	}
	 }

     
 }
 
 /*----------------------------------------------------------
  *
  * Function Name  : GetDianIndexByDian
  * Returned Value : void
  * Comments	  :
  *
  *-----------------------------------------------------------*/
 static void  GetDianIndexByDian(UINT_8 subnet,UINT_8 dev,
 	                                     UINT_16 *dian_index)
 {
    UINT_8 i,diansum;
    diansum=GetSysDianCount();
   	for(i=0;i<diansum;i++)
   	{
   	  if((pDiAnDiDiTable->indextab[i].dianaddr[0]==subnet)&&
	  	 (pDiAnDiDiTable->indextab[i].dianaddr[1]==dev))
   	  { 	     
		*dian_index=i;
		break;
   	  }
   	}
	if(i==diansum)
	{
	   *dian_index=0xff;
	}   

 }
 /*----------------------------------------------------------
  *
  * Function Name  : GetSysDidiCount
  * Returned Value : void
  * Comments	  :
  *
  *-----------------------------------------------------------*/

 static UINT_16 GetSysDidiCount(void)
 {
    UINT_8 i,diansum,didisum;
    diansum=GetSysDianCount();
	didisum=0;
   	for(i=0;i<diansum;i++)
   	{
      didisum= didisum+(pDiAnDiDiTable->indextab[i].count[0]<<8)+pDiAnDiDiTable->indextab[i].count[1];
   	}
	return didisum;
}

/*----------------------------------------------------------
 *
 * Function Name  : HAVST_ParseInfo(void)
 * Returned Value  void
 * Comments		 :
 *
 *-----------------------------------------------------------*/
 static UINT_8  Check_nums(UINT_32  x )
{
  UINT_8  countx = 0;
  while( x )
  {
    countx ++;
    x = x & ( x-1 );
  }
  return countx;
} 
static void HAVST_ParsePlantInfo(void)
{
  UINT_8  dian_index;
  UINT_16 dian_count;
  DIANDIDIINDEXTAB   *dian_table;
  if(HavstHasDataToHandle)
  {
  dian_count=GetSysDianCount();
  dian_table=&(pDiAnDiDiTable->indextab[0]);
  //didi_count=(dian_table->count[0]<<8)+dian_table->count[1];

	  for(dian_index=0; dian_index<dian_count;dian_index++,dian_table++)
	  {
	    if(!DianInfo[dian_index].init_end)
        {
		  if((DianInfo[dian_index].sys_cur_st&0xf0)==DIDN_SWITCHING_ST)
		  {
		     if ((DianInfo[dian_index].dian_sample_st==(DianInfo[dian_index].sys_expect_st&0xf0))&&
			      (DianInfo[dian_index].dian_sample_st==DIDN_NO_MPPT_ST))
		     {
				DianInfo[dian_index].sys_cur_st &= 0x0f;
				DianInfo[dian_index].sys_cur_st |= DIDN_NO_MPPT_ST;
		     }else
		     	{
                    DianInfo[dian_index].command |=DIAN_EXIT_MPPT_CMD;
		     	}
		  }	 
		  if((DianInfo[dian_index].sys_cur_st&0x0f)==DIDI_ALL_SWITCHING_ST)
		  {
            //this condition is too strict.  
			if(Check_nums(DianInfo[dian_index].didi_sample_st)>=(((dian_table->count[0]<<8)+dian_table->count[1])-3))
			{
			  DianInfo[dian_index].sys_cur_st &= 0xf0;
			  DianInfo[dian_index].sys_cur_st |= (DianInfo[dian_index].sys_expect_st&0x0f);
			}else
			  {
				DianInfo[dian_index].command |=DIDI_ALL_ENTER_NOMAL_CMD;
			  }
		  }	
		 if(((DianInfo[dian_index].sys_cur_st&0xf0)==DIDN_NO_MPPT_ST)&&
			((DianInfo[dian_index].sys_cur_st&0x0f)== DIDI_ALL_NORMAL_ST))
		 {
            DianInfo[dian_index].init_end=1;  // finish init state
             DEBUG_DIS(printf("\n dihavst system init end\n"));
            
		 }else if( DianInfo[dian_index].sys_cur_st ==(DIDN_INIT_ST|DIDI_ALL_INIT_ST))
	     {
	        DianInfo[dian_index].sys_cur_st=DIDN_SWITCHING_ST|DIDI_ALL_SWITCHING_ST;
	        DianInfo[dian_index].command=DIAN_EXIT_MPPT_CMD|DIDI_ALL_ENTER_NOMAL_CMD;
		    ResetExpectState(dian_index);
	     }

	  }  else{
//      for(dian_index=0; dian_index<dian_count;dian_index++,dian_table++)
        if(((DianInfo[dian_index].sys_cur_st&0xf0)==DIDN_SWITCHING_ST)||
			((DianInfo[dian_index].sys_cur_st&0x0f)==DIDI_ALL_SWITCHING_ST))
 	    {	
 	        DianInfo[dian_index].command =0x00;
 	         //check DiAn
    		if((DianInfo[dian_index].sys_cur_st&0xf0)==DIDN_SWITCHING_ST)
    		{
    		    
				if(DianInfo[dian_index].dian_sample_st==(DianInfo[dian_index].sys_expect_st&0xf0))
				{
                  DianInfo[dian_index].sys_cur_st &= 0x0f;
				  DianInfo[dian_index].sys_cur_st |= DianInfo[dian_index].dian_sample_st;
				}else
				  {

                    if((DianInfo[dian_index].sys_expect_st&0xf0)==DIDN_MPPT_ST)
                    {
                       DianInfo[dian_index].command |= DIAN_START_MPPT_CMD;					   	
                    }else if((DianInfo[dian_index].sys_expect_st&0xf0)==DIDN_NO_MPPT_ST)
                     {
                       DianInfo[dian_index].command |= DIAN_EXIT_MPPT_CMD;					   	
                    }
				  }
    		}	
		 //check DiDi
		if((DianInfo[dian_index].sys_cur_st&0x0f)==DIDI_ALL_SWITCHING_ST)
		{
		    DEBUG_DIS(printf("\n DianInfo[%d].didi_sample_st=%x",dian_index,DianInfo[dian_index].didi_sample_st));	
			if(Check_nums(DianInfo[dian_index].didi_sample_st)==((dian_table->count[0]<<8)+dian_table->count[1]))
			{
			  DianInfo[dian_index].sys_cur_st &= 0xf0;
			  DianInfo[dian_index].sys_cur_st |= (DianInfo[dian_index].sys_expect_st&0x0f);
			}else
			  {
				DianInfo[dian_index].command |=DIDI_ALL_ENTER_NOMAL_CMD;
			  }
		}	
   }else{
    	
            if((DianInfo[dian_index].sys_cur_st&0xf0)==DIDN_MPPT_ST)      	   
            {
                // Dian reset
				if((DianInfo[dian_index].dian_sample_st)!=DianInfo[dian_index].sys_cur_st)
               	{
					DianInfo[dian_index].command&=0x0f;
					DianInfo[dian_index].command |= DIAN_START_MPPT_CMD;
                }
             DEBUG_DIS(printf("\n low_power_count=%x",DianInfo[dian_index].low_power_count));	
                if(	DianInfo[dian_index].low_power_count<3)
                {
                  DianInfo[dian_index].command&=0x0f;
                  DianInfo[dian_index].command |= DIAN_EXIT_MPPT_CMD;
                }
            }
    		if((DianInfo[dian_index].sys_cur_st&0xf0)==DIDN_NO_MPPT_ST) //DiAn stablised     	   
            {
               //DiAn reset
                if((DianInfo[dian_index].dian_sample_st)!=DianInfo[dian_index].sys_cur_st)
               	{
					DianInfo[dian_index].command&=0x0f;
					DianInfo[dian_index].command |= DIAN_EXIT_MPPT_CMD;
                }
			   
                if(	DianInfo[dian_index].low_power_count>=3)
                {
                  DianInfo[dian_index].command&=0x0f;
                  DianInfo[dian_index].command |= DIAN_START_MPPT_CMD;
                }else{
					   CalcuBusVol(dian_index,(dian_table->offset[0]<<8)+dian_table->offset[1],
						           (dian_table->count[0]<<8)+dian_table->count[1]);
					   DianInfo[dian_index].command = DIAN_ADJUST_VOL_CMD;
                	}
            }
    		ResetExpectState(dian_index);
      	}
  	}
  }
    //start to send control command to dian and all didi 
    DEBUG_DIS(printf("\n HAVST_State=%d\n",HAVST_State));
    CmdCurDianIndex=0;
	EMN_SetParaType=HAVST_SEND_CMD;
    HAVST_State = DI_HAVST_HANDLE_COMMAND;
    EMN_SendMsg.dest_id= EMN_HAVST_TASK;	  
    EMN_SendMsg.msg_id = EMN_HAVST_CONTROL_CMD_REQ;	
    SYS_SendMsg(&EMN_SendMsg);
  }else
  	{
      NewRoundCollectData();
	  EMN_APL_SampleFromTop();
	  HavstHasDataToHandle=0;
	  CmdCurDianIndex=0;
	  EMN_SetParaType=NO_USED;
	  HAVST_State = DI_HAVST_START;		
	  DEBUG_DIS(printf("\n HAVST_State=%d\n",HAVST_State));
	  EMN_SendMsg.dest_id = EMN_APL_TASK;
	  EMN_SendMsg.msg_id = EMN_NWK_START_GET_DATA;
	  //DEBUG_DIS(printf("\n EMN_APL_GetSampleData"));		 
	  SYS_SendMsg(&EMN_SendMsg);
  	}
}

/*----------------------------------------------------------
 *
 * Function Name  : ResetExpectState
 * Returned Value : void
 * Comments		 :
 *
 *-----------------------------------------------------------*/
static void ResetExpectState(UINT_8 dian_index)
{
	DianInfo[dian_index].sys_expect_st=0;
	switch((DianInfo[dian_index].command)&0xf0)
	{
	  case DIAN_START_MPPT_CMD: 	
			 DianInfo[dian_index].sys_expect_st |=DIDN_MPPT_ST;
			 break;
	  case DIAN_EXIT_MPPT_CMD:
			 DianInfo[dian_index].sys_expect_st |=DIDN_NO_MPPT_ST;
			 break;
	  case DIAN_ADJUST_VOL_CMD:
			 DianInfo[dian_index].sys_expect_st |=DIDN_NO_MPPT_ST;
			 break;
	  default:	break;	
	}
	switch((DianInfo[dian_index].command)&0x0f)
	{
	  case DIDI_ALL_ENTER_NOMAL_CMD:
			 DianInfo[dian_index].sys_expect_st |= DIDI_ALL_NORMAL_ST;
		   break;
	  case DIDI_CHANGE_MPPT_MODE_CMD:
			 DianInfo[dian_index].sys_expect_st |= DIDI_ALL_NORMAL_ST;
		   break;
	  default:	break;	
	}


}
/*----------------------------------------------------------
 *
 * Function Name  : HAVST_HandleCmdReq
 * Returned Value : void
 * Comments		 :
 *
 *-----------------------------------------------------------*/
static void HAVST_HandleCmdReq(void)
{
  UINT_16  dian_count;
  dian_count=GetSysDianCount();
   
  if(CmdCurDianIndex>=dian_count)
  {//finish send command to DiAn and DiDi 
	  NewRoundCollectData();
	  EMN_APL_SampleFromTop();
	  HavstHasDataToHandle=0;
	  CmdCurDianIndex=0;
	  EMN_SetParaType=NO_USED;
	  HAVST_State = DI_HAVST_START;	
	  DEBUG_DIS(printf("\n HAVST_State=%d\n",HAVST_State));
	  EMN_SendMsg.dest_id = EMN_APL_TASK;
	  EMN_SendMsg.msg_id = EMN_NWK_START_GET_DATA;
	  //DEBUG_DIS(printf("\n EMN_APL_GetSampleData"));		 
	  SYS_SendMsg(&EMN_SendMsg);
	  return;
   }
  while(CmdCurDianIndex<dian_count)
  {
  //  DEBUG_DIS(printf("\n DianInfo[CmdCurDianIndex].command=%d\n",DianInfo[CmdCurDianIndex].command));
    if((DianInfo[CmdCurDianIndex].command)&0xf0)		
    {
      
	  DEBUG_DIS(printf("\n SendDiAnCmd\n"));
	    SendDiAnCmd(pDiAnDiDiTable->indextab[CmdCurDianIndex].dianaddr[0],
			        pDiAnDiDiTable->indextab[CmdCurDianIndex].dianaddr[1],
			        DianInfo[CmdCurDianIndex].command&0xf0,NOBROADCAST_FLAG);
		DianInfo[CmdCurDianIndex].command &= 0x0f;
		if((DianInfo[CmdCurDianIndex].sys_cur_st&0xf0)!=DIDN_SWITCHING_ST)
		{
		  DianInfo[CmdCurDianIndex].sys_cur_st &= 0x0f;
		  DianInfo[CmdCurDianIndex].sys_cur_st |= DIDN_SWITCHING_ST;
		} 
		if(DianInfo[CmdCurDianIndex].command==0x00)
		{  
		   CmdCurDianIndex++;
		}   
		break;
		
    }else if((DianInfo[CmdCurDianIndex].command)&0x0f)            
         {
          DEBUG_DIS(printf("\n SendAllDiDiCmd\n"));
		   SendAllDiDiCmd(pDiAnDiDiTable->didiaddr[pDiAnDiDiTable->indextab[CmdCurDianIndex].offset[0]<<8+pDiAnDiDiTable->indextab[CmdCurDianIndex].offset[1]],
		  	             pDiAnDiDiTable->didiaddr[(pDiAnDiDiTable->indextab[CmdCurDianIndex].offset[0]<<8+pDiAnDiDiTable->indextab[CmdCurDianIndex].offset[1])+1],
		  	             DianInfo[CmdCurDianIndex].command&0x0f,BROADCAST_FLAG);
		   DianInfo[CmdCurDianIndex].command &= 0xf0;
		   if(((DianInfo[CmdCurDianIndex].sys_cur_st&0xf0)!=DIDI_ALL_SWITCHING_ST))
		   {
		      DianInfo[CmdCurDianIndex].sys_cur_st &= 0xf0;
		      DianInfo[CmdCurDianIndex].sys_cur_st |= DIDI_ALL_SWITCHING_ST;
		   }
		   if(DianInfo[CmdCurDianIndex].command==0x00)
		   {  
		      CmdCurDianIndex++;
		   }  
		   break;
		   
        }else
        	{
               CmdCurDianIndex++;
			 //  DEBUG_DIS(printf("\n CmdCurDianIndex++\n"));
			   if(CmdCurDianIndex>=dian_count)
               {//finish send command to DiAn and DiDi 
            	  NewRoundCollectData();
            	  EMN_APL_SampleFromTop();
            	  HavstHasDataToHandle=0;
            	  CmdCurDianIndex=0;
            	  EMN_SetParaType=NO_USED;
            	  HAVST_State = DI_HAVST_START;		 
            	  EMN_SendMsg.dest_id = EMN_APL_TASK;
            	  EMN_SendMsg.msg_id = EMN_NWK_START_GET_DATA;
				  DEBUG_DIS(printf("\n HAVST_State=%d\n",HAVST_State));
            	  //DEBUG_DIS(printf("\n EMN_APL_GetSampleData"));		 
            	  SYS_SendMsg(&EMN_SendMsg);
            	  break;
               }
			   
        	}
      
   }
//  DEBUG_DIS(printf("\n CmdCurDianIndex=%d\n",CmdCurDianIndex));

 
}
/*----------------------------------------------------------
 *
 * Function Name  : HAVST_HandleCmdCfm
 * Returned Value : void
 * Comments		 : reset all variblies for new round handle
 *
 *-----------------------------------------------------------*/
static void NewRoundCollectData(void)
{
	UINT_16 i;
	for(i=0; i<REG_DIAN_NUM; i++)
	{
		DianInfo[i].low_power_count=0;	
		DianInfo[i].command=0;
		DianInfo[i].dian_bus_voltage=0;
		DianInfo[i].dian_sample_st=0;
	}	 
	for(i=0; i<REG_DIDI_NUM; i++)
	{
		DidiInfo[i].scale=0;
		DidiInfo[i].vol_in=0;
		DidiInfo[i].vol_out=0;
	}
}

/*----------------------------------------------------------
 *
 * Function Name  : HAVST_HandleCmdCfm
 * Returned Value : void
 * Comments		 :
 *
 *-----------------------------------------------------------*/
static void HAVST_HandleCmdCfm(void)
{
   SYS_StopTimer(&HavstTimer);
   EMN_SendMsg.dest_id= EMN_HAVST_TASK;		
   EMN_SendMsg.msg_id = EMN_HAVST_CONTROL_CMD_REQ;
   DEBUG_DIS(printf("\n HAVST_HandleCmdCfm\n"));
   SYS_SendMsg(&EMN_SendMsg);
}

/*----------------------------------------------------------
 *
 * Function Name  : SendDiAnCmd
 * Returned Value : void
 * Comments		 :
 *
 *-----------------------------------------------------------*/
static void SendDiAnCmd(UINT_8 subnet_addr,UINT_8 dev_addr,
                                UINT_8 cmd, UINT_8 send_mode)
{
	ParaType cmddata;
	switch(cmd)
	{
	   case DIAN_START_MPPT_CMD:   	
	   case DIAN_EXIT_MPPT_CMD:
			cmddata.brd=send_mode;
			cmddata.index=200;
			cmddata.len=0x01;
			cmddata.dev=dev_addr;//0X09;			
			cmddata.data[0]=cmd;
			EMN_APL_SetParameter(subnet_addr,dev_addr,&cmddata);
			if(cmd==DIAN_START_MPPT_CMD)
				{
				 DEBUG_DIS(printf("\n set DIAN_START_MPPT_CMD command\n"));
				}
			else if(cmd==DIAN_EXIT_MPPT_CMD)
				{
				 DEBUG_DIS(printf("\n set DIAN_EXIT_MPPT_CMD command\n"));
				}
			EMN_SendMsg.data[0]=subnet_addr;
			EMN_SendMsg.data[1]=dev_addr;
			EMN_SendMsg.dest_id = EMN_APL_TASK;
			EMN_SendMsg.msg_id = EMN_NWK_START_SET_DATA;		  
			SYS_SendMsg(&EMN_SendMsg);
			  break;
	   case DIAN_ADJUST_VOL_CMD:
	   		cmddata.brd=send_mode;
			cmddata.index=200;
			cmddata.len=0x03;
			cmddata.dev=dev_addr;//0X09;			
			cmddata.data[0]=cmd;
			cmddata.data[1]=(UINT_8)(DianInfo[CmdCurDianIndex].dian_bus_voltage>>8);
			cmddata.data[2]=(UINT_8)DianInfo[CmdCurDianIndex].dian_bus_voltage;
			DEBUG_DIS(printf("\n adjust dian bus voltage=%d\n",DianInfo[CmdCurDianIndex].dian_bus_voltage));
			EMN_APL_SetParameter(subnet_addr,dev_addr,&cmddata);   
			EMN_SendMsg.data[0]=subnet_addr;
			EMN_SendMsg.data[1]=dev_addr;
			EMN_SendMsg.dest_id = EMN_APL_TASK;
			EMN_SendMsg.msg_id = EMN_NWK_START_SET_DATA;	  
			SYS_SendMsg(&EMN_SendMsg);
			  break;
	   default:  break;  
	}
   if(send_mode==BROADCAST_FLAG)
   {
     //  DEBUG_DIS(printf("\n send_mode==BROADCAST_FLAG\n"));
      HavstTimer.value = 1000;
      HavstTimer.msgID = SYS_TIMEOUT_HAVST_DIAN_CMD;
      SYS_StartTimer(&HavstTimer);
   }	
}

/*----------------------------------------------------------
 *
 * Function Name  : SendAllDiDiCmd
 * Returned Value : void
 * Comments		 :
 *
 *-----------------------------------------------------------*/
static void  SendAllDiDiCmd(UINT_8 subnet_addr,UINT_8 dev_addr,
                                   UINT_8 cmd,UINT_8 send_mode)
{
	ParaType cmddata;

	cmddata.brd=send_mode;
	cmddata.index=200;
	cmddata.len=0x01;
	cmddata.dev=dev_addr;//0X09;
	cmddata.data[0]=cmd;
	EMN_APL_SetParameter(subnet_addr,dev_addr,&cmddata);
	EMN_SendMsg.data[0]=subnet_addr;
	EMN_SendMsg.data[1]=dev_addr;
	EMN_SendMsg.dest_id = EMN_APL_TASK;
	EMN_SendMsg.msg_id = EMN_NWK_START_SET_DATA; 	 
	SYS_SendMsg(&EMN_SendMsg);
    if(cmd==DIDI_ALL_ENTER_NOMAL_CMD)
    	{
    	   DEBUG_DIS(printf("\n set DIDI_ALL_ENTER_NOMAL_CMD command\n"));
    	}
    else if(cmd==DIDI_CHANGE_MPPT_MODE_CMD)
    	{
    	   DEBUG_DIS(printf("\n set DIDI_CHANGE_MPPT_MODE_CMD command\n"));
    	}
   if(send_mode==BROADCAST_FLAG)
   {
   //   DEBUG_DIS(printf("\n send_mode==BROADCAST_FLAG\n"));
      HavstTimer.value = 1000;
      HavstTimer.msgID = SYS_TIMEOUT_HAVST_DIDI_CMD;
      SYS_StartTimer(&HavstTimer);
   }
}

/*----------------------------------------------------------
 *
 * Function Name  : CaluBusVol
 * Returned Value : void
 * Comments		 :
 *
 *-----------------------------------------------------------*/
 #define HAVST_SCALE_COUNT   (32)
typedef struct 
{
 UINT_16  offset;
 UINT_16  vol;
} DIDI_INFO_TEMP;

static void  CalcuBusVol(UINT_8 dian_index,UINT_16 offset, 
                              UINT_16 didinum)
{
    UINT_16 min=0,max=0,i,calcu_vbus;
	UINT_32 sum=0;
	UINT_8  j,m;
	UINT_8  which_sector;
	UINT_8  *buffer;
	DIDI_INFO_TEMP  *p_maxsector;

	//search for max and min to calcuate scale 
	//scale == constant voltage(2V)
    for(i=offset;i<(offset+didinum);i++)
    {
        if(max< DidiInfo[i].vol_out)
        { 
           max=DidiInfo[i].vol_out;
        }
		if(min>DidiInfo[i].vol_out)
		{
		   min= DidiInfo[i].vol_out;
		}
		sum+=DidiInfo[i].vol_out;
    }	
	buffer=(UINT_8 *) _mem_alloc_zero(32);
	// search for sector that has max number of DiDi
	for(i=offset;i<(offset+didinum);i++)
	{
	   buffer[(DidiInfo[i].vol_out-min)/200]++;
	   DidiInfo[i].scale=(DidiInfo[i].vol_out-min)/200;
	}	
	max=0;     //number of DiDi in sector	
	for(i=0;i<HAVST_SCALE_COUNT;i++)
	{
        if(max<= buffer[i])    //have two sectors that have the same number of DiDi???????
        { 
             max=buffer[i];
			 which_sector=i;
        }
	}
    _mem_free(buffer);

	//search sampe DiDi in sample sector
	   //at first, order data
	p_maxsector=(DIDI_INFO_TEMP *) _mem_alloc_zero((max+3)*sizeof(DIDI_INFO_TEMP));
	for(i=offset,m=0;i<(offset+didinum);i++)
	{
	  if(DidiInfo[i].scale==which_sector)
	  {
	    if(m==0)
	    {
	      p_maxsector[m].vol=DidiInfo[i].vol_out;
		  p_maxsector[m].offset=i;
	    }else
		   {
		     for(j=m;j>0;j--)
		     {
		        if(DidiInfo[i].vol_out>p_maxsector[j].vol)
		        {
				     //insert the value.
				   p_maxsector[j+1].vol=DidiInfo[i].vol_out;
				   p_maxsector[j+1].offset=i;
				   break;
				   
		        }else
		        	{
                       p_maxsector[j+1].vol=p_maxsector[j].vol;
				       p_maxsector[j+1].offset=p_maxsector[j].offset;
		        	}
		     }
		   }
	    m++;
		if(m>=max)
		{
		   break;
		}
	  }	
	}
	max=max/2;        //?????  need to improved
	offset=p_maxsector[max].offset;
    _mem_free(p_maxsector);
	calcu_vbus= (((DidiInfo[offset].vol_in-1)*sum)/(DidiInfo[offset].vol_out))/100;
     
	 if(calcu_vbus>DIAN_VBUS_MAX)
	 	{
	 	   DianInfo[dian_index].dian_bus_voltage =DIAN_VBUS_MAX;
	 	}
	 else if(calcu_vbus<DIAN_VBUS_MIN)
	 	{
	 	   DianInfo[dian_index].dian_bus_voltage =DIAN_VBUS_MIN;
	 	}
	 else
	 	{
	 	   DianInfo[dian_index].dian_bus_voltage =calcu_vbus;
	 	}
	 	DEBUG_DIS(printf("\n calcu_vbus=%x \n",DianInfo[dian_index].dian_bus_voltage));
}

/*--------------------------------------------------------
 *
 *     the implementaion about network
 *
 *-------------------------------------------------------*/ 
void EMN_HAVST_Task(SYS_MESSAGE *msg)
{
  
   switch(HAVST_State)
   {
     case  DI_HAVST_START:		   
     case  DI_HAVST_COLLECT_DATA:
	 	    break;
     case  DI_HAVST_PARSE_DATA:
	 	     if(msg->msg_id==EMN_HAVST_PARSE_INFO_CMD)
	 	     {
	 	       HAVST_ParsePlantInfo();
	 	     }  
		   	 break;
     case  DI_HAVST_HANDLE_COMMAND:
	 	    	 switch(msg->msg_id)
				 {
				   case EMN_HAVST_CONTROL_CMD_REQ:
				   	         HAVST_HandleCmdReq();
				   	         break;
				   case EMN_HAVST_CONTROL_CMD_CFM:
				   	         HAVST_HandleCmdCfm();
				   	         break; 
				   case EMN_TIMEOUT:
				   	   	     switch(msg->data[0])
   	     	                 {
   	     	                  case  SYS_TIMEOUT_HAVST_DIAN_CMD:
							  case  SYS_TIMEOUT_HAVST_DIDI_CMD:	
							  	    DEBUG_DIS(printf("\n SYS_TIMEOUT_HAVST_DIDI_CMD\n"));
                                    EMN_SendMsg.dest_id= EMN_HAVST_TASK;	  
                                    EMN_SendMsg.msg_id = EMN_HAVST_CONTROL_CMD_REQ;
									SYS_SendMsg(&EMN_SendMsg);
									break;
							  default :
							  	      break;
				   	   	     }
				   default: break;		  
				  }
		   	 break;
	 
       default :
	   	     break;

   }

}
/*----------------------------------------------------------
 *
 * Function Name  : EMN_HAVST_TaskInit
 * Returned Value : void
 * Comments		 :
 *
 *-----------------------------------------------------------*/

void EMN_HAVST_TaskInit(void)
{
   UINT_8 i;
   UINT_16 dian_index,didi_index,didi_offset;
   HavstTimer.id = SYS_TIMER4;
   HavstTimer.fastfun = NULL;
   HavstTimer.taskID = EMN_HAVST_TASK;
   HavstTimer.value =0;
   HavstTimer.msgID = 0;  
   REG_DIDI_NUM = GetSysDidiCount();
   for(i=0; i<REG_DIAN_NUM; i++)
   {
	   DianInfo[i].sys_expect_st=0;
	   DianInfo[i].sys_cur_st=DIDN_INIT_ST|DIDI_ALL_INIT_ST; 
	   DianInfo[i].low_power_count=0;
	   DianInfo[i].didi_sample_st=0;
	   DianInfo[i].command=0;
       DianInfo[i].dian_bus_voltage=0;
	   DianInfo[i].init_end=0;
   }	
   for(i=0; i<REG_DIDI_NUM; i++)
   {
       DidiInfo[i].scale=0;
	   DidiInfo[i].vol_in=0;
	   DidiInfo[i].vol_out=0;
   }
   HAVST_State=DI_HAVST_START;         //DiCor reset
   EMN_SetParaType=NO_USED;
   HavstHasDataToHandle=0;
   DEBUG_DIS(printf("\n HAVST_State=%d\n",HAVST_State));
}
#endif
/* EOF */


