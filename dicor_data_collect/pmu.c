/*************************************************************
* 
* Copyright (c) convertergy ;
* All Rights Reserved
* FileName: pmu.c
* Version : 
* Date    : 2011/09/28
* ower    : Alex
*
* Comments:电源管理&软件看门狗管理
*
*
***************************************************************/

#include <mqx.h>
#include <bsp.h>
#include "dicor.h"
#include "pmu.h"
#include "logging_public.h"
#include "rs485.h"

static MCF5225_GPIO_STRUCT_PTR pmu_gpio_ptr;
static VMCF522XX_EPORT_STRUCT_PTR pmu_eport_ptr;

static void pmu_pfo_int_hdl(pointer param);
static void reboot(void);

volatile uchar LowVolFlag = 0;
static void dicor_low_vol_task(void);
volatile uchar TaskIndex = 255;

extern void upload_data_exit(void);
extern void rf_nwk_exit(void);
extern void dicor_get_logtime(DATE_STRUCT * date);

extern const TASK_TEMPLATE_STRUCT MQX_template_list[];

static uchar MatchTaskName(pointer td_ptr)
{
	uchar i;
	_task_id task_id;
	pointer ptr;

	for (i=0; i<DICOR_MAX; i++)
	{
		task_id = _task_get_id_from_name(MQX_template_list[i].TASK_NAME);
		ptr = (pointer)_task_get_td(task_id);
		if (ptr == td_ptr)
		{
			break;
		}
	}
	return i;
}



void Wacthdog_Error(pointer td_ptr)
{
	uchar i;
    
    i = MatchTaskName(td_ptr);
    TaskIndex = i;
}


static void pmu_pfo_int_hdl(pointer param)
{
	if (!(pmu_eport_ptr->EPPDR & 0x80))//防止误动作
	{
		LowVolFlag = 1;
	}
	pmu_eport_ptr->EPFR |= 0x80; /* clear flag(s) */
}

void PMUInit(void)
{
	//install  interrupter   

	VMCF5225_STRUCT_PTR  mcf5225_ptr;
	
	pmu_gpio_ptr = (MCF5225_GPIO_STRUCT_PTR) _bsp_get_gpio_base_address();

   //init I/O for EPORT. 
	pmu_gpio_ptr->PNQPAR = ((pmu_gpio_ptr->PNQPAR&0x3fff)|0x4000) ;
	pmu_gpio_ptr->DDRNQ &= 0x7f;
	
    //init EPORT
    mcf5225_ptr = _PSP_GET_IPSBAR();
    pmu_eport_ptr = &mcf5225_ptr->EPORT[0];
	pmu_eport_ptr->EPDDR &= 0x7f; /*  irq7 is input */
    pmu_eport_ptr->EPPAR =((pmu_eport_ptr->EPPAR &0x3fff)|0x8000);/*  falling edge */
    pmu_eport_ptr->EPIER |=0x80; /* set interrupt enable */
	//install ISR
	_int_install_isr(MCF5225_INT_EPORT0_EPF7, 
	                   (void (_CODE_PTR_)(pointer))pmu_pfo_int_hdl, NULL);
	
	_mcf5225_int_init(MCF5225_INT_EPORT0_EPF7, BSP_EPORT0_EPF7_INT_LEVEL, 
					 BSP_EPORT_EPFX_INT_SUBLEVEL_MIDPOINT, 
					 TRUE);


//	pmu_gpio_ptr->PNQPAR = ((pmu_gpio_ptr->PNQPAR&0x3fff)|0x0000) ;
//	pmu_gpio_ptr->DDRNQ &= 0x7f;
	

}



static void reboot(void)
{

	MCF5225_CCM_STRUCT_PTR ccr_reg_addr;
	ccr_reg_addr =(MCF5225_CCM_STRUCT_PTR) (&((VMCF5225_STRUCT_PTR) _PSP_GET_IPSBAR())->CCM);
	ccr_reg_addr->RCR |= 0x80;

}



static void dicor_low_vol_task(void)
{
	printf("\r\n*********low vol intertupt**********\r\n");
		

	//保护现场
	upload_data_exit();
	rf_nwk_exit();
	
	while(1)
	{
		//检测寄存器，等待高电平
		if (pmu_eport_ptr->EPPDR & 0x80)
		{
			printf("\r\n*********power on**********\r\n");
			printf("reboot\r\n");
			reboot();
	
		}
		
		//增加中断检测来关断  14.4.15
	//	if (pmu_eport_ptr->EPPDR & 0x80)
		
		
		//_time_delay(2);
	}
	
}

void dicor_watchdog_task(uint_32 initial_data)
{
	DATE_STRUCT date;
	while (1)
	{
		if (LowVolFlag)
		{
			dicor_low_vol_task();
		}
		//有软件看门狗触发重启
		if (TaskIndex != 255)
		{
		// by younger 2013.7.1
			printf("\nSoftware watchdog reboot by task [%s]\n", MQX_template_list[TaskIndex].TASK_NAME);
		//	dicor_get_logtime(&date);
		//by younger 2013.7.1	
			sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t 软件看门狗复位，复位源任务[%s]\r\n", date.HOUR,date.MINUTE,date.SECOND,MQX_template_list[TaskIndex].TASK_NAME);
     //	PrintLog(pDiCorLog->logbuf);
			//释放CPU资源让低优先级的任务保存数据到SD卡中
			_time_delay(3000);
			//等待保存结束
			_lwsem_wait(&pDiCorLog->writesem);
			reboot(); // by younger 2013.7.1
		}
		_time_delay(100);
	}
}

/* EOF */
