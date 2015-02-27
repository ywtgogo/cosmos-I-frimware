/***********************************************************
* 
* 	Copyright (c) 2011 ;
* 	All Rights Reserved convertergy
* 	FileName: dicor_spi_dri.c
* 	Version : 
* 	Date    : 
*
* Comments:
*      
************************************************************/
#include <mqx.h>
#include <spi.h>
//#include <bsp.h>
//#include <psp_cpu.h>
//#include <io_gpio.h>
#include <mcf5225.h>

#include "dicor_spi_dri.h"
#include <network_frame.h>
#include "network_phy.h"
#include "dicor_upload.h"
#include "led.h"


#define RF_QSPI_BAUDRATE               (500000)//500K
const MCF5XXX_QSPI_INIT_STRUCT _rf_qspi0_init = { 
	   0,							 /* SPI channel 					*/
	   MCF5XXX_QSPI_QDR_QSPI_CS0,	 /* Chip Select 0					*/
	   SPI_DEVICE_MASTER_MODE,		 /* Transfer mode				*/
	   RF_QSPI_BAUDRATE,			 /* SPI Baud rate register value	*/
	   BSP_SYSTEM_CLOCK/2,			 /* Clock Speed 					*/
	   SPI_CLK_POL_PHA_MODE0,		 /* SPI clock phase 				*/
	   BSP_QSPI_RX_BUFFER_SIZE, 	 /* Rx Buffer Size (interrupt only) */
	   BSP_QSPI_TX_BUFFER_SIZE		 /* Tx Buffer Size (interrupt only) */
};

#define MCF5XXX_QSPI_QDR_TX (0x0001)
#define MCF5XXX_QSPI_QDR_RX (0x0002)

//the pointer to register address
MCF5225_GPIO_STRUCT_PTR rf_gpio_ptr;
static  VMCF5XXX_QSPI_STRUCT_PTR  rf_qspi_ptr;
VMCF522XX_EPORT_STRUCT_PTR RF_eport_ptr;

//extern LWSEM_STRUCT	rfnet_sem;

static void TX_waite(void)  // for 16bit buffer is used for 8bit transmite
{
	uint_8 i; 
//	while(!getRegBit(QSPI0_SCTRL,SPTE)){};
	//for(i=0;i<120;i++);
	for(i=0;i<160;i++)
	{
		
	};
}

#if 0
#define Init_RfContIo() \
                         {\
                           rf_gpio_ptr->PUCPAR &= 0x00;\
                           rf_gpio_ptr->PTCPAR &= 0xcf;\
                           rf_gpio_ptr->PUBPAR &= 0x3f;\
                           rf_gpio_ptr->PQSPAR =((rf_gpio_ptr->PQSPAR&0xff00)+0x0015);\
                           rf_gpio_ptr->DDRUC = 0x09;\
                           rf_gpio_ptr->DDRTC &= 0xF7;\
	                       rf_gpio_ptr->DDRUB |= 0x08;\
	                       rf_gpio_ptr->DDRQS |= 0x08;\
                           }
#endif 
/*硬件V2.2以后有变*/
#define Init_RfContIo() \
                         {\
                         	rf_gpio_ptr->PUCPAR &= 0x07;\
                           rf_gpio_ptr->PTAPAR &= 0xd3;\
                           rf_gpio_ptr->PTCPAR &= 0xcf;\
                           rf_gpio_ptr->PUBPAR &= 0x3f;\
                           rf_gpio_ptr->DDRUC |= 0x08;\
                           rf_gpio_ptr->DDRTA |= 0x04;\
                           rf_gpio_ptr->DDRTC &= 0xF7;\
	                       rf_gpio_ptr->DDRUB |= 0x08;\
                         }







#if 0

//static void _rf_qspi_polled_init(void);
uint_32  _rf_qspi_polled_rx(uchar_ptr, uint_32);
uint_32  _rf_qspi_polled_tx( uchar_ptr, uint_32);
uint_32  _rf_qspi_polled_tx_rx( uint_8_ptr, uint_8_ptr, uint_32, uint_16);

#endif

void RfNwkTimerInstall(void);
void RfNWKDrIsrInstall(void);
static void rf_dr_int_hdl(pointer param);
void  _rf_qspi_polled_init(void);
void test_rf_io_pin(void);


void test_rf_io_pin(void)
{

  rf_gpio_ptr =(MCF5225_GPIO_STRUCT_PTR) _bsp_get_gpio_base_address();
  rf_qspi_ptr =(VMCF5XXX_QSPI_STRUCT_PTR)_bsp_get_qspi_base_address (_rf_qspi0_init.CHANNEL);
  Init_RfContIo();

  CLEAR_TRX_CE;
  SET_TRX_CE;
   CLEAR_TRX_CE;
		//SPI_CS0----(MCU21)-----PQS3---output
  SET_CSN;

  CLEAR_CSN;
  
  SET_CSN;
		
		//RF_PWR_UP---(MCU68)PUC0----output
  CLEAR_PWR;
  SET_PWR;
    CLEAR_PWR;
		//RF_TX_EN ---(MCU38)PUB3----output(1)
  CLEAR_TXEN;
  SET_TXEN;
 CLEAR_TXEN;		
	
//GET_DR_PIN();
		//RF_CD 	  ---(MCU69)PUC1--- input
//GET_CD_PIN();

}

#if 0
/*************************************************************
* 
* Function Name    : _rf_qspi_find_baudrate
* Returned Value   : uint_8 divider register setting
* Comments         :
*    Find closest setting of divider register for given baudrate.
*
**************************************************************/
//clock: Module input clock in Hz
//baudrate:	Desired baudrate in Hz 
static uint_32 _rf_qspi_find_baudrate (uint_32 clock, uint_32 baudrate) 
{
    uint_32 div = 0;
    
    if (0 != baudrate)
    {
        div = (clock + (baudrate >> 1)) / baudrate;
        if (div < 2) div = 2;
        if (div > 255) div = 255;
    }
    return MCF5XXX_QSPI_QMR_BAUD(div);
}  

/***********************************************************
* 
* Function Name    : 
* Returned Value   : 
* Comments         :
*    This function initializes the SPI module 
*
************************************************************/
void  _rf_qspi_polled_init(void)
{

	rf_gpio_ptr =(MCF5225_GPIO_STRUCT_PTR) _bsp_get_gpio_base_address();
	rf_qspi_ptr =(VMCF5XXX_QSPI_STRUCT_PTR)_bsp_get_qspi_base_address (_rf_qspi0_init.CHANNEL);
	Init_RfContIo();

	
   #if 0
    if (_bsp_qspi_io_init (_rf_qspi0_init->CHANNEL) == -1)
    {
        return SPI_ERROR_CHANNEL_INVALID;
    }
   #endif
    /* Disable and clear SPI */
    rf_qspi_ptr->QIR = ((MCF5XXX_QSPI_QIR_WCEF) |( MCF5XXX_QSPI_QIR_ABRT)
                         | (MCF5XXX_QSPI_QIR_SPIF));
    rf_qspi_ptr->QWR = ((MCF5XXX_QSPI_QWR_HALT) | (MCF5XXX_QSPI_QWR_CSIV)
		                 | (MCF5XXX_QSPI_QWR_NEWQP(0)) | (MCF5XXX_QSPI_QWR_ENDQP(0)));
    while (rf_qspi_ptr->QDLYR & MCF5XXX_QSPI_QDLYR_SPE) 
       {};
    rf_qspi_ptr->QIR = ((MCF5XXX_QSPI_QIR_WCEF) | (MCF5XXX_QSPI_QIR_ABRT)
		                 | (MCF5XXX_QSPI_QIR_SPIF));
    
    
    /* Set the SPI clock baud rate divider */
    rf_qspi_ptr->QMR = ((_rf_qspi_find_baudrate (_rf_qspi0_init.CLOCK_SPEED, 
                          _rf_qspi0_init.BAUD_RATE)) |( MCF5XXX_QSPI_QMR_BITS_SET(8)));

    /* Set up SPI clock polarity and phase */
    switch (_rf_qspi0_init.CLOCK_POL_PHASE)
    {
        case (SPI_CLK_POL_PHA_MODE0):
            /* Inactive state of SPI_CLK = logic 0 */
            rf_qspi_ptr->QMR &= (~ MCF5XXX_QSPI_QMR_CPOL);
            /* SPI_CLK transitions middle of bit timing */
            rf_qspi_ptr->QMR &= (~ MCF5XXX_QSPI_QMR_CPHA);
            break;
        case (SPI_CLK_POL_PHA_MODE1):
            /* Inactive state of SPI_CLK = logic 0 */
            rf_qspi_ptr->QMR &= (~ MCF5XXX_QSPI_QMR_CPOL);
            /* SPI_CLK transitions begining of bit timing */
            rf_qspi_ptr->QMR |= MCF5XXX_QSPI_QMR_CPHA;
            break;
        case (SPI_CLK_POL_PHA_MODE2):
            /* Inactive state of SPI_CLK = logic 1 */
            rf_qspi_ptr->QMR |= MCF5XXX_QSPI_QMR_CPOL;
            /* SPI_CLK transitions middle of bit timing */
            rf_qspi_ptr->QMR &= (~ MCF5XXX_QSPI_QMR_CPHA);
            break;
        case (SPI_CLK_POL_PHA_MODE3):
            /* Inactive state of SPI_CLK = logic 1 */
            rf_qspi_ptr->QMR |= MCF5XXX_QSPI_QMR_CPOL;
            /* SPI_CLK transitions begining of bit timing */
            rf_qspi_ptr->QMR |= MCF5XXX_QSPI_QMR_CPHA;
            break;
        default:
            return ;       
    }
    /* Set transfer mode */
    if (_rf_qspi0_init.TRANSFER_MODE == SPI_DEVICE_MASTER_MODE)
    {
        rf_qspi_ptr->QMR |= MCF5XXX_QSPI_QMR_MSTR;
    }

    /* Enable SPI (clear HALT) */
    rf_qspi_ptr->QWR &= (~ MCF5XXX_QSPI_QWR_HALT);


 
    
}

#endif

/***********************************************************
* 
* Function Name    : 
* Returned Value   : 
* Comments         :
*    This function initializes the SPI module 
*
************************************************************/
void  _rf_qspi_polled_init(void)
{

	rf_gpio_ptr =(MCF5225_GPIO_STRUCT_PTR) _bsp_get_gpio_base_address();
	rf_qspi_ptr =(VMCF5XXX_QSPI_STRUCT_PTR)_bsp_get_qspi_base_address (_rf_qspi0_init.CHANNEL);
	Init_RfContIo();   
}

#if 0  
/*****************************************************************
* 
* Function Name    : _rf_qspi_polled_tx_rx
* Returned Value   : byte received   
* Comments         : 
*   Actual transmit and receive function.
*
******************************************************************/

uint_32 _rf_qspi_polled_tx_rx(uint_8_ptr buffer_in,uint_8_ptr buffer_out,
                                              uint_32  size,
                                              uint_16 command)
{
    uint_16   data;
    uint_32   num, bytes;
      /* Set transfer size */
    bytes = 1;
    if (MCF5XXX_QSPI_QMR_BITS_GET(rf_qspi_ptr->QMR) != 8) 
	{
	  bytes++;
    }  
    if (size > (16 << (bytes - 1))) 
	{
	   size = (16 << (bytes - 1));
    }   
    if (bytes > size) 
	{	return 0 ;}

    /* Assert actual chip select */ 
#if 1
   ;

#else
    if (io_info_ptr->CS ^ io_info_ptr->CS_ACTIVE)
    {
        for (num = 0; num < MCF5XXX_QSPI_CS_COUNT; num++)
        {
            if ((0 != ((io_info_ptr->CS ^ io_info_ptr->CS_ACTIVE) & (1 << num))) 
				    && (NULL != io_info_ptr->CS_CALLBACK[num]))
            {
                io_info_ptr->CS_CALLBACK[num] (1 << num, 
					     (io_info_ptr->CS_ACTIVE >> num) & 1, io_info_ptr->CS_USERDATA[num]);
            }
        }
        io_info_ptr->CS_ACTIVE = io_info_ptr->CS;
    }
#endif 
    /* Write commands */
    rf_qspi_ptr->QAR = 0x0020;
    data = command & 0xFF00;
    for (num = 0; num < size; num += bytes)
    {
        rf_qspi_ptr->QDR = data;
    }

    /* Write data */
    rf_qspi_ptr->QAR = 0x0000;
    if (bytes > 1)
    {
        for (num = 0; num < size; num += bytes)
        {
            data = *buffer_in++;
            data = (data << 8) | *buffer_in++;
            rf_qspi_ptr->QDR = data;
        }
        data = (size >> 1);
    }
    else
    {
        for (num = 0; num < size; num += bytes)
        {
            data = *buffer_in++;
            rf_qspi_ptr->QDR = data;
        }
        data = size;
    }
  //  CLEAR_CSN;  

    /* Setup queue */
    rf_qspi_ptr->QWR &= (~(MCF5XXX_QSPI_QWR_NEWQP(0xF) | MCF5XXX_QSPI_QWR_ENDQP(0xF)));
    rf_qspi_ptr->QWR |= MCF5XXX_QSPI_QWR_NEWQP(0) | MCF5XXX_QSPI_QWR_ENDQP(data - 1);
     
    /* Initiate transmission */
    rf_qspi_ptr->QDLYR |= MCF5XXX_QSPI_QDLYR_SPE;
#if 1
;
#else
    /* If keeping CS low is needed between transfers */
    if (io_info_ptr->CSIV_ACTIVE)
    {
        /* CS inactive low for tranfers longer than 16 frames */
        rf_qspi_ptr->QWR &= (~ MCF5XXX_QSPI_QWR_CSIV);
    }
#endif

    /* Wait transmission end */
    while (! (rf_qspi_ptr->QIR & MCF5XXX_QSPI_QIR_SPIF)) 
        { };
//    SET_CSN_DATA;  
    rf_qspi_ptr->QIR |= MCF5XXX_QSPI_QIR_SPIF | MCF5XXX_QSPI_QIR_ABRT | MCF5XXX_QSPI_QIR_WCEF;

   
    if (command & MCF5XXX_QSPI_QDR_RX)
    {
        /* Read data */
        rf_qspi_ptr->QAR = 0x0010;
        if (bytes > 1)
        {
            for (num = 0; num < size; num += bytes)
            {
                data = rf_qspi_ptr->QDR;
                *buffer_out++ = (uint_8)(data >> 8);
                *buffer_out++ = (uint_8)data;
            }
        }
        else
        {
            for (num = 0; num < size; num += bytes)
            {
                data = rf_qspi_ptr->QDR;
                *buffer_out++ = (uint_8)data;
            }
        }
    }

       return num;
}





#endif
extern volatile UINT_8 RF_PhySendEnd;

static void rf_dr_int_hdl(pointer param)
{
  
	//_lwsem_post(&rfnet_sem);
   if(EMN_PHY_State == RX_MODE)
   {
   		if (RF_PhySendEnd == 1)
   		{
   	
        	dicor_rf_signal(RF_GET_DATA_END);
        	RF_PhySendEnd = 0;
   		}
        
   	RFLedNum++;
	 if((RF_RxReady==0)&&(RF_PhyGetDataEnd==1))  
     {
        RF_RxReady = 1;
	 }
	 else{
	 	 RF_RxOverflow=1;
	 	}
   } 
  RF_eport_ptr->EPFR |= 0x02; /* clear flag(s) */
  dicor_rf_signal(RFNET_EVENT);
  
}
void RfNWKDrIsrInstall(void)
{
	//install RF_DR interrupter   

	VMCF5225_STRUCT_PTR  mcf5225_ptr;
   //init I/O for EPORT. 
	rf_gpio_ptr->PNQPAR = ((rf_gpio_ptr->PNQPAR&0xfff3)|0x0004) ;
	rf_gpio_ptr->DDRNQ &= 0xfd;
	
    //init EPORT
    mcf5225_ptr = _PSP_GET_IPSBAR();
    RF_eport_ptr = &mcf5225_ptr->EPORT[0];
	RF_eport_ptr->EPDDR &= 0xfd; /*  irq1 is input */
    RF_eport_ptr->EPPAR =((RF_eport_ptr->EPPAR &0xfff3)|0x0004);/*  rasing edge */
    RF_eport_ptr->EPIER |=0x02; /* set interrupt enable */
	//install ISR
	_int_install_isr(MCF5225_INT_EPORT0_EPF1, 
	                   (void (_CODE_PTR_)(pointer))rf_dr_int_hdl, NULL);
	
	_mcf5225_int_init(MCF5225_INT_EPORT0_EPF1, BSP_EPORT0_EPF1_INT_LEVEL, 
					 BSP_EPORT_EPFX_INT_SUBLEVEL_MIDPOINT, 
					 TRUE);

}
/* Initialize the timer interrupt */

void RfNwkTimerInstall(void)
{
   
   _time_set_timer_vector(RFNWK_TIMER_INTERRUPT_VECTOR);
   _int_install_isr(RFNWK_TIMER_INTERRUPT_VECTOR,
      (void (_CODE_PTR_)(pointer))SYS_TimersISR, NULL) ;
   
    _mcf5225_timer_init_freq(RFNWK_TIMER,
	    RFNWK_TIMER_FREQUENCY, BSP_SYSTEM_CLOCK, FALSE);

    _mcf5225_timer_unmask_int(RFNWK_TIMER);
}
#if 0   //use lightweigh timer for ourself OS's timer
#include <lwtimer.h>
static LWTIMER_PERIOD_STRUCT    dicor_timer_queue;
static LWTIMER_STRUCT_PTR       EMN_Timer;
void EMN_TimerInstall(void)
{
	if (_lwtimer_create_periodic_queue(&dicor_timer_queue, 1, 0) != MQX_OK)
			   printf("create_perildic_queue failed !");
	
	 
	  EMN_Timer = (LWTIMER_STRUCT_PTR)_mem_alloc(sizeof(LWTIMER_STRUCT));
	  if (!EMN_Timer) 
	  {
	  
		 printf("create Timer_dt failed !");
	  } 
	  if(MQX_OK != _lwtimer_add_timer_to_queue(&dicor_timer_queue, EMN_Timer,0, SYS_TimersISR, NULL)) 
	  {
	    printf("add_timer failed!");
	  } 

}
#endif

extern void PMUInit(void);
void dicor_dri_init(void)
{
 // test_rf_io_pin();
	
	
  _rf_qspi_polled_init();	
  //rf_gpio_ptr =(MCF5225_GPIO_STRUCT_PTR) _bsp_get_gpio_base_address();
  RfNWKDrIsrInstall(); 
  RfNwkTimerInstall();
  PMUInit();
//  EMN_TimerInstall();

}



