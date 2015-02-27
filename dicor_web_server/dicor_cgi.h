/**********************************************************************
* 
* Copyright (c) 2010 convertergy;
* All Rights Reserved
*
* FileName: dicor_cgi.h
* Version :
* Date    : 
*
* Comments:
*
*************************************************************************/

#ifndef _dicor_cgi_h_
#define _dicor_cgi_h_

#define CGI_SEND_NUM(val)						\
{												\
	char str[20];									\
	sprintf((char_ptr)&str, "%ld\n", val);			\
	httpd_sendstr(session->sock, str);				\
}

#define CGI_SEND_STR(val)						\
{												\
	httpd_sendstr(session->sock, val);				\
	httpd_sendstr(session->sock, "\n");				\
}


#endif
