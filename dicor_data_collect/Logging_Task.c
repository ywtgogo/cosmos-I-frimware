/*************************************************************
*   
* Copyright (c) convertergy ;
* All Rights Reserved
* FileName: Logging_Task.c
* Version : 
* Date    : 2011-12-08
* ower: 	Alex
*
* Comments: 将Log信息打印到SD卡中保存
*
*
***************************************************************/

#include <string.h>
#include <mfs.h>
#include <watchdog.h>
#include "logging_public.h"
#include "dicor_upload.h"
#include "logging_private.h"

_pool_id  log_pool=NULL;
LWSEM_STRUCT Logging_init_sem;
DICOR_LOG* pDiCorLog;

extern _mem_pool_id _user_pool_id;

extern MQX_FILE_PTR filesystem_handle;
extern void Dicor_Reboot(void);

void dicor_get_logtime(DATE_STRUCT * date)
{
	TIME_STRUCT time;

	_time_get(&time);
	//将互联网的格林时间转换成北京时间
	time.SECONDS += 8*3600;
	_time_to_date(&time, date);
}


void LogInit(void)
{
	_task_id taskid;
	//先申请内存
	pDiCorLog = (DICOR_LOG*) _mem_alloc_zero_from(_user_pool_id, sizeof(DICOR_LOG));
    if (pDiCorLog == NULL)
    {
    	printf("error when mem alloc\r\n");	
    	Dicor_Reboot();
    }
    pDiCorLog->fd_log_ptr = NULL;
    _lwsem_create(&pDiCorLog->writesem,	1);  
	_lwsem_create(&Logging_init_sem,0);
	taskid = _task_create(0, DICOR_LOGGING_TASK, 0); 
	if (taskid == MQX_NULL_TASK_ID)
	{
		printf("Could not create DICOR_LOGGING_TASK \n");
	}
	_lwsem_wait(&Logging_init_sem);	
}


void PrintLog(char_ptr msg) 
{
	LOG_MESSAGE_PTR msg_ptr;

	if (log_pool) 
	{
		msg_ptr = (LOG_MESSAGE_PTR)_msg_alloc(log_pool);

		if (msg_ptr != NULL) 
		{
			msg_ptr->HEADER.TARGET_QID = _msgq_get_id(0, LOG_QUEUE);
			strncpy(msg_ptr->MESSAGE,msg,LOG_MESSAGE_SIZE);
			_msgq_send(msg_ptr);
		}
	}
}


static void dicor_get_logyeardir(DATE_STRUCT * date, char* name)
{
	sprintf(name, "d:\\dicorlogs\\%04d", date->YEAR);	
}

static void dicor_get_logmonthdir(DATE_STRUCT * date, char* name)
{
	sprintf(name, "d:\\dicorlogs\\%04d\\%02d", date->YEAR, date->MONTH);	
}

static void dicor_get_logfilename(DATE_STRUCT * date, char* name)
{
	sprintf(name, "d:\\dicorlogs\\%04d\\%02d\\log_%04d-%02d-%02d.txt", date->YEAR, date->MONTH, date->YEAR, date->MONTH, date->DAY);	
}


static MQX_FILE_PTR logging_getfileptr(void)
{
	uint_32 error;
	DATE_STRUCT date;
	dicor_get_logtime(&date);

	//先创建目录
	dicor_get_logyeardir(&date, pDiCorLog->filename);
	error = ioctl(filesystem_handle, IO_IOCTL_CREATE_SUBDIR, (uint_32_ptr) pDiCorLog->filename);
	dicor_get_logmonthdir(&date, pDiCorLog->filename);
	error = ioctl(filesystem_handle, IO_IOCTL_CREATE_SUBDIR, (uint_32_ptr) pDiCorLog->filename);
	dicor_get_logfilename(&date, pDiCorLog->filename);
	
	//文件不存在创建文件
	pDiCorLog->fd_log_ptr = fopen(pDiCorLog->filename, "a+");
	if (pDiCorLog->fd_log_ptr == NULL )
	{
		printf("Error open file\r\n");
	}
	return pDiCorLog->fd_log_ptr;
}
	
	
void dicor_logging_task(uint_32 param)
{
	_queue_id         log_qid;
	LOG_MESSAGE_PTR   msg_ptr;
	MQX_FILE_PTR      log_fp;
	
	if (_watchdog_create_component(BSP_TIMER_INTERRUPT_VECTOR,Wacthdog_Error)!= MQX_OK) 	
	{ 
		printf("lost logging task create watchdog failed !");
		Dicor_Reboot();
	}
	
	_watchdog_start(60*60*1000);
	
	// create a pool of logging messages   
	log_pool = _msgpool_create(sizeof(LOG_MESSAGE), LOG_POOL_SIZE, 0, 0);
	// open a message queue to receive log message on
	log_qid = _msgq_open(LOG_QUEUE, 0);
	  
	// signal that initialization is complete
	_lwsem_post(&Logging_init_sem);
	_watchdog_stop();
	_watchdog_start(60*60*1000);
	
	while (TRUE) 
	{
		// wait for a message
		msg_ptr = _msgq_receive(log_qid, 0);
		if (msg_ptr) 
		{
			// Open the log file and position to the end
			log_fp = logging_getfileptr();
			if (log_fp) 
			{
				_lwsem_wait(&pDiCorLog->writesem);
				// fseek(log_fp,0,IO_SEEK_END);
				do 
				{
					// Write the message to the log file
					write(log_fp,msg_ptr->MESSAGE, strlen(msg_ptr->MESSAGE));

					// Return the message back to the message pool
					_msg_free(msg_ptr);

					// check for another message
					msg_ptr = _msgq_poll(log_qid);
				} while (msg_ptr != NULL);

				// close the file
				fclose(log_fp);
				pDiCorLog->fd_log_ptr = NULL;
				_lwsem_post(&pDiCorLog->writesem);
			}
			_watchdog_start(60*60*1000);
		}
	}
}


/* EOF */