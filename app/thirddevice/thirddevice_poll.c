/**********************************************************************/ /*!
*
* @file modbus_poll.c
*
* @author Younger
*
* @date 2012-04-06
*
* @version 0.1.8.0
*
* @brief 第三方设备轮询相关操作
*
***************************************************************************/

#include <string.h>
#include "dicor.h"
#include "thirddevice_poll.h"
#include "third_device.h"


extern _mem_pool_id _user_pool_id;

static uint_8 ThirdDevID = 0;

THIRDDEVICELINK* ThirdDevLinkRoot = NULL;
extern THIRD_DEVICE_DATA_ST* g_pThirdDeviceData;


//创建设备链表
static THIRDDEVICELINK* create_third_dev_link_node(thirddev_poll_list_entry_t* poll_list, Third_Dev_Poll_Timer* poll_timer)  
{  
    THIRDDEVICELINK* pLinkNode = NULL;  
    
    pLinkNode = (THIRDDEVICELINK*)_mem_alloc_zero_from(_user_pool_id, sizeof(THIRDDEVICELINK));  
    if (NULL == pLinkNode)
	{
		printf("ERROR create sub link node\n");
		return NULL;
	}
  
    memset(pLinkNode, 0, sizeof(THIRDDEVICELINK));  
    pLinkNode->devid = ThirdDevID;
  	pLinkNode->poll_list = poll_list;
  	pLinkNode->poll_timer = poll_timer;
    ThirdDevID++;
    return pLinkNode;  
}  



//设备链表插入设备数据 
static uint_8 insert_data_third_dev_link(THIRDDEVICELINK** ppLinkNode, thirddev_poll_list_entry_t* poll_list, Third_Dev_Poll_Timer* poll_timer)  
{
	THIRDDEVICELINK* pNode;
	THIRDDEVICELINK* pIndex;
	
    if(NULL == ppLinkNode)
	{
        return FALSE;  
	}
	
	//根节点为空
    if(NULL == *ppLinkNode)
	{  
		pNode = create_third_dev_link_node(poll_list, poll_timer);  
		if (NULL == pNode)
		{
			printf("pNode is NULL\n");	
			return FALSE;
		} 
        *ppLinkNode = pNode;  
        (*ppLinkNode)->next = NULL;  
        return TRUE;  
    }  
    
    if ((poll_list==NULL) || (poll_timer==NULL))
    {
    	printf("poll_list or poll_timer is NULL\n");	
		return FALSE;
    }
    
    pNode = *ppLinkNode;
    if (pNode->poll_list == NULL)
    {
    	pNode->poll_list = poll_list;
    	pNode->poll_timer = poll_timer;
    	return TRUE;
    }
    
	pNode = create_third_dev_link_node(poll_list, poll_timer);  
	if (NULL == pNode)
	{
		printf("pNode is NULL\n");	
		return FALSE;
	} 
 	pIndex = *ppLinkNode;	 
    while(NULL != pIndex->next)
	{
        pIndex = pIndex->next;  
	}
    pNode->next = NULL;  
    pIndex->next = pNode;  
    
    return TRUE;  
}  


//插入第三方设备根节点，创建链表
void Init_ThirdDevLinkNode(void)
{
	void* ptr = NULL;
	if (insert_data_third_dev_link(&ThirdDevLinkRoot, NULL, NULL))
	{
		printf("Create root node succeed!\n");
	}
	else
	{
		printf("Create root node fail!\n");
	}
}



//将第三方设备插入链表
uint_8 InsetThirdDeviceToLink(thirddev_poll_list_entry_t* poll_list, Third_Dev_Poll_Timer* poll_timer)
{
	if (insert_data_third_dev_link(&ThirdDevLinkRoot, poll_list, poll_timer))
	{
		printf("insert node succeed!\n");
	}
	else
	{
		printf("insert node fail!\n");
	}
	return TRUE;
}

//将所有任务当前时间累加
void Third_Device_Timer_Inc(void)
{
	THIRDDEVICELINK* pNode;
	pNode = ThirdDevLinkRoot;
	
	if (pNode->next == NULL && pNode->poll_timer == NULL)
	{
		return;
	}
	while (pNode != NULL)
	{
		pNode->poll_timer->cur++;
		pNode = pNode->next;
	}
	 //	printf("RESET CC2530");
	     
	    //  fclose(rs485_dev);
	     // clearerr(rs485_dev);
	     // rs485_dev = NULL;
	     //	fflush( rs485_dev);
	     //	fclose( (pointer) (IO_SERIAL_NON_BLOCKING) );  
	     //  N_RESET_CC2530();
	       
	      //  uart1_dev  = fopen(UART1_CHANNEL, NULL );  
	       
	       // fflush(uart1_dev);
	       // N_RESET_CC2530();
	    
	    
	    //	_time_delay(3000);
	     
	      // rs485_init();
	      // RESET_CC2530();
}

//轮询所有的第三方设备
void Third_Device_Poll(void)
{
	THIRDDEVICELINK* pNode = ThirdDevLinkRoot;
	
	if (pNode->next == NULL && pNode->poll_timer == NULL)
	{
		return;
	}
	while (pNode != NULL)
	{
		if (pNode->poll_timer->cur >= pNode->poll_timer->top)
		{
			pNode->poll_timer->cur = 0;
			if (pNode->poll_list->service != NULL)
			{
				g_pThirdDeviceData = (THIRD_DEVICE_DATA_ST*)(pNode->poll_list->service_param);
				pNode->poll_list->service(pNode->poll_list->service_param);
				_time_delay(200);
			}
		}
		pNode = pNode->next;
	}
}


//注册第三方设备的定时器
Third_Dev_Poll_Timer* register_third_device_timer(uint_32 period)
{
	Third_Dev_Poll_Timer* pThirdDeviceTimer;

	
	pThirdDeviceTimer = (Third_Dev_Poll_Timer *) _mem_alloc_zero_from(_user_pool_id, sizeof(Third_Dev_Poll_Timer));
	if (pThirdDeviceTimer == NULL)
	{
		printf("error when mem alloc Third_Dev_Poll_Timer buf\r\n");	
		return NULL;
	}
	memset(pThirdDeviceTimer, 0, sizeof(Third_Dev_Poll_Timer));
	pThirdDeviceTimer->cur = 0;
	pThirdDeviceTimer->top = period;
	return pThirdDeviceTimer;
}

//注册第三方设备的轮询函数
thirddev_poll_list_entry_t* register_third_device_poll(thirddev_poll_service_t service, void *service_param)
{
	thirddev_poll_list_entry_t* pThirdDevicepoll;
	pThirdDevicepoll = (thirddev_poll_list_entry_t *) _mem_alloc_zero_from(_user_pool_id, sizeof(thirddev_poll_list_entry_t));
	if (pThirdDevicepoll == NULL)
	{
		printf("error when mem alloc thirddev_poll_list_entry_t buf\r\n");	
		return NULL;
	}
	memset(pThirdDevicepoll, 0, sizeof(thirddev_poll_list_entry_t));
	pThirdDevicepoll->service = service;
	pThirdDevicepoll->service_param = service_param;
	return pThirdDevicepoll;
}
