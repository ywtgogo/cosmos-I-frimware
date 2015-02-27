#include <mqx.h>
#include <bsp.h>
#include <string.h>
#include <lwtimer.h>
#include <mfs.h>
#include <shell.h>
#include "dicor.h"
#include "My_Shell_Commands.h"
#include "upgradedidi.h"
#include "dicor_update.h"

const SHELL_COMMAND_STRUCT Shell_commands[] = {  
	{ "reboot", Shell_reboot },
	{ "kill", Shell_kill	},
	{ "pregtable", Shell_pregtable },
	{ "pbaseconfig", Shell_pbaseconfig },
	{ "wbaseconfig", Shell_wbaseconfig}, 
	{ "rbaseconfig", Shell_rbaseconfig },
	{ "rthirddevicetable", Shell_rthirddevicetable},
	{ "wregtable", Shell_wregtable	},
	{ "rregtable", Shell_rregtable	},
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
    { "setdatacenter",	Shell_setdatacenter},
    { "poweroff",    Shell_poweroff },  
    { "gethardversion",    Shell_gethardversion },      
    { "sethardversion",    Shell_sethardversion },
    { "getsoftversion",    Shell_getsoftversion },
    { "getdidipt",    Shell_GetDiDiPt }, 
    { "setdidipt",    Shell_SetDiDiPt },
    { "updatedidi",    Shell_UpdateDiDi },
    { "getdidifilename",    Shell_Getdidiprogramfilename },
    { "setdidifilename",    Shell_Setdidiprogramfilename },
    { "downloaddidiprogram",    Shell_Downloaddidiprogram },
    { "upgradedidi",    Shell_UpgradeDiDi },
    { "detectdevice",    Shell_DetectDevice },
    { "detectdevupgrade",    Shell_DetectDevUpgrade },
    { "detectdevupgrade8255",    Shell_DetectDevUpgrade },
    { "printdidieeprom",    Shell_PrintDiDiEeprom },
    { "setdidieeprom",    Shell_SetDiDiEeprom },
    { "upgradedicor",    Shell_UpgradeDiCor },
   // { "testrf",    Shell_TestRF },
    //{ "t", Shell_RFTest },
    //{ "s", Shell_StopGetData },
    { "setrffreq", Shell_SetRFFreq   },
	{"getdidiversion",Shell_GetDiDiVersion},
    //{ "u",    Shell_UpgradeDiDi },
    { "getdianarg",    Shell_GetDiAnArg },
    { "setdianarg",    Shell_SetDiAnArg },
    #if RTCSCFG_ENABLE_UDP
    { "tftp",      Shell_TFTP_client },
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


void  dicor_shell_task(uint_32 initial_data)
{
	printf("SHELL\n");
	for(;;) 
    {
        Shell(Shell_commands, NULL);
        printf("Shell exited, restarting...\n");
    }
}

