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
* $FileName: Security_Shell_Commands.c$
* $Version : 3.6.11.0$
* $Date    : Jun-4-2010$
*
* Comments:
*
*   
*
*END************************************************************************/

#include <mqx.h>
#include <bsp.h>
#include <mfs.h>
#include <shell.h>
#include "Telnet_Shell_Commands.h"
#include "My_Shell_Commands.h"
#include "upgradedidi.h"
#include "Dicor_update.h"




const SHELL_COMMAND_STRUCT Telnet_commands[] = {
	{ "reboot", Shell_reboot },
	{ "testwatchdog", Shell_testwatchdog },
	{ "kill", Shell_kill	},
	{ "pregtable", Shell_pregtable },
	{ "rregtable", Shell_rregtable	},
	{ "wbaseconfig", Shell_wbaseconfig }, 
	{ "wregtable", Shell_wregtable	},
	{ "pbaseconfig", Shell_pbaseconfig },
	{ "rbaseconfig", Shell_rbaseconfig },
	{ "wroutertable", Shell_wroutertable },
	{ "proutertable", Shell_proutertable },
	{ "rroutertable", Shell_rroutertable },
	{ "wdidi_diantable", Shell_wdidi_diantable },
	{ "pdidi_diantable", Shell_pdidi_diantable },
	{ "rdidi_diantable", Shell_rdidi_diantable },
	{ "wall",    Shell_wall },
	{ "rall",    Shell_rall },
	{ "pall",    Shell_pall },
	{ "getrtc",    Shell_getrtc },      
    { "setrtc",    Shell_setrtc },
    { "getuid",    Shell_getuid },  
    { "setuid",    Shell_setuid },    
    { "poweroff",    Shell_poweroff },  
    { "gethardversion",    Shell_gethardversion },  
    { "sethardversion",    Shell_sethardversion },
    { "getsoftversion",    Shell_getsoftversion },
    { "getdidipt",    Shell_GetDiDiPt }, 
    { "setdidipt",    Shell_SetDiDiPt }, 
    { "updatedidi",    Shell_UpdateDiDi },
    { "downloaddidiprogram",    Shell_Downloaddidiprogram },
    { "getdidifilename",    Shell_Getdidiprogramfilename },
   { "setdidifilename",    Shell_Setdidiprogramfilename },
    { "upgradedidi",    Shell_UpgradeDiDi },
    { "upgradedidi8255",    Shell_UpgradeDiDi },
    //{ "upgradedidi8245",    Shell_UpgradeDiDi8245 },
    { "detectdevice",    Shell_DetectDevice },
   // { "detectdevice430",    Shell_DetectDevice430 },
   // { "detectdevupgrade",    Shell_DetectDevUpgrade },
    //{ "detectdevupgrade8245",    Shell_DetectDevUpgrade8245 },
    { "detectdevupgrade8255",    Shell_DetectDevUpgrade },
    { "printdidieeprom",    Shell_PrintDiDiEeprom },
    { "setdidieeprom",    Shell_SetDiDiEeprom },
    { "upgradedicor",    Shell_UpgradeDiCor },
 //   { "testrf",    Shell_TestRF },
    //{ "t", Shell_RFTest },
    {"getdidiversion",Shell_GetDiDiVersion},
    //{ "s", Shell_StopGetData },
    { "setrffreq", Shell_SetRFFreq   },
    //{ "u",    Shell_UpgradeDiDi },
    { "getdianarg",    Shell_GetDiAnArg },
    { "setdianarg",    Shell_SetDiAnArg },
    #if RTCSCFG_ENABLE_UDP
    { "tftp",      Shell_TFTP_client },
    //{ "tftpd",	Shell_TFTPd    },
	#endif
    { "cd",        Shell_cd },      
    { "copy",      Shell_copy },
    { "create",    Shell_create },
    { "del",       Shell_del },       
    { "disect",    Shell_disect},      
    { "dir",       Shell_dir },      
    { "exit",      Shell_exit }, 
    { "format",    Shell_format },
    { "help",      Shell_help }, 
    { "mkdir",     Shell_mkdir },     
    { "pwd",       Shell_pwd },       
    { "read",      Shell_read },      
    { "ren",       Shell_rename },    
    { "rmdir",     Shell_rmdir },
    { "sh",        Shell_sh },
    { "type",      Shell_type },
    { "write",     Shell_write },     
    { "?",         Shell_command_list },     
    { NULL,        NULL } 
};


   
void Telnetd_shell_fn() 
{  
  Shell(Telnet_commands,NULL);
}


