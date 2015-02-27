/**********************************************************************
* 
* Copyright (c) 2010 convertergy;
* All Rights Reserved
*
* FileName: dicor_Shell_Commands.c
* Version : 
* Date    : May-17-2010
* Onwer   : Peter Li
*
* Comments:   this file contains all kinds of implementation about all command. they are used by 
* the user who are logging by our telnet server.
*   
*
*************************************************************************/

#include <string.h>
#include <mqx.h>
#include <bsp.h>
#include "dicor__shell_commands.h"
#include <Sh_rtcs.h>

DICOR_PARAS dicor_Params = {0};

/* Event string table */
static char* dicor_Event_Tab[]={
   "write mg UID",
   "write etu  UID",
} ; 

 const SHELL_COMMAND_STRUCT dicor_telnet_commands[] = {
//////////
//system command 
   {  "displaylog", Shell_displaylog},
   {  "clearlog",   Shell_clearlog},
    { "ipconfig",  Shell_ipconfig },         
#if RTCSCFG_ENABLE_ICMP      
    { "ping",      Shell_ping },
#endif
// the feature from system shell   
   {  "exit",       Shell_exit },  
   {  "help",       Shell_help }, 
   {  "?",          Shell_command_list },      
#if 0    
//////////
    { "gethbn",    Shell_get_host_by_name }, 
    { "getrt",	   Shell_getroute },

    { "netstat",    Shell_netstat },
    { "pause",     Shell_pause },
    { "arpadd",    Shell_arpadd },
    { "arpdel",     Shell_arpdel },
    { "arpdisp",   Shell_arpdisp },
#if RTCSCFG_ENABLE_NAT
    { "dnat",      Shell_dnat },     
#endif
#if RTCSCFG_ENABLE_UDP
    { "exec",       Shell_exec },              
#endif
    { "gate",	   Shell_gate },
#if RTCSCFG_ENABLE_UDP
    { "load",      Shell_load },      
#endif
#if RTCSCFG_ENABLE_UDP
    { "sendto",    Shell_sendto },
#endif
    { "telnetd",   Shell_Telnetd },
#endif 
   { NULL,          NULL } 
};
   
void dicor_telnetd_shell_fn(void) 
{  
  Shell(dicor_telnet_commands,NULL);
}


static void print_usage_simple (boolean shorthelp, const char_ptr argv, 
	                                                    const char_ptr longhelp) 
{
  if (shorthelp)  
  {
    printf("%s\n", argv);
  } 
  else  
  {
    printf("Usage: %s\n", argv);
    printf("   %s\n", longhelp);
  }
}

/*
 *  Erase log. Clears all entries.
 */
int_32  Shell_clearlog(int_32 argc, char_ptr argv[] )
{
   boolean           print_usage, shorthelp = FALSE;
   int_32            return_code = SHELL_EXIT_SUCCESS;
   
   int i;

	
   print_usage = Shell_check_help_request(argc, argv, &shorthelp );

   if (!print_usage)  
   {
      if (argc > 1) 
      {
         printf("Error, invalid number of parameters\n");
         return_code = SHELL_EXIT_ERROR;
         print_usage=TRUE;
      } 
      else 
      {		
		for(i=0;i<MAX_QUEUE;i++)
		{
		   dicor_Params.History[i].eventptr=NULL;
		   dicor_Params.History[i].time.SECONDS=0;
		   dicor_Params.History[i].time.MILLISECONDS=0;
		}  
		dicor_Params.HistoryIndex=0;
		printf("Log Cleared\n");
      	
      }
   }
   return return_code;
} 
/*
 *  
 */
int_32  Shell_displaylog(int_32 argc, char_ptr argv[] )
{
   boolean           print_usage, shorthelp = FALSE;
   int_32            return_code = SHELL_EXIT_SUCCESS;
      	
 	TIME_STRUCT time;
 	DATE_STRUCT date;
 	DICOR_HIST_PTR hist;
	int i;

	
   print_usage = Shell_check_help_request(argc, argv, &shorthelp );

   if (!print_usage)  
   {
      if (argc > 1) 
      {
         printf("Error, invalid number of parameters\n");
         return_code = SHELL_EXIT_ERROR;
         print_usage=TRUE;
      } 
      else 
      {

		    _time_get_elapsed(&time);
	    	    _time_to_date(&time,&date);   
		    printf("Time Since Bootup: %02d:%02d:%02d\n",date.HOUR,date.MINUTE,date.SECOND);    
		
		    /* Print out last MAX_QUEUE events */
		    for(i = (MAX_QUEUE - 1);i >= 0;i--)
		    {
           hist = &(dicor_Params.History[(i+dicor_Params.HistoryIndex)%MAX_QUEUE]);
		       if(NULL != hist->eventptr) {
              _time_to_date(&hist->time,&date);
              printf("%02d:%02d:%02d %s\n", date.HOUR,date.MINUTE,date.SECOND,hist->eventptr);
		       }
		    }   	
      	
      }
   }
   
   if (print_usage)  
   {
      print_usage_simple (shorthelp, argv[0], "Display event log");
   }
   return return_code;
} 
/*
 *  Set up initial parameters
 */
void dicor_InitializeParameters(void) 
{
   int i;
   /* Clear the log */
	 for(i=0;i<MAX_QUEUE;i++)
	 {
	    dicor_Params.History[i].eventptr=NULL;
	    dicor_Params.History[i].time.SECONDS=0;
	    dicor_Params.History[i].time.MILLISECONDS=0;
	 }  
   dicor_Params.HistoryIndex=0;
 }


void dicor_telenet_Initialize(void)
{
   
   /* Initialize parameters for our system */
   dicor_InitializeParameters();
}


