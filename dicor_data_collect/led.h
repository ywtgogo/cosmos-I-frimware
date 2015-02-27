#ifndef __led_h__
#define __led_h__
#include "mqx.h"
/**********************************************************************
* 
* Copyright (c) 2011 convertergy ;
* All Rights Reserved
* FileName: led.h 
* Version : 
* Date    : 2011-12-08
*
* Comments:
*
*   This file contains definitions for the upload MG's data to data center
*
*************************************************************************/


typedef enum {
   DICOR_RF_LED=0,
   DICOR_ETHERNET_LED,
   DICOR_DIMO_LED,
   DICOR_ALIVE_LED,
   DICOR_SD_LED,
   DICOR_MAX_LED
} DICOR_Led_t;

extern volatile uchar RFLedNum;

//#define CLEAR_LED12		(led_gpio_ptr->CLRUC = 0xFB)
//#define SET_LED12		(led_gpio_ptr->PORTUCP_SETUC = 0x04)

//#define CLEAR_LED13		(led_gpio_ptr->CLRAN = 0xFE)
//#define SET_LED13		(led_gpio_ptr->PORTANP_SETAN = 0x01)


//#define CLEAR_LED28		(led_gpio_ptr->CLRTA = 0xF7)
//#define SET_LED28		(led_gpio_ptr->PORTTAP_SETTA = 0x08)

//#define CLEAR_LED11		(led_gpio_ptr->CLRAS = 0xFD)
//#define SET_LED11		(led_gpio_ptr->PORTASP_SETAS = 0x02)

//#define CLEAR_LED9		(led_gpio_ptr->CLRAS = 0xFE)
//#define SET_LED9		(led_gpio_ptr->PORTASP_SETAS = 0x01)
#define BSP_BUTTON_REPAIR   (GPIO_PORT_NQ | GPIO_PIN7)
//#define BSP_HARD_WDG		(GPIO_PORT_TC | GPIO_PIN4)
#define BLINK_WDG	 	(led_gpio_ptr->PORTTC ^= 0x04)

#define LED2RED_OFF		(led_gpio_ptr->PORTUA &= 0xfb)
#define LED2RED_ON		(led_gpio_ptr->PORTUA |= 0x04)
#define LED2RED_BLIK	(led_gpio_ptr->PORTUA ^= 0x04)

#define LED2GREEN_OFF	(led_gpio_ptr->PORTUA &= 0xf7)
#define LED2GREEN_ON	(led_gpio_ptr->PORTUA |= 0x08)

#define CLEAR_LED12		(led_gpio_ptr->PORTUC &= 0xFB)
#define SET_LED12		(led_gpio_ptr->PORTUC |= 0x04)

#define CLEAR_reset_pin     (led_gpio_ptr->PORTUC &= 0xFD)
#define SET_reset_pin      	(led_gpio_ptr->PORTUC |= 0x02)       

#define CLEAR_LED13		(led_gpio_ptr->PORTAN &= 0xFE)
#define SET_LED13		(led_gpio_ptr->PORTAN |= 0x01)


#define CLEAR_LED28		(led_gpio_ptr->PORTTA &= 0xF7)
#define SET_LED28		(led_gpio_ptr->PORTTA |= 0x08)

#define CLEAR_LED11		(led_gpio_ptr->PORTAN &= 0x7F)
#define SET_LED11		(led_gpio_ptr->PORTAN |= 0x80)
#define BLINK_LED11		(led_gpio_ptr->PORTAN ^= 0x80)

#define CLEAR_LED9		(led_gpio_ptr->PORTAN &= 0xF7)
#define SET_LED9		(led_gpio_ptr->PORTAN |= 0x08)

//D13-AN0 //D9-PAN3 D11-PAN7


void LedInit(void);
void LedTest(void);
void AllLedOn(void);
void AllLedOff(void);
void LED2RED_on(void);
void LED2RED_off(void);
void LED2GREEN_off(void);
void LED2GREEN_on(void);
void RESET_CC2530(void);
void N_RESET_CC2530(void);
void RfLedOn(void);
void RfLedOff(void);
void EthernetLedOn(void);
void EthernetLedOff(void);
void DimoLedOn(void);
void DimoLedOff(void);
void RunLedOn(void);
void RunLedOff(void);
void RunLedBlink(void);
void SdLedOn(void);
void SdLedOff(void);
void LedOutPut(DICOR_Led_t signal, boolean state);
void GPIO_PULLDOWN(void);
void RepairModeInit(void);

#endif
/* EOF */
