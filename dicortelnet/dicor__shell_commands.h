#ifndef __dicor_shell_commands_h__
#define __dicor_shell_commands_h__

/**********************************************************************
* 
* Copyright (c) 2010 convertergy;
* All Rights Reserved
*
*  FileName: dicor_Shell_Commands.h
*  Version : 
*  Date    : May-17-2010
*  Owner  : Peter li 
* Comments:
*
*   
*
*************************************************************************/
#include <shell.h>
#include <mqx.h>

extern int_32 Shell_displaylog(int_32 argc, char_ptr argv[] ); 
extern int_32 Shell_clearlog(int_32 argc, char_ptr argv[] ); 
extern void dicor_telnetd_shell_fn(void) ;
#define MAX_QUEUE            3
typedef struct  {
   char_ptr       eventptr;
   TIME_STRUCT    time;
} DICOR_HIST, * DICOR_HIST_PTR;

typedef struct {

   _mqx_int    HistoryIndex;
   DICOR_HIST    History[MAX_QUEUE];
} DICOR_PARAS, * SEC_PARAMS_PTR;

 typedef enum {
   DIDI_WRITE_UID=0,
   DIDI_READ_UID
} DICOR_DIDI_OP;



extern DICOR_PARAS  DICOR_Params;
#endif

/* EOF*/