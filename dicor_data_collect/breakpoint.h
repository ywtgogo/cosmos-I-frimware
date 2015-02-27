#ifndef __break_point_h__
#define __break_point_h__
/**********************************************************************
* 
* Copyright (c) 2010 convertergy ;
* All Rights Reserved
* FileName: breakpoint.h 
* Version : 
* Date    : 
*
* Comments:
*
*   This file contains definitions for the upload MG's data to data center
*
*************************************************************************/
#include <message.h>
#include "dicor_network_apl.h"
#include "dicor.h"
#include "mutex.h"
#include "dicor_upload.h"



//单个文件的最大文件大小
#define FILEMAXSIZE		2*1024*1024
//#define FILEMAXSIZE		1024




typedef struct
{
   uint_16 YEAR;	/* 1970 - 2099 */
   uint_16 MONTH;	/* 1 - 12 */
   uint_16 DAY;		/* 1 - 31 (depending on month) */
   uint_16 HOUR;	/* 0 - 23 */
   uint_16 MINUTE;	/* 0 - 59 */
   uint_16 SECOND;	/* 0 - 59 */
}SAVE_TIME;

typedef struct 
{
	uint_8    type;
	uint_8    dicor_id[DICOR_ID_LEN];
	uint_8	  dumy[1];
	SAVE_TIME save_time;
	uint_16   num;
	uint_8    didi_data[DICOR_UPLOAD_BUFFER_SIZE];
} SAVE_DATA_BUFFER;

typedef struct 
{
  UINT_16 readindex;
  UINT_16 writeindex;
  FILE_PTR fd_read_ptr;
  FILE_PTR fd_write_ptr;
  UINT_8 Fileexist;
} BREAK_POINT_INFO, _PTR_  BREAK_POINT_INFO_PTR;


extern SAVE_DATA_BUFFER  Save_Data_Buff;
extern BREAK_POINT_INFO BreakPointInfo;

UINT_8 SaveData2SdCard(DICOR_PACKET1 * data0);
INT_8 BreakpointContinuingly(SAVE_DATA_BUFFER * save_data_buff,
			     uint_32 * offset);
uint_8 BreakpointInit(void);
UINT_8 SaveFileCounter(void);
void WriteTest(void);


#endif
/* EOF */
