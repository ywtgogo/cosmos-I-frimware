#ifndef _dicor_network_apl_h_
#define _dicor_network_apl_h_
/*********************************************************
* 
* 	Copyright (c) 2010  Convertergy;
*	 All Rights Reserved
* 	FileName: dicor_network_apl.h
* 	Version : 
* 	Date    : 
* 	Ower   : peter li
*     Comments:
************************************************************/
#include "network_frame.h"

typedef struct{
  UINT_8  over_incur:1;
  UINT_8  low_incur:1;
  UINT_8  over_invol:1;
  UINT_8  low_invol:1;
  UINT_8  over_outcur:1;
  UINT_8  low_outcur:1;
  UINT_8  over_outvol:1;
  UINT_8  low_outvol:1;
  UINT_8  over_tmp:1;
  UINT_8  state:1;
  UINT_8  dumy:6;
}SMC_DIGITAL_DATA_ST;


/*
typedef struct{
   EMN_SUBNET_ADDR  dev_addr;
   UINT_16  in_cur;
   UINT_16  in_Vol;
   UINT_16  out_cur;
   UINT_16  out_vol;
   UINT_16  temp; //chassis temperature
   SMC_DIGITAL_DATA_ST d;
}EMN_SMC_DATA_ST;

*/

typedef struct{
 UINT_8   frm_type;	
 UINT_8   len;
// UINT_8  subnet; 
 UINT_16  dev_id;
 UINT_16  Vin; //in_cur;
 UINT_16  Vout;//in_Vol;
 UINT_16  Iout;//out_cur;
 UINT_16  Temp;
 UINT_16  out_power;
 UINT_16  d;   //bit11:   Normal   bit12: lowpower
 UINT_16  Energy;
 UINT_16  a;
 UINT_16  b;
}EMN_SMC_DATA_ST;

typedef struct{
  UINT_8 UID[DICOR_ID_LEN];
  EMN_SUBNET_ADDR  addr;  
}REG_TAB_ITEM;
extern ParaType SetParameterData;
extern ParaType NwkSetParaData;
extern GetParaType GetParaData;
extern UINT_8 Get_Data_Flag;
extern UpdateType UpdataData;

extern void EMN_APL_SetParameter(UINT_8 subnet,UINT_8 dev,ParaType *para);
extern void EMN_APL_GetParameter(UINT_8 subnet,UINT_8 dev,GetParaType *para);
extern void EMN_APL_GetSampleData();

extern void SMC_APL_Task(SYS_MESSAGE *msg);
extern void SMC_APL_TaskInit(void);


extern void EMN_APL_Task(SYS_MESSAGE *msg);
extern void EMN_APL_TaskInit(void);
extern void EMT_WriteEthBuf(UINT_8 *data,UINT_8 sample_num);
extern UINT_8  SetDiDiStatus;
extern DIANDIDITABLE_PTR pDiAnDiDiTable;

#define    EMN_UPGRADE_ROUTER_CMD       (1)
#define    EMN_UPGRADE_NOROUTER_CMD       (2)


#endif

