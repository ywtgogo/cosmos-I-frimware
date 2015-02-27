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
#ifdef DI_HARVESTER_V4
#include <mqx.h>
#include <bsp.h>
#include "unconnected_grid.h"

#include <eeprom.h>

UINT_8 HAVST_State;
static SYS_TIMER HavstTimer;
static UINT_8    HavstHasDataToHandle;
UINT_8 REG_DIDI_NUM;
extern DIANDIDITABLE_PTR pDiAnDiDiTable;
static DIDI_INFO    DidiInfo[REG_DIDI_NUM_EXCEPT];
static UINT_8       DidiCmd[REG_DIDI_NUM_EXCEPT/2];
static STRING_INFO  StringInfo[REG_STRING_NUM];



extern void    EMN_APL_SampleFromTop(void);
static void    NewRoundCollectData(void);
static void    GetDianIndexByDidi(UINT_8 subnet,UINT_8 dev,UINT_16 *dian_index,UINT_16 *didi_index,UINT_16 *didi_offset);
static void    GetDianIndexByDian(UINT_8 subnet,UINT_8 dev,UINT_16 *dian_index);
static UINT_16 GetSysDidiCount(void);
#define GetSysStringCount()     (((UINT_16)pDiAnDiDiTable->diancount[0])<<8)|pDiAnDiDiTable->diancount[1]



/*----------------------------------------------------------
 *
 * Function Name  : HAVST_CollectInfo(void)
 * Returned Value : void
 * Comments		 :
 *
 *-----------------------------------------------------------*/
#define DEVICE_TYPE_DIDI   (0x10)
#define DEVICE_TYPE_DIAN   (0x20)
void HAVST_CollectPlantInfo(UINT_8 *data,UINT_16 data_num)
{
   UINT_16 i,didi_index,dian_index,didi_offset,cur_incurrent_out,cur_vol_out;
   UINT_32 temp_power;
   if((HAVST_State==DI_HAVST_COLLECT_DATA)&& data_num)
   {
    HavstHasDataToHandle=1;
     for(i=0;i<data_num;)
     {
        //DiDi device 
        if(data[i]==DEVICE_TYPE_DIDI)
        {
          GetDianIndexByDidi(data[i+2],data[i+3],&dian_index,&didi_index,&didi_offset);
		  cur_incurrent_out=(UINT_16)((UINT_16)data[i+9]<<8|data[i+8]);
		  cur_vol_out=(UINT_16)((UINT_16)data[i+11]<<8|data[i+10]);
          temp_power=cur_incurrent_out*cur_vol_out;
		  DidiInfo[didi_index].cur_power_out=temp_power/100;//(temp_power>>10+temp_power>>5+temp_power>>7)>>2;
          DidiInfo[didi_index].state |= ((data[i+15]>>3)&0x01); 
		//  DEBUG_DIS(printf("\n DidiInfo[%d].state=%d\n",didi_index,DidiInfo[didi_index].state));
		//  DEBUG_DIS(printf("\n DidiInfo[%d].cur_power_out=%d\n",didi_index,DidiInfo[didi_index].cur_power_out));
		 // DiAn device  
        }          
		i+=data[i+1]; 
		
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
     UINT_16 *didi_table_p,didi=0,i,j,didisum,diansum;
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
	 diansum =GetSysStringCount();
	 for(j=0;j<diansum;j++)
	 {
       if((2*(*didi_index) >=(pDiAnDiDiTable->indextab[j].offset[0]<<8+pDiAnDiDiTable->indextab[j].offset[1]))&&
           (2*(*didi_index) <(pDiAnDiDiTable->indextab[j+1].offset[0]<<8+pDiAnDiDiTable->indextab[j+1].offset[1])))	   	
       	{
           	   	 *dian_index = j;
				 *didi_offset=2*(*didi_index)-(pDiAnDiDiTable->indextab[j].offset[0]<<8+pDiAnDiDiTable->indextab[j].offset[1]);
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
    diansum=GetSysStringCount();
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
    diansum=GetSysStringCount();
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
  
  UINT_8  string_index,string_count,i,small_count=0,value_count;
  UINT_32 mppt_powerL=0,mppt_powerH=0,max_power1=0,max_power2=0;
  UINT_32	sum_aver=0,Power_Down_state=0;
  DIANDIDIINDEXTAB   *dian_table;
  if(HavstHasDataToHandle)
  {
    for(i=0,sum_aver=0; i<REG_DIDI_NUM; i++)
    { 
    
      if((max_power1<= DidiInfo[i].cur_power_out))
      	{

	      	 max_power2 = max_power1;
	         max_power1 = DidiInfo[i].cur_power_out;
      	}
	  else if(max_power2 < DidiInfo[i].cur_power_out)
	  	{
	  	      max_power2 = DidiInfo[i].cur_power_out;
	  	}
	  
		if((UINT_32)(DidiInfo[i].cur_power_out*100)>=(max_power1*90))
		  	{
	         sum_aver+= DidiInfo[i].cur_power_out;
		  	}    
		  else
		    {		    
		  	  small_count++;
			  Power_Down_state|=1<<i;
		  	}	
	  	}  
	
	if((max_power2 == 0)&&(max_power1 == 0))
	{		
		sum_aver = 0;
	}
	else if(((max_power2 == 0)&&(max_power1 != 0))||(max_power2*100 < max_power1*90))    
		{
		 value_count = (REG_DIDI_NUM-small_count-1);	
		}
	else 
		{ 
		 value_count = (REG_DIDI_NUM-small_count-2);
		}
		if(value_count == 0) 
		{
	    	value_count =1;	
		}
		
		sum_aver=(sum_aver-max_power1-max_power2)/value_count;
		mppt_powerL= 100*sum_aver-(sum_aver*3);
		mppt_powerH= 100*sum_aver+(sum_aver*3);
	
	  DEBUG_DIS(printf("\n sum_aver=%d\n",sum_aver));
	  DEBUG_DIS(printf("\n max_power1=%d\n",max_power1));
	  DEBUG_DIS(printf("\n max_power2=%d\n",max_power2));
    for(i=0;i<REG_DIDI_NUM; i++)
    {
     if((DidiInfo[i].cur_power_out<=30*100)&&(DidiInfo[i].cur_power_out>0))
      	{     
      	     Power_Down_state&=~(1<<i);
			// DEBUG_DIS(printf("\n <=30 didi %d DIDI_START_DIRECT_CMD \n",i));
	      	 if(DIDI_MPPT_ST==(DidiInfo[i].state&0x0f))
	          {	      	   
				DidiInfo[i].cmd=DIDI_START_DIRECT_CMD;
				DidiInfo[i].state=DIDI_DIRECT_ST<<4;
				DidiInfo[i].last_power_out= DidiInfo[i].cur_power_out;
				DEBUG_DIS(printf("\n <=30 didi %d DIDI_START_DIRECT_CMD \n",i));  
	      	  }
      	}
       else if((Power_Down_state&(1<<i))&&(DidiInfo[i].cur_power_out>0))
       	{
       	  if(DIDI_DIRECT_ST==(DidiInfo[i].state&0x0f))
       	  	{
				DidiInfo[i].cmd= DIDI_START_MPPT_CMD;
				DidiInfo[i].state=DIDI_MPPT_ST<<4;
				DidiInfo[i].last_power_out= DidiInfo[i].cur_power_out; 
				 DEBUG_DIS(printf("\n Power_Down_state DIDI_MPPT_ST \n"));
       	  	}
       	}
	   else if((DidiInfo[i].state>>4) ==(DidiInfo[i].state&0x0f))// higher 4bits:expected state  low 4bits:current state(sample state)
       {
          if(DIDI_MPPT_ST==(DidiInfo[i].state&0x0f))
          {
          
            /* if(DidiInfo[i].cmd!=0x00)// the first enter this handle
             {
                 if(DidiInfo[i].last_power_out<(DidiInfo[i].cur_power_out+5))  //can change state
                 {
                     DidiInfo[i].cmd=0x00;
					 DidiInfo[i].last_power_out= 0x00;
					 
                 
                 }else   
                 	{
						DidiInfo[i].cmd=DIDI_START_DIRECT_CMD;
						DidiInfo[i].state=DIDI_DIRECT_ST<<4;
						DidiInfo[i].last_power_out= DidiInfo[i].cur_power_out;
						DEBUG_DIS(printf("\n else didi %d DIDI_START_DIRECT_CMD \n",i));
                 	}
             }*/
			 if(((UINT_32)(100*DidiInfo[i].cur_power_out)>mppt_powerL)&&((UINT_32)(100*DidiInfo[i].cur_power_out)<mppt_powerH))
			 {
				 DidiInfo[i].cmd=DIDI_START_DIRECT_CMD;
				 DidiInfo[i].state=DIDI_DIRECT_ST<<4;
				 DidiInfo[i].last_power_out= DidiInfo[i].cur_power_out;
		    	 DEBUG_DIS(printf("\n DidiInfo[%d] DIDI_START_DIRECT_CMD",i));
			 }
			
          }else if(DIDI_DIRECT_ST==(DidiInfo[i].state&0x0f))
          	{
			/*  if(DidiInfo[i].cmd!=0x00)// the first enter this handle
			  {
				  if(DidiInfo[i].last_power_out<(DidiInfo[i].cur_power_out+5))	//can change state
				  {
					  DidiInfo[i].cmd=0x00;
					  DidiInfo[i].last_power_out= 0x00;
				  //  DEBUG_DIS(printf("\n DIDI_MPPT_ST cmd=0 \n"));
				  }else
					 {
						 DidiInfo[i].cmd= DIDI_START_MPPT_CMD;
						 DidiInfo[i].state=DIDI_MPPT_ST<<4;
						 DidiInfo[i].last_power_out= DidiInfo[i].cur_power_out;
					     DEBUG_DIS(printf("\n didi %d DIDI_START_MPPT_CMD \n",i));
					 }
			  }*/
			  if(((100*DidiInfo[i].cur_power_out<=mppt_powerL)&&(DidiInfo[i].cur_power_out>0))||(100*DidiInfo[i].cur_power_out>=mppt_powerH))
			  {
				  DidiInfo[i].cmd=DIDI_START_MPPT_CMD;
				  DidiInfo[i].state=DIDI_MPPT_ST<<4;
				  DidiInfo[i].last_power_out= DidiInfo[i].cur_power_out;
				  DEBUG_DIS(printf("\n DidiInfo[%d] DIDI_START_MPPT_CMD",i));
			  }
		
          	}
		  	

       }
      		
    }
    
#if 0	
    string_count=GetSysStringCount();
    dian_table=&(pDiAnDiDiTable->indextab[0]);
    for(string_index=0; string_index<string_count;string_index++,dian_table++)
    {
    }
#endif 	
    NewRoundCollectData();
  }
  EMN_APL_SampleFromTop();
  HavstHasDataToHandle=0;
  HAVST_State = DI_HAVST_START;		
  DEBUG_DIS(printf("\n HAVST_State=%d\n",HAVST_State));
  EMN_SendMsg.dest_id = EMN_APL_TASK;
  EMN_SendMsg.msg_id = EMN_NWK_START_GET_DATA;
   //DEBUG_DIS(printf("\n EMN_APL_GetSampleData"));		 
  SYS_SendMsg(&EMN_SendMsg);
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
	for(i=0; i<REG_STRING_NUM; i++)
	{

	}	 
	for(i=0; i<REG_DIDI_NUM; i++)
	{
        DidiInfo[i].state &=0xf0;
		DidiInfo[i].cur_power_out=0;
		DidiInfo[i].vol_in=0;
		DidiInfo[i].vol_out=0;
	}

}
/*----------------------------------------------------------
 *
 * Function Name  : NWK_CordMakeCmdPacket
 * Returned Value : void
 * Comments		 :
 *
 *-----------------------------------------------------------*/

void NWK_CordMakeCmdPacket(UINT_8 subnet,UINT_8 *frame_data)
{
	UINT_16  i,num;
	for(i=0;i<16;i++)
	{
      frame_data[i]=0;
	}
	num=0;
  	for(i=0; i<REG_DIDI_NUM; i++)
	{
       if(pDiAnDiDiTable->didiaddr[2*i]==subnet)
       {
          // DEBUG_DIS(printf("\n DidiInfo[%d].cmd=0x%x",i,DidiInfo[i].cmd));
		   frame_data[(2*pDiAnDiDiTable->didiaddr[2*i+1])/8+2]|=((DidiInfo[i].cmd&0x0f)<<2*(pDiAnDiDiTable->didiaddr[2*i+1]%4));
		  //DEBUG_DIS(printf("\n frame_data[%d]=%x",((2*pDiAnDiDiTable->didiaddr[2*i+1])/8+2),frame_data[(2*pDiAnDiDiTable->didiaddr[2*i+1])/8+2]));
		   num++;
       }
	}
    frame_data[0]=EMN_HAVST4_GET_SAMPLE_CMD_FORMAT3;
	frame_data[1]=num;
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
   UINT_16 sting_index,didi_index,didi_offset;
   HavstTimer.id = SYS_TIMER4;
   HavstTimer.fastfun = NULL;
   HavstTimer.taskID = EMN_HAVST_TASK;
   HavstTimer.value =0;
   HavstTimer.msgID = 0;  
   REG_DIDI_NUM = GetSysDidiCount(); 
   for(i=0; i<REG_STRING_NUM; i++)
   {
     ;
   }	
   for(i=0; i<REG_DIDI_NUM; i++)
   {
      DidiInfo[i].state =DIDI_DIRECT_ST<<4;
	  DidiInfo[i].cur_power_out=0;
	  DidiInfo[i].last_power_out=0;
	  DidiInfo[i].cmd=DIDI_START_DIRECT_CMD;
   }
   HAVST_State=DI_HAVST_START;         //DiCor reset
   HavstHasDataToHandle=0;
  // DEBUG_DIS(printf("\n HAVST_State=%d\n",HAVST_State));
}
#endif
/* EOF */


