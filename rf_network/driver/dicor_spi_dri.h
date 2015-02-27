#ifndef _dicor_spi_dri_h_
#define _dicor_spi_dri_h_
/***********************************************************
* 
* 	Copyright (c) 2011  Convertergy;
*	 All Rights Reserved
*	 FileName: dicor_spi_dri.h
*	 Version : 
*	 Date    : 
* 	 Ower   : peter li
*
*      Comments:
*
**************************************************************/
#include <mqx.h>
#include <bsp.h>
#include <mcf5225.h>
extern 	MCF5225_GPIO_STRUCT_PTR rf_gpio_ptr;

//RF_TRX_CE---(MCU64)PUC3 ---output
//#define CLEAR_TRX_CE    (rf_gpio_ptr->CLRUC = 0xf7 )
//#define SET_TRX_CE      (rf_gpio_ptr->PORTUCP_SETUC = 0x08 ) 
#define CLEAR_TRX_CE    (rf_gpio_ptr->PORTUC &= 0xf7 )
#define SET_TRX_CE      (rf_gpio_ptr->PORTUC |= 0x08 ) 
	
//SPI_CS0----(MCU21)-----PQS3---output
//#define SET_CSN          (rf_gpio_ptr->PORTQSP_SETQS = 0x08 )
#define SET_CSN          (rf_gpio_ptr->PORTQS |= 0x08 )
//#define SET_CSN_DATA      TX_waite();  SET_CSN;        
#define SET_CSN_DATA      SET_CSN;        
//#define CLEAR_CSN         (rf_gpio_ptr->CLRQS = 0xf7 )    
#define CLEAR_CSN         (rf_gpio_ptr->PORTQS &= 0xf7 )
	
//RF_PWR_UP---(MCU68)PUC0----output
//#define CLEAR_PWR       (rf_gpio_ptr->CLRUC = 0xfe )
//#define SET_PWR         (rf_gpio_ptr->PORTUCP_SETUC = 0x01 )
//#define CLEAR_PWR       (rf_gpio_ptr->PORTUC &= 0xfe )
//#define SET_PWR         (rf_gpio_ptr->PORTUC |= 0x01 )
//硬件V2.2以后换成RF_PWR_UP---(MCU62)ICOC2----output
#define CLEAR_PWR       (rf_gpio_ptr->PORTTA &= 0xfc )
#define SET_PWR         (rf_gpio_ptr->PORTTA |= 0x04 )
	
//RF_TX_EN ---(MCU38)PUB3----output(1)
//#define CLEAR_TXEN      (rf_gpio_ptr->CLRUB = 0xf7 )
//#define SET_TXEN        (rf_gpio_ptr->PORTUBP_SETUB = 0x08 )
#define CLEAR_TXEN      (rf_gpio_ptr->PORTUB &= 0xf7 )
#define SET_TXEN        (rf_gpio_ptr->PORTUB |= 0x08 )
	

	
	
	//RF_AM 	 ---(MCU42)PTC2----input
	
	//RF_DR引脚硬件有变更---(MCU70)PNQ1----input
//#define GET_DR_PIN()		 (rf_gpio_ptr->PORTNQP_SETNQ&0x02)	
#define GET_DR_PIN()		 (RF_eport_ptr->EPPDR & 0x02)
	//RF_DR   -----(MCU65)PUC2----input(0)		
//#define GET_DR_PIN()		 (rf_gpio_ptr->PORTUCP_SETUC&0x04)
	//RF_CD 	  ---(MCU69)PUC1--- input
//#define GET_CD_PIN()         (rf_gpio_ptr->PORTUCP_SETUC&0x02)
//硬件V2.2以后换成RF_CD---(MCU61)ICOC1----input
#define GET_CD_PIN()         (rf_gpio_ptr->PORTTAP_SETTA&0x02)
	

#define RFNWK_TIMER_INTERRUPT_LEVEL                 (BSP_PIT1_INT_LEVEL)
#define RFNWK_TIMER_INTERRUPT_SUBLEVEL              (BSP_PIT1_INT_SUBLEVEL)
#define RFNWK_TIMER_INTERRUPT_VECTOR                (BSP_PIT1_INT_VECTOR)
#define RFNWK_TIMER                                 (BSP_LAST_TIMER)
#define RFNWK_TIMER_FREQUENCY                        (500)

extern void dicor_dri_init(void);



#endif

