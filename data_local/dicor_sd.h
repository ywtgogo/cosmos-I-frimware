/**********************************************************************
* 
* Copyright (c) 2010 convertergy;
* All Rights Reserved
*
* FileName: dicor_sd.h
* Version :
* Date    : 
*
* Comments:
*
*************************************************************************/

#ifndef _dicor_sd_h_
#define _dicor_sd_h_

#define SD_PWR_ON()			(sdpwr_gpio_ptr->PORTAN &= 0xFB)
#define SD_PWR_OFF()		(sdpwr_gpio_ptr->PORTAN |= 0x04)


extern MQX_FILE_PTR filesystem_handle;

void SDcard_Install(void);


#endif
