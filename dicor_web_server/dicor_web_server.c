/**HEADER********************************************************************
* 
* Copyright (c) convertergy ;
* All Rights Reserved
* FileName: dicor_web_srever.c
* Version : 
* Date    : 
* ower: peter li
*
* Comments:*
* 
*************************************************************************/
#include <mqx.h>
#include <bsp.h>
#include <rtcs.h>
#include <httpd.h>

extern const HTTPD_FN_LINK_STRUCT dicor_fn_lnk_tbl[]; 		
extern const HTTPD_CGI_LINK_STRUCT dicor_cgi_lnk_tbl[];


#define HTTPD_SEPARATE_TASK     0
//provide static content 
const HTTPD_ROOT_DIR_STRUCT dicor_page_root_dir[] = {
	{ "", "tfs:" },
	{ "test", "FAT12:/test" },		
	{ 0, 0 }
};
/*-----------------------------------------------------
* 
* Task Name    : dicor_web_server_task
* Comments     :
*    This task opens a second I/O channel and prints a message
* to it.
*
*-----------------------------------------------------*/

void dicor_web_server_task ( uint_32 initial_data  )
{
        int_32            error;
    	HTTPD_STRUCT *server;
	HTTPD_PARAMS_STRUCT *params;  
	
    printf("preparing http server...\n");    
    params = httpd_default_params(NULL);
    if (params) 
    {
	params->root_dir = (HTTPD_ROOT_DIR_STRUCT*)dicor_page_root_dir;
    	params->index_page = "\\index.html";
	server = httpd_init(params);
	}
    HTTPD_SET_PARAM_CGI_TBL(server, (HTTPD_CGI_LINK_STRUCT*)dicor_cgi_lnk_tbl);
    HTTPD_SET_PARAM_FN_TBL(server, (HTTPD_FN_LINK_STRUCT*)dicor_fn_lnk_tbl);

    printf("run http server...\n");
#if HTTPD_SEPARATE_TASK || !HTTPDCFG_POLL_MODE	
	httpd_server_run(server);	
	/* user stuff come here */
	_task_block();
#else	            // if possible, telnet server and web server use the one task.
    while (1) {
        httpd_server_poll(server, 1);        
        /* user stuff come here - only non blocking calls */
    }
#endif	
}

/* EOF */
