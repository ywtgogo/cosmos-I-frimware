#ifndef _network_os_h_
#define _network_os_h_
/*************************************************************
* 
* 	Copyright (c) 2010  Convetergy;
* 	All Rights Reserved
* 	FileName: smc_os.h
*	Version : 
* 	Date    : 2010-11-17
* 	Ower   : peter li
*
* Comments:
*   The main configuration file for entire network implementation.
*   for this project, all definitions in this file is globle
*
*************************************************************/
#include "network_frame.h"

#define ENABLE_HEAP_MANAGER      0
#define ENBALE_LLC_TASK          0


/******************************************************
 * 
 *                definitions of memory function
 *
 *******************************************************/
#if  ENABLE_HEAP_MANAGER      
 #define SYS_HEAP_SIZE          (512)
 #define SYS_HEAP_BLOCK_SIZE    (16)    //16 bytes in one block
 #define SYS_HEAP_BLOCK_NUM     (SYS_HEAP_SIZE/SYS_HEAP_BLOCK_SIZE)
 #define SYS_HEAP_BITS_IN_FLAG  (32)
 #define SYS_HEAP_FLAG_WORD_NUM (SYS_HEAP_BLOCK_NUM/SYS_HEAP_BITS_IN_FLAG)
#endif

/******************************************************
 * 
 *                definitions of timer function
 *
 *******************************************************/
#define SYS_TIMER0         (0)  //MAC layer
#define SYS_TIMER1         (1)  //NWk layer
#define SYS_TIMER2         (2)  //PHY layer
#define SYS_TIMER3         (3)  //APL lyaer
#define SYS_TIMER4         (4)  //APL lyaer.harvester
#define SYS_TIME_MAX_NUM   (SYS_TIMER4+1)



typedef  struct
{
 UINT_8    id;
 UINT_16   value;
 UINT_8    msgID;
 UINT_8    taskID;
 void	   (*fastfun)(void); 

}SYS_TIMER;

extern void SYS_TimerInit(void);
#if 	EMN_EMT_ONLY 
extern void SYS_TimersISR(void *dummy);
#else
extern void SYS_TimersISR(void);
#endif 
extern UINT_8  SYS_CkeckTimer(SYS_TIMER *timer);
extern void SYS_StopTimer(SYS_TIMER *timer);
extern void SYS_StartTimer(SYS_TIMER *timer);
extern UINT_8 SetSubnet,SetDev,SetType;


/*************************************************************
 *
 *                        definitions of  message function and task managment
 *
 ********************************************** ***************/
 //task ID definition depends on task initialization order
typedef enum
{
 EMN_PHY_TASK = 0,
 EMN_MAC_TASK = 1, 
 EMN_NWK_TASK = 2,
 EMN_APL_TASK = 3,
 EMN_HAVST_TASK = 4,
 EMN_TASK_MAX
}SYS_TASK_ID;


#define   SYS_MSG_OK         (0)
#define   SYS_MSG_FULL       (1)
#define   BYTE_IN_POINT   sizeof(UINT_8 *)  
typedef struct
{
 SYS_TASK_ID dest_id;
 SYS_MESSAGE_ID  msg_id;
 UINT_8  data[4];  
 }SYS_MESSAGE;

typedef struct 
{
   SYS_TASK_ID task_id;
   UINT_8  pri;
   UINT_8  timeslice;   // 0 :default 
   void	(*task_fun)(SYS_MESSAGE*);
   void (*task_init)(void); 
}SYS_TASK_TEMPLATE;


extern UINT_8  SYS_SendMsg(SYS_MESSAGE *msg);
extern void SYS_MainFunction(void);

#if EMN_DEBUG
#define SYS_ResetTrg()  trigger_ClrVal()
#define SYS_SendTrg()   {trigger_ClrVal();  trigger_SetVal();}
#else
#define SYS_ResetTrg()   
#define SYS_SendTrg()    

#endif


#endif

