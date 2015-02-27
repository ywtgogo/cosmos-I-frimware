#ifndef __rs485_h__
#define __rs485_h__
#include "mqx.h"
#include "fio.h"
#include "modbus.h"
#include <serial.h>
/**********************************************************************
* 
* Copyright (c) 2011 convertergy ;
* All Rights Reserved
* FileName: utility.h 
* Version : 
* Date    : 
*
* Comments:
*
*   This file contains definitions for the upload MG's data to data center
*
*************************************************************************/
void rs485_init(void);
_mqx_int rs485_read(uchar* data_buffer);
void rs485_write(uchar* data_buffer, _mqx_int len);
extern uchar  RS485_Data_Buffer[1024]; //½«buffer¸ÄÎª512
void rs485_clear_buf(void);
void rs485_clear_stats(void);
#endif
/* EOF */
