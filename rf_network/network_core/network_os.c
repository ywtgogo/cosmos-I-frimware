/**********************************************************************
* 
* 	Copyright (c) 2010 ;
* 	All Rights Reserved Convertegy
* 	FileName: network_os.c
* 	Version : 
* 	Date    : 
*
* Comments:
*      
*
*************************************************************************/
#include <mqx.h>
#include <bsp.h>
#include <Watchdog.h>
#include "network_frame.h"
#include "network_phy.h"
#include "network_dlc.h"
#include "network_nwk.h"
#include "dicor_network_apl.h"
#include "dicor_spi_dri.h"
#include  "dicor_upload.h"
#include "di-harvest.h"


UINT_16 SYS_GetTime(SYS_TIMER timer);
void Wacthdog_Error(pointer td_ptr);
extern void EMN_HAVST_Task(SYS_MESSAGE *msg);
extern void EMN_HAVST_TaskInit(void);
extern SET_PARA_TYPE  EMN_SetParaType;

/*---------------------------------------------------------
 *
 *     the implementaion about timer
 *
 *----------------------------------------------------------*/


#define CHECK_BIT(val,n)  (val&(0x01 <<n))
static  UINT_32  SYS_Tick;
ParaType ParaData0;
UINT_8 SetSubnet,SetDev,SetType;

LWSEM_STRUCT NetworkRfSem;


typedef  struct
{
 UINT_32   value[SYS_TIME_MAX_NUM];
 UINT_8    msgID[SYS_TIME_MAX_NUM];
 UINT_8    taskID[SYS_TIME_MAX_NUM];
//fast  handle
 void	   (*fastfun[SYS_TIME_MAX_NUM])(void);
// void	   (*slowfun[SYS_TIME_MAX_NUM])(void);
 UINT_8    timeout;  // if timeout happen not.
 UINT_8    seting;   // if this timer is set not.
}SYS_TIMERS;
volatile static SYS_TIMERS   SYS_Timers;
static UINT_8       _TimerEnable; 
static UINT_8       _NoHandleNum;

#define SYS_EnableTimerISR()  { _TimerEnable = 1;}   //it should be finish in one CPU circle
#define SYS_DisableTimerISR() { _TimerEnable = 0;}


void SYS_TimerInit(void)
{
  UINT_8 i;
  SYS_Timers.seting = 0;
  SYS_Timers.timeout = 0;
  for(i=0;i<SYS_TIME_MAX_NUM;i++)
  {
       SYS_Timers.fastfun[i]= NULL;
//	   SYS_Timers.slowfun[i]= NULL;
	   SYS_Timers.value[i]= 0; 
       SYS_Timers.msgID[i] = EMN_PRIIMITIVE_MAX;
	   SYS_Timers.taskID[i]= EMN_TASK_MAX; 
  }
  _NoHandleNum =0;
  SYS_EnableTimerISR();
  SYS_Tick=0;
}

void SYS_StartTimer(SYS_TIMER *timer)
{
    SYS_DisableTimerISR();
	SYS_Timers.value[timer->id]= timer->value;
	SYS_Timers.taskID[timer->id] = timer->taskID;
	SYS_Timers.msgID[timer->id] = timer->msgID;
	SYS_Timers.fastfun[timer->id]= timer->fastfun;
	SYS_Timers.seting |= 0x01<<timer->id;
    SYS_Timers.timeout = (~(0x01<<timer->id))& SYS_Timers.timeout;
	SYS_EnableTimerISR();
}

void SYS_StopTimer(SYS_TIMER *timer)
{
    SYS_DisableTimerISR();
	SYS_Timers.value[timer->id]= 0;
	SYS_Timers.fastfun[timer->id]= NULL;
	SYS_Timers.taskID[timer->id] = EMN_TASK_MAX;
	SYS_Timers.msgID[timer->id] = EMN_PRIIMITIVE_MAX;
	SYS_Timers.seting  = (~(0x01<<timer->id))& SYS_Timers.seting;;
    SYS_Timers.timeout = (~(0x01<<timer->id))& SYS_Timers.timeout;
	SYS_EnableTimerISR();
}

UINT_8  SYS_CkeckTimer(SYS_TIMER *timer)
{

    if(CHECK_BIT(SYS_Timers.timeout,timer->id)) 
	{ 
	    return TRUE;
	}else
	    { 
	       return FALSE;
		}

}
UINT_16 SYS_GetTime(SYS_TIMER timer)
{

  return SYS_Timers.value[timer.id];
}

extern volatile UINT_32 LostLogTime;

void SYS_TimersISR(void *dummy) 
{

    UINT_8  i;
	SYS_Tick++;
	if(SYS_Tick == 0xFFFFFFFF)
	{
	  SYS_Tick=0;
	}  
	if(!_TimerEnable)
	{
	    _NoHandleNum++;
#if 	EMN_EMT_ONLY    
   _mcf5225_timer_clear_int(RFNWK_TIMER);
#endif		
		return;
	}
	if(!(SYS_Timers.seting))
	{
#if 	EMN_EMT_ONLY    
		   _mcf5225_timer_clear_int(RFNWK_TIMER);
#endif
		return;
	}	
   do{

    for(i=0;i<SYS_TIME_MAX_NUM;i++)
    {
       if(CHECK_BIT(SYS_Timers.seting,i))
       	{
            SYS_Timers.value[i]--;
    		if(SYS_Timers.value[i]==0)
    		{
    			dicor_rf_signal(RFNET_EVENT);
				//_lwsem_post(&rfnet_sem);
    		    SYS_Timers.seting  = (~(0x01<<i))& SYS_Timers.seting;
    			SYS_Timers.timeout |= 0x01<<i;
    			if(SYS_Timers.fastfun[i]!= NULL)
    			{
    			    SYS_Timers.fastfun[i]();
    			}	
    		}
       	}    
    }
	
	if(_NoHandleNum)
	{
        _NoHandleNum--;
	}
 }while(_NoHandleNum > 0);
#if 	EMN_EMT_ONLY    
   _mcf5225_timer_clear_int(RFNWK_TIMER);
#endif
	LostLogTime++;
}

/*----------------------------------------------------------
 *
 *     the implementaion about task management and message handle 
 *
 *-----------------------------------------------------------*/
#define SYS_LOW_MESSAGE_MAX_NUM               (10)
#define SYS_HIGH_MESSAGE_MAX_NUM              (3)

const SYS_TASK_TEMPLATE SYS_TaskTable[]={
	{EMN_PHY_TASK,1,0,EMN_PHY_Task,EMN_PHY_TaskInit},
	{EMN_MAC_TASK,1,0,EMN_MAC_Task,EMN_MAC_TaskInit},
	{EMN_NWK_TASK,1,0,EMN_NWK_Task,EMN_NWK_TaskInit},
	{EMN_APL_TASK,1,0,EMN_APL_Task,EMN_APL_TaskInit},
	{EMN_HAVST_TASK,1,0,EMN_HAVST_Task,EMN_HAVST_TaskInit},	
	{EMN_TASK_MAX,0,0,0}
};
static SYS_MESSAGE  _LowMsgList[SYS_LOW_MESSAGE_MAX_NUM]; 
static UINT_8 _LowMsgHead, _LowMsgTail;
volatile static UINT_8 _LowCurSizes;



UINT_8  SYS_SendMsg(SYS_MESSAGE *msg)
{
   
  if(_LowCurSizes >= SYS_LOW_MESSAGE_MAX_NUM)
  {
      //return SYS_MSG_FULL;  // if this condition is meeted, MUST imporve system handle.
      SYS_Error(0x10);
  }else
  	{
         _LowCurSizes++;
		 _LowMsgList[_LowMsgTail].dest_id = msg->dest_id;
		 _LowMsgList[_LowMsgTail].msg_id = msg->msg_id;
		 _LowMsgList[_LowMsgTail].data[0] = msg->data[0];
 		 _LowMsgList[_LowMsgTail].data[1] = msg->data[1];
		 _LowMsgList[_LowMsgTail].data[2] = msg->data[2];
		 _LowMsgList[_LowMsgTail].data[3] = msg->data[3];		 
		 if((++_LowMsgTail) == SYS_LOW_MESSAGE_MAX_NUM)
		 {
		     _LowMsgTail = 0;
		 }
		 return SYS_MSG_OK;
  	}
}
extern void dicor_dis_timer(void);


extern EMN_MAC_DisNetwork(void);
extern UINT_8 NoRegTimes;
void EMN_APL_GetSampleData()
{
	
	
		EMN_SendMsg.dest_id = EMN_APL_TASK;
		EMN_SendMsg.msg_id = EMN_NWK_START_GET_DATA;
		DEBUG_DIS(printf("EMN_APL_GetSampleData\n"));		 
		SYS_SendMsg(&EMN_SendMsg);


}
void EMN_APL_GetParameter(UINT_8 subnet,UINT_8 dev,GetParaType * para)
{

		 if((EMN_SetParaType != HAVST_SEND_CMD)||(workmode != WORKMODE_DIHAVST4))
	   { 
          Get_Data_Flag =1;	
		}
		  GetParaData.index=para->index;
		  GetParaData.len=para->len;
		  GetParaData.dev=para->dev;
		  GetParaData.flag=0;
		  GetParaData.data[0]=para->data[0];
		  
		  SetSubnet=subnet;
		  SetDev=dev;
		  SetType=EMN_NWK_START_GET_PARA_DATA;
#if 0		
          EMN_SendMsg.dest_id = EMN_APL_TASK;
		  EMN_SendMsg.msg_id = EMN_NWK_START_GET_PARA_DATA;
          
		  DEBUG_DIS(printf("\n EMN_APL_GetParameter dev=%d",SetParameterData.dev));		  
		 SYS_SendMsg(&EMN_SendMsg);
#endif 
}
void EMN_APL_SetParameter(UINT_8 subnet,UINT_8 dev,ParaType *para)
{
      UINT_8 i;
	 if((EMN_SetParaType != HAVST_SEND_CMD)||(workmode != WORKMODE_DIHAVST4))
	{ 
       Get_Data_Flag =1;
	 }
	  
	  SetSubnet=subnet;
  if(workmode == WORKMODE_DIHAVST4)
  	{
	  if(para->brd==BROADCAST_FLAG)
	  	{
	  	  SetDev=0xff;
	  	}
	  else
	  	{
	  	  SetDev=dev;
	  	}
  	}
  else
  	{
  	  SetDev=dev;
  	}
	  
	  SetType=EMN_NWK_START_SET_DATA;
	 
	  SetParameterData.brd=para->brd;
	  SetParameterData.index=para->index;
	  SetParameterData.len=para->len;
	  SetParameterData.dev=para->dev;
	  SetParameterData.flag=0;
	  DEBUG_DIS(printf("\n EMN_APL_SetParameter dev=%d",SetParameterData.dev));
	  
	  for(i=0;i<para->len;i++)
		{
		 SetParameterData.data[i]=para->data[i];	 
	  	}
		
}


extern void dicor_low_vol_chk(void);



void SYS_Init(void)
{
	UINT_8	i;  
	
	SYS_TimerInit();
	//init message list
	_LowMsgHead  = 0;
	_LowMsgTail  = 0;
	_LowCurSizes = 0;
}



// At this stage, we don't handle priority and time slice, if necessary,we implemente them.
void SYS_MainFunction(void)  
{
  UINT_8  i,n;
  SYS_MESSAGE msg;
	_watchdog_start(3600000);
  //init system timer
  SYS_TimerInit();
   //init message list
  _LowMsgHead  = 0;
  _LowMsgTail  = 0;
  _LowCurSizes = 0;
  for(i = 0; i< EMN_TASK_MAX; i++)
  {
      (*SYS_TaskTable[i].task_init)();
  }
#if DICOR_STATIC_REG_TAB
  DEBUG_DIS(printf("\nRF_int end"));
//  dicor_rf_signal(RF_REG_END);  
#endif   

 //create timer for softeware watchdog
 //
// if (_watchdog_create_component(MCF5225_INT_SWT,Wacthdog_Error)!= MQX_OK)
 if (_watchdog_create_component(BSP_TIMER_INTERRUPT_VECTOR,Wacthdog_Error)!= MQX_OK) 	
 { 
   printf("RF task create watchdog failed !");
 }
 ParaData0.brd=0x00;
 ParaData0.index=0x4;
 ParaData0.len=0x01;
 ParaData0.dev=0x01;
 for(i=0;i<ParaData0.len;i++)
 ParaData0.data[i]=0x03;
 //EMN_APL_GetParameter(0x00,0x01,&ParaData0);
 EMN_APL_GetSampleData();
 _watchdog_stop();
 //send message to start set data
 /*EMN_SendMsg.dest_id= EMN_APL_TASK;
 EMN_SendMsg.msg_id =EMN_NWK_START_GET_DATA;
 SYS_SendMsg(&EMN_SendMsg);*/
 upload_buffer.state=WRITTING;
 //_watchdog_start(30000);
 _watchdog_start(60*60*1000);
//task switch and handle
//  _time_delay(1000*10);	// test watchdog

//用于和shell同步
  _lwsem_create(&NetworkRfSem, 1);
  
  while(1)
  {
	
//	dicor_low_vol_chk();	//低电压检测，放在优先级最高的任务里	
		//_lwsem_wait(&rfnet_sem);
	_lwsem_wait(&NetworkRfSem);
	while((_LowCurSizes != 0)||( SYS_Timers.timeout != 0)||(RF_RxReady ==1))
	{
      if(_LowCurSizes != 0)
      {
          if(_LowMsgList[_LowMsgHead].dest_id >= EMN_TASK_MAX)
          {
              SYS_Error(2);
          }
		  SYS_TaskTable[_LowMsgList[_LowMsgHead].dest_id].task_fun(&(_LowMsgList[_LowMsgHead]));
		  _LowCurSizes--;
		  if((++_LowMsgHead)==SYS_LOW_MESSAGE_MAX_NUM)
		  {
		      _LowMsgHead = 0;
		  }

      }
      if(RF_RxReady ==1)
	  {
		// directly 
		 msg.dest_id = EMN_PHY_TASK;
		 msg.msg_id = EMN_RF_RX_READY;	
		 RF_PhyGetDataEnd=0;
		 #if 0
		 SYS_TaskTable[EMN_PHY_TASK].task_fun(&msg);
		 #else
         RF_PhyGetDataEnd=0;
         SYS_SendMsg(&msg);
		 RF_RxReady =0;	
		 #endif
	  }
#if 0	  
	  if(upload_buffer.state==CAN_WRITE)
	  {
#if DICOR_STATIC_REG_TAB
    {
      EMN_SendMsg.dest_id= EMN_APL_TASK;
      EMN_SendMsg.msg_id =EMN_NWK_START_GET_DATA;
      SYS_SendMsg(&EMN_SendMsg);
      upload_buffer.state=WRITTING;
    }
#else
	    if(rfnwk_state== RF_NWK_DIS)
	    {
               upload_buffer.state=CAN_READ;
		//	   dicor_rf_signal(RF_GET_DATA_END);
		       DEBUG_DIS(printf("\nend data:no"));			  
	    }else if( rfnwk_state== RF_NWK_REGING)
	      {   if(NoRegTimes!=0)
	    	  {
                 EMN_MAC_DisNetwork();
			     rfnwk_state= RF_NWK_REGING1;
	    	   }  
	      }else if( rfnwk_state!= RF_NWK_REGING1)
	        {
       	      EMN_SendMsg.dest_id= EMN_APL_TASK;
              EMN_SendMsg.msg_id =EMN_NWK_START_GET_DATA;
              SYS_SendMsg(&EMN_SendMsg);
		      upload_buffer.state=WRITTING;
	        }
#endif		  
	  }
#endif 	  
	  if( SYS_Timers.timeout != 0)
	  {
//        m = SYS_Timers.seting;
		n = SYS_Timers.timeout;
		for(i =0 ;i< SYS_TIME_MAX_NUM ; i++)
		{
    	//	if((m>>i)&& (n>>i) )  // timeout happened.
    		if((n>>i)&0x01 )  // timeout happened.
    		{
				//stop specified timer
				SYS_DisableTimerISR();				
				SYS_Timers.value[i]= 0;
				SYS_Timers.fastfun[i]= NULL;
				SYS_Timers.seting  = (~(0x01<<i))& SYS_Timers.seting;;
				SYS_Timers.timeout = (~(0x01<<i))& SYS_Timers.timeout;
				SYS_EnableTimerISR();

             // directly handle timeout event, not push it into message queue.
			  msg.dest_id = SYS_Timers.taskID[i];
			  msg.msg_id = EMN_TIMEOUT;
			  msg.data[0] = SYS_Timers.msgID[i];
			  
			  SYS_Timers.taskID[i] = EMN_TASK_MAX;
			  SYS_Timers.msgID[i] = EMN_PRIIMITIVE_MAX;
			  #if 0
              SYS_TaskTable[msg.dest_id].task_fun(&msg);		  
			  #else
              SYS_SendMsg(&msg);
			  #endif
        
    		}
	  	}
	  }
		}
			_lwsem_post(&NetworkRfSem); 
	dicor_waite_rf(RFNET_EVENT,50);
  } 
}


/*----------------------------------------------------------
 *
 *     the implementaion about heap mangament
 *
 *-----------------------------------------------------------*/
 #if  ENABLE_HEAP_MANAGER 
static  UINT_8  Heap[SYS_HEAP_SIZE];
static 	UINT_32 HeapFlag[SYS_HEAP_FLAG_WORD_NUM];
static  UINT_8  HeapSize[SYS_HEAP_BLOCK_NUM];

typedef struct{
    UINT_8 old_index;
	UINT_8 new_indx;
}HEAP_HANDLE;
#define SYS_HEAP_FIX_NUM    (4)
static HEAP_HANDLE HeapFix[SYS_HEAP_FIX_NUM];
static UINT_8 HeapCnt;
void _SYS_HeapInit(void)
{
    UINT_8 i;
	for(i=0;i<SYS_HEAP_FLAG_WORD_NUM;i++)
	{
      HeapFlag[i]=0;	  
	}
	for(i=0; i< SYS_HEAP_BLOCK_NUM; i++)
	{
      HeapSize[i] = 0;  // set it to empty
	}

}
UINT_8* SYS_MEM_Alloc(UINT_8  size)
{
   UINT_8  i,max,num;
   UINT_32  tmp;
//count the number of requested block
   tmp = size/SYS_HEAP_BLOCK_SIZE;
   if(size%SYS_HEAP_BLOCK_SIZE)
   {
	  size = tmp;
      size++;
   }
//count the number of free block   
	i = 0;
	while( HeapFlag[0])
	{
	  i++;
	  HeapFlag[0]= HeapFlag[0] & ( HeapFlag[0]-1 );
	}  // i = the number of 1in HeapFlag
	
// no enough memory for this requestion	
   if((SYS_HEAP_BLOCK_NUM-i) <= size)//if =, fix is too difficult
	  return (NULL) ; 

   
   tmp = 0;
   for(i =0; i < size ;i++)
   {
     tmp &= 0x01<<i; 
   }
   for(i =0; i<(SYS_HEAP_BLOCK_SIZE-size);i++)
   {
     if(!((HeapFlag[0]>>i)&tmp))
      {
        return (&Heap[i*SYS_HEAP_BLOCK_SIZE]);
      }
   }
   return (NULL);  // MUST implement this in future 
 //fix heap for re-allocate, do it  
#if 0
  //step 1: find max len of bit0 string
   max =0
   for(i=0; i< SYS_HEAP_BITS_IN_FLAG; i++)
   {
      if((HeapFlag[0]>>i)&0x00000001)
      	{
      	  num = 0;
		  if(num>max)  { max = num, tmp = i;}
      	}else {  num++;	}
   }
   //step2 :re-arrenge 0 order
  // if((max+SYS_HEAP_FIX_NUM)< size)
  // 	return (NULL);
 
#endif    
 
}

void  SYS_MEM_Free(UINT_8*  free_p)
{
  UINT_8  index,size;
  UINT_32 tmp;
  if(((free_p- &Heap[0])%SYS_HEAP_BLOCK_SIZE != 0)
  	    && (free_p < &Heap[0])
  	    &&(free_p >&Heap[SYS_HEAP_SIZE]))
  {
     index = (free_p - &Heap[0])/SYS_HEAP_BLOCK_SIZE;
	 size = HeapSize[index];
	 for(i =0; i < size ;i++)
     {
       tmp &= 0x01<<i;	   
     }
	 tmp <<= index;
	 {
	 	// IMPORTANT : the the following only implement 32 block.
       if(HeapFlag[0]&tmp)
       {
         HeapFlag &= (~tmp);
		 HeapSize[index]= 0;
       }else
       	{
          SYS_Error(3);
       	}
	 }
	  
  }else// the input parameter is wrong
  	{
       SYS_Error(4);
  	}
  
}
#endif

/* EOF */

