#ifndef __sh_my_h__
#define __sh_my_h__

extern int_32 Shell_get_time();
extern int_32 Shell_get_zone();
extern int_32 Shell_set_zone(int_32 argc, char_ptr argv[]);
extern int_32 Shell_setrtc(int_32 argc, char_ptr argv[]); 
extern int_32 Shell_getrtc(int_32 argc, char_ptr argv[]); 
extern int_32 Shell_wall(int_32 argc, char_ptr argv[]);
extern int_32 Shell_pall(int_32 argc, char_ptr argv[]);
extern int_32 Shell_rall(int_32 argc, char_ptr argv[]);
extern int_32 Shell_wbaseconfig(int_32 argc, char_ptr argv[]);
extern int_32 Shell_wregtable(int_32 argc, char_ptr argv[]);
extern int_32 Shell_wdidi_diantable(int_32 argc, char_ptr argv[]);
extern int_32 Shell_pregtable(int_32 argc, char_ptr argv[]);
extern int_32 Shell_reboot(int_32 argc, char_ptr argv[]);
extern int_32 Shell_pbaseconfig(int_32 argc, char_ptr argv[]);
extern int_32 Shell_rbaseconfig(int_32 argc, char_ptr argv[]);
extern int_32 Shell_rthirddevicetable(int_32 argc, char_ptr argv[]);
extern int_32 Shell_setdatacenter(int_32 argc, char_ptr argv[]);
extern int_32 Shell_setport(int_32 argc, char_ptr argv[]);
extern int_32 Shell_setdatacentermux(int_32 argc, char_ptr argv[]);
extern int_32 Shell_setportmux(int_32 argc, char_ptr argv[]);


extern int_32 Shell_rregtable(int_32 argc, char_ptr argv[]);
extern int_32 Shell_wroutertable(int_32 argc, char_ptr argv[]);
extern int_32 Shell_proutertable(int_32 argc, char_ptr argv[]);
extern int_32 Shell_rroutertable(int_32 argc, char_ptr argv[]);
extern int_32 Shell_pdidi_diantable(int_32 argc, char_ptr argv[]);
extern int_32 Shell_rdidi_diantable(int_32 argc, char_ptr argv[]);
extern int_32  Shell_TFTP_client(int_32 argc, char_ptr argv[] );
extern int_32  Shell_TFTPd(int_32 argc, char_ptr argv[] );
extern int_32  Shell_kill(int_32 argc, char_ptr argv[] );
extern int_32 Shell_setuid(int_32 argc, char_ptr argv[]);
extern int_32 Shell_getuid(int_32 argc, char_ptr argv[]);
extern int_32 Shell_sethardversion(int_32 argc, char_ptr argv[]);
extern int_32 Shell_gethardversion(int_32 argc, char_ptr argv[]);
extern int_32 Shell_getsoftversion(int_32 argc, char_ptr argv[]);
extern int_32  Shell_GetDiDiPt(int_32 argc, char_ptr argv[] );
extern int_32  Shell_SetDiDiPt(int_32 argc, char_ptr argv[] );
extern int_32  Shell_GetDiAnArg(int_32 argc, char_ptr argv[]);
extern int_32  Shell_SetDiAnArg(int_32 argc, char_ptr argv[]);
extern int_32 Shell_poweroff(int_32 argc, char_ptr argv[]);
extern int_32 Shell_testwatchdog(int_32 argc, char_ptr argv[]);
extern int_32  Shell_UpdateDiDi(int_32 argc, char_ptr argv[] );
extern int_32 Shell_SetRFFreq(int_32 argc, char_ptr argv[]);
extern int_32 Shell_GetDiDiVersion(int_32 argc, char_ptr argv[]);

#endif

/* EOF*/
