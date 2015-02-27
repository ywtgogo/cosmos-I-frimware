/**********************************************************************
* 
* Copyright (c) convertergy ;
* All Rights Reserved
*
*
* FileName: dicor_telnet_server.c
* Version : 
* Date    : 
* Ower: Peter Li
*
* Comments:
*
*************************************************************************/
#include <mqx.h>
#include <bsp.h>
#include <rtcs.h>
#include "dicor__shell_commands.h"
#define return_error_if(c) { if (c) return IO_ERROR; }
/*TASK*-----------------------------------------------------------------
*
* Function Name    : lw_telnet_server
* Returned Value   : none
* Comments  :  A light weight TCP/IP Telnet Server
*
*END*-----------------------------------------------------------------*/

_mqx_int dicor_telnet_server(void (_CODE_PTR_ shell)(void))
{
   uint_32        listensock, sock;
   sockaddr_in    addr;
   uint_32        error, option;
   FILE_PTR       sockfd, telnetfd;
   _mqx_uint      echoflag = IO_SERIAL_ECHO;

 
   /* Install device drivers for socket and telnet I/O */
   _io_socket_install("socket:");
   _io_telnet_install("telnet:");

   listensock = socket(PF_INET, SOCK_STREAM, 0);
   return_error_if(listensock == RTCS_SOCKET_ERROR);

   option = TELNETDCFG_BUFFER_SIZE;   
   error = setsockopt(listensock, SOL_TCP, OPT_TBSIZE, &option, sizeof(option));
   return_error_if(error != RTCS_OK);

   option = TELNETDCFG_BUFFER_SIZE;   
   error = setsockopt(listensock, SOL_TCP, OPT_RBSIZE, &option, sizeof(option));
   return_error_if(error != RTCS_OK);

   option = TELENETDCFG_TIMEWAIT_TIMEOUT;   
   error = setsockopt(listensock, SOL_TCP, OPT_TIMEWAIT_TIMEOUT, &option, sizeof(option));
   return_error_if(error != RTCS_OK);


   /* Bind the socket to the Telnet port */
   addr.sin_family      = AF_INET;
   addr.sin_port        = IPPORT_TELNET;
   addr.sin_addr.s_addr = INADDR_ANY;

   error = bind(listensock, &addr, sizeof(addr));
   return_error_if(error != RTCS_OK);

   /* Put the socket into listening mode */
   error = listen(listensock, 0);
   return_error_if(error != RTCS_OK);

   do {
      /* Wait for a connection */
      sock= accept(listensock, NULL, NULL);
      if (sock != RTCS_SOCKET_ERROR) {
         sockfd = fopen("socket:", (char_ptr)sock);
         if (sockfd != NULL) {
            telnetfd = fopen("telnet:", (char_ptr)sockfd);
            if (telnetfd != NULL) {

               ioctl(telnetfd, IO_IOCTL_SERIAL_SET_FLAGS, &echoflag);
                //set system stardard system I/O devices
               _io_set_handle(IO_STDIN, telnetfd);
               _io_set_handle(IO_STDOUT, telnetfd);

               (*shell)();
               /*
               ** Allow some time for queued data to go out.
               */
               RTCS_time_delay(100);
               fclose(telnetfd);
            }
            fclose(sockfd);
         }
         shutdown(sock, FLAG_CLOSE_TX);
      } 
   }  while (sock != RTCS_SOCKET_ERROR);
   shutdown(listensock, FLAG_CLOSE_TX);
   return RTCS_OK;
} 


#if 0
void dicor_GetTime() 
{
#if DICORCFG_ENABLE_SNTP
   
       _ip_address  ipaddr;
	TIME_STRUCT time;
	DATE_STRUCT date;   

   printf("\nGetting time from time server ... ");

   if (RTCS_resolve_ip_address(SNTP_SERVER,&ipaddr,NULL,0)) {
      /* Contact SNTP server and update time */
      if(SNTP_oneshot(ipaddr,1000)==RTCS_OK) 
      {
         printf("Succeeded\n");
      } 
      else 
      {
         printf("Failed\n");
      }
       
   }
   else 
   {
      printf("Failed\n");
   }
   
   /* Get elapsed time since bootup */
   _time_get(&time);
   _time_to_date(&time,&date);	
	printf("System Time: %02d/%02d/%02d %02d:%02d:%02d\n",
	   date.YEAR,date.MONTH,date.DAY,date.HOUR,date.MINUTE,date.SECOND);
#endif 	
}
#endif
/*-----------------------------------------------------
* 
* Task Name    : dicor_telnet_server_task
* Comments     :
*    This task opens a second I/O channel and prints a message
* to it.
*
*-----------------------------------------------------*/

void dicor_telnet_server_task ( uint_32 initial_data  )
{
   while (1) {  
        dicor_telnet_server(dicor_telnetd_shell_fn);
    }

}
//   TELNETSRV_init("Telnet_server", 
//   	 7, //priority, should be lower than TCP/IP, such as 7,8,9
//   	 1000,  //stack 
//   	 (RTCS_TASK_PTR) &Telnetd_shell_template );

/* EOF */
