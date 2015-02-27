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
#include <shell.h>
#include <Watchdog.h> 

#include "lw_telnet.h"

#include "Telnet_Shell_Commands.h"

#include "dicor_upload.h"
extern DIDI_DATA_BUFFER    upload_buffer;
//extern DIDI_DATA_BUFFER_PTR p_upload_buffer;



void dicor_telnet_server_task(uint_32 initial_data)
{

	while ((upload_buffer.eth_st!=ETH_CABLE_IP) && (upload_buffer.eth_st!=ETH_CABLE_IP_CON))
	//while ((p_upload_buffer->eth_st!=ETH_CABLE_IP) && (p_upload_buffer->eth_st!=ETH_CABLE_IP_CON))
	{
		RTCS_time_delay(100);
	}
	//start watchdog 
	if (_watchdog_create_component(BSP_TIMER_INTERRUPT_VECTOR,Wacthdog_Error)!= MQX_OK)  
	{ 
	  printf("connect task create watchdog failed !");
	}
	_watchdog_start(60*1000);
	while (1)
	{
		lw_telnet_server(Telnetd_shell_fn);
		_watchdog_start(60*1000);
	}
}
