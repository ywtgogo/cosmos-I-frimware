#ifndef __dicor_update_h__
#define __dicor_update_h__
/**********************************************************************
* 
* Copyright (c) 2011 convertergy ;
* All Rights Reserved
* FileName: dicor_update.h 
* Version : 
* Date    : 
*
* Comments:
*
*   This file contains definitions for the upload MG's data to data center
*
*************************************************************************/
#include <message.h>

#define LOCALVERSIONFILENAME	"d:\\version.txt"
#define REMOTEVERSIONFILENAME	"version.txt"
#define TFTPHOSTIPADDR			IPADDR(115,29,192,154)

//更新时间区间范围[UPDATECHKSTARTTIME,UPDATECHKENDTIME)
#define UPDATECHKSTARTTIME		1
#define UPDATECHKENDTIME		5

void Dicor_Reboot(void);
void Dicor_Update(void);
 int_8 AnalyseVersionFile(const char * filename);
int_32  Shell_UpgradeDiCor(int_32 argc, char_ptr argv[]);

#endif
/* EOF */
