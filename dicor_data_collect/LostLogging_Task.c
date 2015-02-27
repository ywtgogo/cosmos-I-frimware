/*************************************************************
* 
* Copyright (c) convertergy ;
* All Rights Reserved
* FileName: LostLogging_Task.c
* Version : 
* Date    : 2011-12-09
* ower: 	Alex
*
* Comments: 将丢包情况保存到SD卡中保存起来
*
*
***************************************************************/

#include <string.h>
#include <mfs.h>
#include <watchdog.h>
#include "lostlogging_public.h"
#include "dicor.h"
#include "lostlogging_private.h"


_pool_id  lost_pool=NULL;	//丢包统计
LWSEM_STRUCT LostLogging_init_sem;
LOST_LOG* pLostLog;

extern _mem_pool_id _user_pool_id;


extern void dicor_get_logtime(DATE_STRUCT * date);
extern MQX_FILE_PTR filesystem_handle;
extern uint_8 ChkSaveLog(DATE_STRUCT* date);
extern void Dicor_Reboot(void);

void LostLogInit(void)
{
	_task_id taskid;
	//先申请内存
	pLostLog = (LOST_LOG*) _mem_alloc_zero_from(_user_pool_id, sizeof(LOST_LOG));
    if (pLostLog == NULL)
    {
    	printf("error when mem alloc\r\n");	
    	Dicor_Reboot();
    } 
    pLostLog->fd_ptr = NULL;
    _lwsem_create(&pLostLog->writesem, 1);
	_lwsem_create(&LostLogging_init_sem, 0);
	taskid = _task_create(0, DICOR_LOSTLOGGING_TASK, 0); 
	if (taskid == MQX_NULL_TASK_ID)
	{
		printf("Could not create DICOR_LOSTLOGGING_TASK \n");
	//	Dicor_Reboot();
	}
	_lwsem_wait(&LostLogging_init_sem);

	
}



void PrintLost(char_ptr msg) 
{
	LOST_MESSAGE_PTR msg_ptr;

	if (lost_pool) 
	{
		msg_ptr = (LOST_MESSAGE_PTR)_msg_alloc(lost_pool);

		if (msg_ptr != NULL) 
		{
			msg_ptr->HEADER.TARGET_QID = _msgq_get_id(0, LOST_QUEUE);
			strncpy(msg_ptr->MESSAGE,msg,LOST_MESSAGE_SIZE);
			_msgq_send(msg_ptr);
	  	}
	}
}


static void dicor_get_lostlogfilename(DATE_STRUCT * date, char* name)
{
	sprintf(name, "d:\\dicorlogs\\%04d\\%02d\\lost_%04d-%02d-%02d.txt", date->YEAR, date->MONTH, date->YEAR, date->MONTH, date->DAY);	
}


static MQX_FILE_PTR lostlogging_getfileptr(void)
{
	DATE_STRUCT date;

	dicor_get_logtime(&date);
  	dicor_get_lostlogfilename(&date, pLostLog->filename);
  	pLostLog->fd_ptr = fopen(pLostLog->filename, "a+");
	if (pLostLog->fd_ptr == NULL )
	{
		printf("Error open file\r\n");
	}
	return pLostLog->fd_ptr;
}


	
void dicor_lostlogging_task(uint_32 param)
{
	_queue_id         lost_qid;
	LOST_MESSAGE_PTR   msg_ptr;
	MQX_FILE_PTR      lost_fp;
	DATE_STRUCT date;
	
	if (_watchdog_create_component(BSP_TIMER_INTERRUPT_VECTOR,Wacthdog_Error)!= MQX_OK) 	
	{ 
		printf("lost logging task create watchdog failed !");
		Dicor_Reboot();
	}
	
	_watchdog_start(1*60*1000);
	
	// create a pool of logging messages   
	lost_pool = _msgpool_create(sizeof(LOST_MESSAGE), LOST_POOL_SIZE, 0, 0);

	// open a message queue to receive log message on
	lost_qid = _msgq_open(LOST_QUEUE, 0);
	 
	// signal that initialization is complete
	_lwsem_post(&LostLogging_init_sem);  
	
	_watchdog_stop();
	
	if (ChkSaveLog(&date))//[6,18)丢包保存
	{
		_watchdog_start(60*60*1000);
	}


	while (TRUE) 
	{
		// wait for a message
		msg_ptr = _msgq_receive(lost_qid, 0);

		if (msg_ptr) 
		{
			// Open the log file and position to the end
			lost_fp = lostlogging_getfileptr();
			
			if (lost_fp) 
			{
				_lwsem_wait(&pLostLog->writesem);
				// fseek(log_fp,0,IO_SEEK_END);
				do 
				{
					// Write the message to the log file
					write(lost_fp,msg_ptr->MESSAGE, strlen(msg_ptr->MESSAGE));
					// Return the message back to the message pool
					_msg_free(msg_ptr);
					// check for another message
					msg_ptr = _msgq_poll(lost_qid);
				} while (msg_ptr != NULL);
				// close the file
				fclose(lost_fp);
				pLostLog->fd_ptr = NULL;
				_lwsem_post(&pLostLog->writesem);
			} 
			
			if (ChkSaveLog(&date))//[6,18)丢包保存
			{
				_watchdog_start(60*60*1000);
			}
			else
			{
				_watchdog_stop();
			}
			
		}
	}
}


/* EOF */