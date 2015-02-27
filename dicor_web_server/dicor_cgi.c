/**********************************************************************
* 
* Copyright (c) 2010 convertergy;
* All Rights Reserved
*
*FileName: dicor_cgi.c
* Version : 
* Date    :
*
* Comments:
*
* 
*
************************************************************************/
#include <httpd.h>
//#include "httpdsrv.h"
#include "dicor_cgi.h"
static int cgi_rtc_data();
static void  fn_rtc_data();
//asp-like table 
const HTTPD_FN_LINK_STRUCT dicor_fn_lnk_tbl[] = {
	{ "rtcdata_fn",		fn_rtc_data},
	{ 0, 0 }
};
//cgi table
const HTTPD_CGI_LINK_STRUCT dicor_cgi_lnk_tbl[] = {
	{ "rtcdata_cgi",		cgi_rtc_data},
	{ 0, 0 }    // DO NOT REMOVE - last item - end of table
};
static int cgi_rtc_data(HTTPD_SESSION_STRUCT *session) 
{
	TIME_STRUCT time;
	int min, hour;
	
	_time_get(&time);
	
	min = time.SECONDS / 60;
	hour = min / 60;
	min %= 60;
	
	CGI_SEND_NUM(hour);
	CGI_SEND_NUM(min);
	CGI_SEND_NUM(time.SECONDS % 60);
	return session->request.content_len;
}
static void  fn_rtc_data(HTTPD_SESSION_STRUCT *session) 
{
	TIME_STRUCT time;
	int min, hour;
	
	_time_get(&time);
	
	min = time.SECONDS / 60;
	hour = min / 60;
	min %= 60;
	
	CGI_SEND_NUM(hour);
	CGI_SEND_NUM(min);
	CGI_SEND_NUM(time.SECONDS % 60);

}



