#ifndef __di_harvest_h_
#define __di_harvest_h_
/*********************************************************
* 
* 	Copyright (c) 2011  Convertergy;
*	 All Rights Reserved
* 	FileName: di-harvester.h
* 	Version : 
* 	Date    : 
* 	Ower   : peter li
*     Comments:
************************************************************/
#include "network_frame.h"
#ifdef DI_HARVESTER_V3
#define  HAVST_SUB0_DEV_ADDR        (0x01) 

//command send by dicor to DiAn
#define  DIAN_START_MPPT_CMD        (0x10)
#define  DIAN_EXIT_MPPT_CMD         (0x20)
#define  DIAN_ADJUST_VOL_CMD        (0x30)

//command send by dicor to DiDi
//#define  DIDI_ALL_FORCE_DIRECT_CMD  (0x01)
#define  DIDI_ALL_ENTER_NOMAL_CMD   (0x02)
#define  DIDI_CHANGE_MPPT_MODE_CMD  (0x03)

//di-harvest state
#define  DI_HAVST_START             (0x01)
#define  DI_HAVST_COLLECT_DATA      (0x02) 
#define  DI_HAVST_PARSE_DATA        (0x03) 
#define  DI_HAVST_HANDLE_COMMAND    (0x04) 



//Plant state
#define  DIDI_ALL_INIT_ST           (0x01)
#define  DIDI_ALL_NORMAL_ST         (0x02)
#define  DIDI_ALL_SWITCHING_ST      (0x03)
#define  DIDI_ALL_FORCE_DIRECT_ST   (0x04)

#define  DIDN_INIT_ST               (0x10)
#define  DIDN_MPPT_ST               (0x20)
#define  DIDN_NO_MPPT_ST            (0x30)
#define  DIDN_SWITCHING_ST          (0x40)

#define REG_DIAN_NUM                (0x01)
#define REG_DIDI_NUM_EXCEPT         (80)

#define DIAN_VBUS_MAX               (550)
#define DIAN_VBUS_MIN               (370)
#define DIAN_VBUS_SCALE             (10) 
typedef struct 
{ 
 UINT_8    sys_cur_st;
 UINT_8    sys_expect_st;
 UINT_8    init_end;
 UINT_8    low_power_count; 
 UINT_8    string_num; 
 UINT_8    command;
 UINT_8    dian_sample_st;
 UINT_32   didi_sample_st;
 UINT_16   dian_bus_voltage;
} DIAN_INFO, _PTR_  DIAN_INFO_PTR;
typedef struct 
{
 UINT_16  vol_in;
 UINT_16  vol_out;
 UINT_8   scale;
} DIDI_INFO, _PTR_  DIDI_INFO_PTR;

/*typedef enum 
{
  NO_USED,
  SET_PARA,
  HAVST_SEND_CMD
}SET_PARA_TYPE;
*/
     
typedef struct 
{
 DIAN_INFO  dian_info[REG_DIAN_NUM];
 DIDI_INFO  didi_info[REG_DIDI_NUM_EXCEPT];
} PLANT_INFO, _PTR_  PLANT_INFO_PTR;

extern UINT_8 HAVST_State;
extern SET_PARA_TYPE EMN_SetParaType;

extern void HAVST_CollectPlantInfo(UINT_8 *data,UINT_16 data_num);
#endif

#endif

