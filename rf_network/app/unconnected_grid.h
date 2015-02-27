#ifndef __di_harvest4_h_
#define __di_harvest4_h_
/*********************************************************
* 
* 	Copyright (c) 2011  Convertergy;
*	 All Rights Reserved
* 	FileName: unconnected_grid.h
* 	Version : 
* 	Date    : 
* 	Ower   : peter li
*     Comments:
************************************************************/
#include "network_frame.h"
#ifdef DI_HARVESTER_V4
#define  HAVST_SUB0_DEV_ADDR        (0x00) 

//command send by dicor to DiDi
#define  DIDI_START_MPPT_CMD        (0x01)
#define  DIDI_START_DIRECT_CMD      (0x02)
#define  DIDI_CHANGE_MPPT_MODE_CMD  (0x03)


//Plant state
#define  DI_HAVST_START             (0x01)
#define  DI_HAVST_COLLECT_DATA      (0x02) 
#define  DI_HAVST_PARSE_DATA        (0x03) 
#define  DI_HAVST_HANDLE_COMMAND    (0x04) 

//one didi state
#define  DIDI_MPPT_ST               (0x00)
#define  DIDI_DIRECT_ST             (0x01)
#define  DIDI_SWITCHING_ST          (0x40)

#define REG_DIDI_NUM_EXCEPT         (80)
#define REG_STRING_NUM              (0x01)
#define DIDI_IN_MPPT_NUM_MAX        (0x0e)
#define REG_BOX_NUM                 (0x01)
typedef struct 
{
 UINT_16  last_power_out;
 UINT_16  cur_power_out;
 UINT_16  vol_in;
 UINT_16  vol_out;
 UINT_8  state;
 UINT_8  cmd;
 UINT_8  power_down_state;
} DIDI_INFO, _PTR_  DIDI_INFO_PTR;

typedef struct 
{
 UINT_16  didi_num;
 UINT_16  voltage; 
 DIDI_INFO  *didi_info;
} STRING_INFO, _PTR_  STRING_INFO_PTR;

typedef struct 
{
 UINT_16  string_num;
 UINT_16  current;
 UINT_16  voltage;
 STRING_INFO  *string_info;
} JUNCTION_BOX_INFO, _PTR_  JUNCTION_BOX_PTR;

    
typedef struct 
{
 JUNCTION_BOX_INFO *box;
} PLANT_INFO, _PTR_  PLANT_INFO_PTR;

extern UINT_8 HAVST_State;
extern SET_PARA_TYPE EMN_SetParaType;

extern void HAVST_CollectPlantInfo(UINT_8 *data,UINT_16 data_num);
extern void NWK_CordMakeCmdPacket(UINT_8 subnet,UINT_8 *frame_data);

#endif

#endif

