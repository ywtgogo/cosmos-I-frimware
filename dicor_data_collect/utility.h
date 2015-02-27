#ifndef __utility_h__
#define __utility_h__
#include "mqx.h"
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

void strupr(char *s);
unsigned char ahextoi(const char* hexstr);
boolean get_dotted_address (char_ptr name, uint_32_ptr address);
uchar cutstr(const char* sour, char* dest, char c);
unsigned short UT_calcrc16(unsigned short crclast, unsigned char *buf, unsigned short len);
void print_sys_time(void);
void print_rtc_time(void);

#endif
/* EOF */
