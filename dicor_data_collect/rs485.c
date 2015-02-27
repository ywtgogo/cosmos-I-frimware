/*************************************************************
* 
* Copyright (c) convertergy ;
* All Rights Reserved
* FileName: rs485.c
* Version : 
* Date    : 2011/09/29
* ower    : Younger
*
* Comments:RS485读写初始化操作
*
*
***************************************************************/

#include <mqx.h>
#include <string.h>
#include "dicor.h"
#include "rs485.h"

extern _mqx_int  _io_serial_int_ioctl(MQX_FILE_PTR, _mqx_uint, pointer);
extern _mqx_int  _mcf52xx_uart_serial_polled_ioctl(MCF52XX_UART_SERIAL_INFO_STRUCT_PTR, uint_32, uint_32_ptr);

//PWM6---RS485_T_S-----PTC3-----19脚
//QSPI_S3---RS485_R_S-----PQS6----22脚

#define RS485_RX()		{rs485_gpio_ptr->PORTUB &= 0xFD; \
							rs485_gpio_ptr->PUBPAR |= 0x0040;}
#define RS485_TX()		{rs485_gpio_ptr->PORTUB &= 0xFE; \
						rs485_gpio_ptr->PUBPAR |= 0x0001;}

/*#define RS485_RX()		{rs485_gpio_ptr->PORTTC &= 0xF7;}
#define RS485_TX()		{rs485_gpio_ptr->PORTTC |= 0x08;}*/

MCF5225_GPIO_STRUCT_PTR rs485_gpio_ptr;
MCF52XX_UART_STRUCT_PTR uart1_ptr;




/* set RS485 output device */
#define RS485_CHANNEL "ittyb:"

MQX_FILE_PTR rs485_dev = NULL;


uchar  RS485_Data_Buffer[1024];



void rs485_init(void)
{	
	rs485_gpio_ptr = (MCF5225_GPIO_STRUCT_PTR) _bsp_get_gpio_base_address();


	RS485_RX();
	RS485_TX();
	
   /* HW 485 flow not available on chip */
   //非阻塞模式
   rs485_dev  = fopen( RS485_CHANNEL, (pointer) (IO_SERIAL_NON_BLOCKING) );
   
   if( rs485_dev == NULL )
   {
      /* device could not be opened */
     	printf("rs485 device could not be opend\r\n");
   }
   printf("\nRS485 init!\n");
  // ioctl( rs485_dev, IO_IOCTL_SERIAL_SET_FLAGS, &flag);
	rs485_clear_stats();	
}

void rs485_clear_buf(void)
{
	_io_serial_int_ioctl(rs485_dev, IO_IOCTL_SERIAL_CLEAR_BUF, NULL);
}

void rs485_clear_stats(void)
{	
//	uart1_ptr = (MCF5225_GPIO_STRUCT_PTR) _bsp_get_serial_base_address(1);
//	uart1_ptr->
	//_mcf52xx_uart_serial_polled_ioctl("ittyb", IO_IOCTL_SERIAL_CLEAR_STATS, NULL);
	
}
/*TASK*-----------------------------------------------------
* 
* Task Name    : rs485_write_task
* Comments     :
*    This task send data_buffer to rs485
*
*END*-----------------------------------------------------*/
void rs485_write(uchar* data_buffer, _mqx_int len)
{
	int i;
	volatile uint_8* p;
	volatile uint_8* pUimr1;
	p = (uint_8*)0x40000248;//UCR1
	pUimr1 = (uint_8*)0x40000254;//UIMR1
	
/*	for (i=0; i<10000; i++)
	{
		asm(nop);
	}*/
	
	_time_delay(150);
	
	//禁止接收
	*p &= 0xFC;
	*p |= 0x02;
	*pUimr1 &= 0xFD;
	
	for (i=0; i<1000; i++)
	{
		asm(nop);
	}
	RS485_TX();
	/*
	for (i=0; i<1000; i++)  //2014/4.21 //younger
	{
		asm(nop);
	}
	*/   
//	_int_enable();
	
//	_time_delay(10);
	write( rs485_dev, data_buffer, len);
	/* empty queue - not needed for polled mode */
	fflush( rs485_dev);
//	_time_delay(5);
//	_time_delay(3);
//1500不行，3000可以
	for (i=0; i<2800; i++)  //3500  //2014/4.17 //younger
	{
		asm(nop);
	}
//	_int_disable();
	RS485_RX();
	
	/*
	for (i=0; i<100; i++)  // 2014.4.17
	{
		asm(nop);          //younger
	}
	*/
	//恢复接收
	
	*p &= 0xFC;
	*p |= 0x01;
	*pUimr1 |= 0x02;
}

/*TASK*-----------------------------------------------------
* 
* Task Name    : rs485_read_task
* Comments     :
*    This task send data_buffer to rs485
*
*END*-----------------------------------------------------*/
_mqx_int rs485_read(uchar* data_buffer)
{
   _mqx_int  result;
	memset(data_buffer, 0, 1024);
	result = read(rs485_dev, data_buffer, 1024);
//	fflush( rs485_dev );
	if (result == -1)
	{
		printf("read error!\n");
	}
	return result;
}

/* EOF */
