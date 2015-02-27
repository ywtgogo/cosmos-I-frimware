
/**********************************************************************/ /*!
*
* @file thirddevice_poll.h
*
* @author Younger
*
* @date 2012-04-06
*
* @version 0.1.13.0
*
* @brief 
*
***************************************************************************/

#ifndef _THIRDDEVICE_POLL_H_

#define _THIRDDEVICE_POLL_H_




typedef void (* thirddev_poll_service_t)(void* service_param);



typedef struct
{
	uint_32 cur;
	uint_32 top;	
} Third_Dev_Poll_Timer;

typedef struct
{
    thirddev_poll_service_t service;
    void *service_param;
}thirddev_poll_list_entry_t;

typedef struct _THIRD_DEVICE_LINK
{
	uint_8 devid;
	char dev_name[7];
	thirddev_poll_list_entry_t* poll_list; 
	Third_Dev_Poll_Timer* poll_timer;

	struct _THIRD_DEVICE_LINK* next;
}THIRDDEVICELINK;


uint_8 InsetThirdDeviceToLink(thirddev_poll_list_entry_t* poll_list, Third_Dev_Poll_Timer* poll_timer);
void Third_Device_Poll(void);
void Third_Device_Timer_Inc(void);
void Init_ThirdDevLinkNode(void);
Third_Dev_Poll_Timer* register_third_device_timer(uint_32 period);
thirddev_poll_list_entry_t* register_third_device_poll(thirddev_poll_service_t service, void *service_param);
#endif
