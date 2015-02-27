/**HEADER********************************************************************
* 
* Copyright (c) 2008 Freescale Semiconductor;
* All Rights Reserved
*
* Copyright (c) 2004-2008 Embedded Access Inc.;
* All Rights Reserved
*
*************************************************************************** 
*
* THIS SOFTWARE IS PROVIDED BY FREESCALE "AS IS" AND ANY EXPRESSED OR 
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  
* IN NO EVENT SHALL FREESCALE OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
* THE POSSIBILITY OF SUCH DAMAGE.
*
**************************************************************************
*
* $FileName: telnet_demo.c$
* $Version : 3.7.20.1$
* $Date    : Apr-4-2011$
*
* Comments:
*
*   Security telnet example using RTCS Library.
*
*END************************************************************************/

#include <mqx.h>
#include <bsp.h>
#include <rtcs.h>
#include <enet.h>
#include <ipcfg.h>
#include "tftp.h"


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name :  Dicor_TFTP_client
*  Returned Value:  none
*  Comments  :  utility to TFTP to or from a host
*  Usage:  
*
*END*-----------------------------------------------------------------*/

int_32  Dicor_TFTP_client(_ip_address hostaddr, char * strremotefile, char* strlocalfile)
{
	char                 hostname[64];
	char_ptr             file_ptr;
	uint_32              tftp_handle, buffer_size/*,byte_number*/;
	uchar_ptr            buffer_ptr;
	TFTP_DATA_STRUCT     tftp_data;
	MQX_FILE_PTR         fd = NULL;
	int_32               error;
	int_32               return_code = 0; 
	boolean              trans = FALSE;
	char 			 	 erase = 1;

	tftp_data.SERVER   = hostaddr;
	tftp_data.FILENAME = strremotefile;
	tftp_data.FILEMODE = "netascii";
	file_ptr = strlocalfile;
	
	if (1)  
	{
		printf("\nDownloading file %s from TFTP server: %s [%ld.%ld.%ld.%ld]\n",
		  tftp_data.FILENAME,hostname, IPBYTES(hostaddr));
		tftp_handle = (*FT_TFTP->OPEN)( (pointer) &tftp_data );
		if ( tftp_handle != RTCS_OK )  
		{
			printf("\nError opening file %s\n",tftp_data.FILENAME);
		  	return_code = -1;
		}
		else
		{
			while (! (*FT_TFTP->EOFT)()) 
			{
				do 
				{
					buffer_ptr = (*FT_TFTP->READ)( &buffer_size );
					if (buffer_ptr != NULL)  
					{					
						if (erase)
						{	
							_time_delay(100);
							fd = fopen(strlocalfile,"w+");
							if (fd == NULL)
							{
								printf("\nError opening local file %s\n",file_ptr);
								return -1;			
							}							
							erase = 0;							
						}										
						 fseek(fd, 0 , IO_SEEK_CUR);
						 _io_write(fd,buffer_ptr,buffer_size); 
						 trans = TRUE;
					}
					else 
					{						
						switch (buffer_size) 
						{
						case (RTCSERR_TFTP_ERROR + 1):
							printf("\nFile %s not found\n", tftp_data.FILENAME);
							return_code = -1;											
							break;
						case (RTCSERR_TFTP_ERROR + 2):
							printf("\nAccess violation\n");
							break;
						case (RTCSERR_TFTP_ERROR + 3):
							printf("\nDisk full or allocation exceeded\n");
							break;
						case (RTCSERR_TFTP_ERROR + 4):
							printf("\nIllegal TFTP operation\n");
							break;
						case (RTCSERR_TFTP_ERROR + 5):
							printf("\nUnknown transfer ID\n");
							break;
						case (RTCSERR_TFTP_ERROR + 6):
							printf("\nFile already exists\n");
							break;
						case (RTCSERR_TFTP_ERROR + 7):
							printf("\nNo such user\n");
							break;
						default:
							if(trans) 
							{
								trans =FALSE;						
								break;
							}
				          	else
				          	{
				            	printf("\nError reading file %s\n", tftp_data.FILENAME);
				            	return_code = -1;			          		
				          	}

						} /* Endswitch */						
					}
			     }while(buffer_ptr !=NULL);
				 if (fd != NULL)
					fclose(fd);	
			 }			 
			 error = (*FT_TFTP->CLOSE)();		  
		}	
	}   	
    return return_code;
} /* Endbody */

void TftpdStart(void)
{
	uint_32 error;

	error = TFTPSRV_init("TFTP_server", SHELL_TFTPD_PRIO, SHELL_TFTPD_STACK );
	if (error ==  0)  
	{
		printf("TFTP Server Started.\n");
	} 
	else  
	{
		printf("Unable to start TFTP Server, error = 0x%x\n",error);
	}
}