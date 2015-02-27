/*************************************************************
* 
* Copyright (c) convertergy ;
* All Rights Reserved
* FileName: led.c
* Version : 
* Date    : 2011/09/27
* ower    : Alex
*
* Comments:LEDµÆÇý¶¯
*
*
***************************************************************/

#include <mqx.h>
#include <bsp.h>

#include "led.h"

static MCF5225_GPIO_STRUCT_PTR led_gpio_ptr;
//#define UART2_CHANNEL "ttyc:"
//MQX_FILE_PTR uart2_dev = NULL;
volatile uchar RFLedNum = 0; 
volatile uint_32 int_counter;
void int_callback(void);

void int_callback(void) 
{
    int_counter++;
    printf("int_counter = %d\n", int_counter);
}

void RepairModeInit(void)
{
	MQX_FILE_PTR	port_file4;
    GPIO_PIN_STRUCT pins_int[] = {
        BSP_BUTTON_REPAIR | GPIO_PIN_IRQ_ZERO,
        GPIO_LIST_END
    };
	printf("\nport_file4 initing!\n");
    /* opening pins/signals for input */
    if (NULL == (port_file4 = fopen("gpio:read", (char_ptr) &pins_int )))
    {
       printf("Opening file4 GPIO with associated pins failed.\n");
      _mqx_exit(-1);
    }
	printf("\nport_file4 inited!\n");

    /* install gpio interrupt callback */
    ioctl(port_file4, GPIO_IOCTL_SET_IRQ_FUNCTION, int_callback);
	printf("\nÑ½£¿\n");
	//ioctl(port_file4, GPIO_IOCTL_ENABLE_IRQ, int_callback);
	
//    while (int_counter < 5);
    
    fclose(port_file4);		
/*
//	printf("BUTTON ON!\n");	
//	led_gpio_ptr->PNQPAR &= 0x3FFF;
//	led_gpio_ptr->DDRNQ &= 0x7F;
    MCF_GPIO_PNQPAR=0x0000;
    MCF_GPIO_DDRNQ=0x00; 	
//	printf("BUTTON ON!\n");
//	while(1)
	{
		_time_delay(10);
		//printf("BUTTON ON! 0x%04x\n", MCF_GPIO_SETNQ);
		if (~MCF_GPIO_SETNQ & 0x80)
		{
			_time_delay(1000);
			if (~MCF_GPIO_SETNQ & 0x80)
			{
				printf("BUTTON ON~~~!\n");
				LED2RED_blik();
			}				
		}
	}
*/

}

void LedInit(void)
{
	led_gpio_ptr = (MCF5225_GPIO_STRUCT_PTR) _bsp_get_gpio_base_address();

//	led_gpio_ptr->PNQPAR &= 0x3FFF;
//	led_gpio_ptr->DDRNQ |= 0x80;
	led_gpio_ptr->PUCPAR &= 0xFB;	//D12-PUC2
	led_gpio_ptr->DDRUC |= (0x04|(0x01<<1));//Lewis.Chen--CC2530ResetPin

//	led_gpio_ptr->PASPAR &= 0xF0;	//D9-PAS0 D11-PAS1
//	led_gpio_ptr->DDRAS |= 0x03;
	
	led_gpio_ptr->PANPAR &= 0x76;	//D13-AN0 //D9-PAN3 D11-PAN7
	led_gpio_ptr->DDRAN |= 0x89;
	
	led_gpio_ptr->PTAPAR &= 0x3F;	//D28-PTA3
	led_gpio_ptr->DDRTA |= 0x08;
		
	AllLedOff();
	
}
void LedTest(void)
{
		
	printf("LED Test ...\n");
	AllLedOn();
	_time_delay(500);
	AllLedOff();
	_time_delay(500);
	AllLedOn();
	_time_delay(500);
	AllLedOff();
	_time_delay(500);
	AllLedOn();
	_time_delay(500);
	AllLedOff();
	printf("LED Test Done!\n");
}
void AllLedOn(void)
{
	CLEAR_LED12;
	CLEAR_LED13;	
	CLEAR_LED28;	
	CLEAR_LED11;
	CLEAR_LED9;
}


void AllLedOff(void)
{
	SET_LED12;
	SET_LED13;	
	SET_LED28;	
	SET_LED11;
	SET_LED9;
}

void LED2RED_off(void)
{
	led_gpio_ptr->PUAPAR &= 0xcF;
	led_gpio_ptr->DDRUA |= 0x04;
	LED2RED_OFF;
}

void LED2RED_on(void)
{
	led_gpio_ptr->PUAPAR &= 0xcF;
	led_gpio_ptr->DDRUA |= 0x04;
	LED2RED_ON;
}

void LED2RED_blik(void)
{
	led_gpio_ptr->PUAPAR &= 0xcF;
	led_gpio_ptr->DDRUA |= 0x04;
	LED2RED_BLIK;
}

void LED2GREEN_off(void)
{
	led_gpio_ptr->PUAPAR &= 0x3F;
	led_gpio_ptr->DDRUA |= 0x08;//(0x04|(0x01<<3));
	LED2GREEN_OFF;
}

void LED2GREEN_on(void)
{
	led_gpio_ptr->PUAPAR &= 0x3F;
	led_gpio_ptr->DDRUA |= 0x08;
	LED2GREEN_ON;
}

void RfLedOn(void)
{
	CLEAR_LED12;
//	SET_reset_pin;
}
void RfLedOff(void)
{
	SET_LED12;
//	CLEAR_reset_pin;
}
void GPIO_PULLDOWN(void)
{
    led_gpio_ptr->PUBPAR &= 0xFFFC;
	led_gpio_ptr->DDRUB |= (0x04|(0x01<<0));
	led_gpio_ptr->PORTUB &= 0xFE;
}
void RESET_CC2530(void)
{
    //	uart2_dev  = fclose(UART2_CHANNEL, NULL); 
    led_gpio_ptr->PUCPAR &= 0xFB;	//D12-PUC2
	led_gpio_ptr->DDRUC |= (0x04|(0x01<<1));//Lewis.Chen--CC2530ResetPin
  // 	led_gpio_ptr->DDRUC |= (0x04|(0x01<<1));
	CLEAR_reset_pin;
	printf("\npull down cc2530\n");
}

void N_RESET_CC2530(void)
{
    
	led_gpio_ptr->PUCPAR &= 0xFB;	//D12-PUC2
	led_gpio_ptr->DDRUC |= (0x04|(0x01<<1));//Lewis.Chen--CC2530ResetPin
	SET_reset_pin;
}

void EthernetLedOn(void)
{
	CLEAR_LED13;
}
void EthernetLedOff(void)
{
	SET_LED13;
}

void DimoLedOn(void)
{
	CLEAR_LED28;
}
void DimoLedOff(void)
{
	SET_LED28;
}

void RunLedOn(void)
{
	CLEAR_LED11;
}
void RunLedOff(void)
{
	SET_LED11;
}
void RunLedBlink(void)
{
	BLINK_LED11;
}
void SdLedOn(void)
{
	CLEAR_LED9;
}
void SdLedOff(void)
{
	SET_LED9;
}

void LedOutPut(DICOR_Led_t signal, boolean state)
{
	if (state)
	{
		switch (signal)
		{
			case DICOR_RF_LED:
				RfLedOn();
			break;
			case DICOR_ETHERNET_LED:
				EthernetLedOn();
			break;
			case DICOR_DIMO_LED:
				DimoLedOn();
			break;
			case DICOR_ALIVE_LED:
				RunLedOn();
			break;
			case DICOR_SD_LED:
				SdLedOn();
			break;
		}
	}
	else
	{
		switch (signal)
		{
			case DICOR_RF_LED:
				RfLedOff();
			break;
			case DICOR_ETHERNET_LED:
				EthernetLedOff();
			break;
			case DICOR_DIMO_LED:
				DimoLedOff();
			break;
			case DICOR_ALIVE_LED:
				RunLedOff();
			break;
			case DICOR_SD_LED:
				SdLedOff();
			break;
		}	
	}
}


/* EOF */
