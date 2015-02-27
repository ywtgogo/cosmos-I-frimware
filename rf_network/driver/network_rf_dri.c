/****************************************************************
* 
* 	Copyright (c) 2010 ;
* 	All Rights Reserved convertergy
* 	FileName: network_rf_dri.c
* 	Version : 
* 	Date    : 
*
* Comments:
*      
*
*******************************************************************/
//#define  EMN_EMT_ONLY  1
#include <network_frame.h>
#if EMN_EMT_ONLY
#include "dicor_spi_dri.h"
#include "network_rf_dri.h"
#include "network_phy.h"
#else
#include "didi_spi_dri.h"
#include <Cpu.h>
#endif

#include <mqx.h>
#include <bsp.h>
#include <spi.h>
extern void Dicor_Reboot(void);



EMN_RF_CONFIG RFConf = {
			10,
	//	 XX 	   :		0			  :0				  :11		 : 0	  001001100  (430MHZ)
	//	 reserved:		   No re_TX :RX_RED_PWR :PA,10db:  PLL	 Channel	
			0x4c,  // for Mewmsg module, should use this value for its HW.just as 430MHz
			0x0c,
	//	X100				: X100
	//	TX address len	 :	RX addr len
			0x44,
	
			0x20,  // rx data len
			0x20,  // TX data len
	// RX address		
			0x00,  //RX_addr[0]
#if DICOR_STATIC_REG_TAB			
			0x00,  //RX_addr[1]
		0x00,//DICOR_NET_ADDR,  //RX_addr[2]
#else			
			0xFF,  //RX_addr[1]
			0xFF,  //RX_addr[2]
#endif			
			EMN_CONVERTERGY_NO,  //RX_addr[3]
	//			0			  : 	  1 	  : 	 011(16M)  :   0				   :00
	//CRC 0-8 1-16bits:    CRC_EN : crystal freq	 :out clock enable	: output clock
			0x58	//
};



const EMN_RF_CONFIG EMN_ScanRFConf = {
			10,
	//	 XX 	   :		0			  :0				  :11		 : 0	  001001100  (430MHZ)
	//	 reserved:		   No re_TX :RX_RED_PWR :PA,10db:  PLL	 Channel	
			0x4c,  // for Mewmsg module, should use this value for its HW.just as 430MHz
			0x0c,
	//	X100				: X100
	//	TX address len	 :	RX addr len
			0x44,
	
			EMN_SCAN_FRM_LEN,  // rx data len
			EMN_SCAN_FRM_LEN,  // TX data len
	// scan network address :RX address		
			EMN_CONVERTERGY_NO,  //RX_addr[0]
			0xFF,  //RX_addr[1]
			0xFF,  //RX_addr[2]
			EMN_CONVERTERGY_NO,  //RX_addr[3]
	//			0			  : 	  1 	  : 	 011(16M)  :   0				   :00
	//CRC 0-8 1-16bits:    CRC_EN : crystal freq	 :out clock enable	: output clock
			0x58	//
};
		

SPI_READ_WRITE_STRUCT  rw;
MQX_FILE_PTR           spifd;
extern LWSEM_STRUCT	spi_sem;



/**************************************************
* 
* Function Name    : 
* Returned Value   : 
* Comments          : SPI读和写
*  
*
******************************************************/
UINT_8 RFSpiReadWrite(MQX_FILE_PTR fd, UINT_8* sendbuf, UINT_8* recvbuf, UINT_8 len)
{
	_lwsem_wait(&spi_sem);

	rw.BUFFER_LENGTH = len;
	rw.WRITE_BUFFER = (char_ptr)sendbuf;
	rw.READ_BUFFER = (char_ptr)recvbuf;
//	if (SPI_OK == ioctl (fd, IO_IOCTL_SPI_READ_WRITE, &rw))
//	{
//		;//printf ("OK\n");
//	}
//	else
//	{
//		printf ("SPI ERROR\n");
	//	_lwsem_post(&spi_sem);
//		return 1;
//	}
	_lwsem_post(&spi_sem);
	return 0;
}


//#if 1
/**************************************************
* 
* Function Name    : 
* Returned Value   : 
* Comments          : Init nRF905 configeration registers.
*  
*
******************************************************/
static UINT_8  PHY_TestNrf905(void)
{

  UINT_8 i,result;
  uchar send_buffer[32];
  uchar recv_buffer[32];

//	uint_32                param; 
	result =0;
	send_buffer[0] = RF_RC;
   	RFSpiReadWrite(spifd, send_buffer, recv_buffer, 1);

   
  //_SpiWriteByte(RF_RC);
    #if (EMN_EMT_ONLY)
  	RFSpiReadWrite(spifd, send_buffer, recv_buffer, RFConf.n);
  	#else
	  RFSpiReadWrite(spifd, send_buffer, recv_buffer, RFConf.n+1);
	#endif
  fflush(spifd);
  for(i=0; i<RFConf.n; i++)
  {
  #if (EMN_EMT_ONLY) 
   // if(recv_buffer[i]!=RFConf.buf[i])
  //#else
//	if(recv_buffer[i+1]!=RFConf.buf[i])
  //#endif		
//	{
       result=0;
	 //  break;
	}   
  }
#if 0  
   //test  rx address
	  CLEAR_CSN;	  // enable SPI
	  _SpiWriteByte(RF_WRA);
	  _SpiWriteByte(0x00);
	  _SpiWriteByte(0x00);
	  _SpiWriteByte(0x1a);
	  _SpiWriteByte(EMN_CONVERTERGY_NO); //compony number
	  _SpiWriteByte(0x58);
	  SET_CSN_DATA;  

		for(i=0;i<15;i++)
		  tmp[i]=0xff;
		CLEAR_CSN;		// enable SPI
		_SpiWriteByte(RF_RRA);
		for(i=0; i<4; i++)
		{
		  tmp[i]=_SpiReadByte();
		}
		SET_CSN;
		//if(tmp[0]==0x00&&tmp[1]==0x00&&
		 //  tmp[2]==0x1a&&tmp[3]==EMN_CONVERTERGY_NO)
	//	{
		  result=0;
	//	}else  result =1;
//test TX address
  CLEAR_CSN;	  // enable SPI
	  _SpiWriteByte(RF_WTA);
	  _SpiWriteByte(0x00);
	  _SpiWriteByte(0x01);
	  _SpiWriteByte(0x1a);
	  _SpiWriteByte(EMN_CONVERTERGY_NO); //compony number
	  SET_CSN_DATA;  

		for(i=0;i<15;i++)
		  tmp[i]=0xff;
		CLEAR_CSN;		// enable SPI
		_SpiWriteByte(RF_RTA);
		for(i=0; i<4; i++)
		{
		  tmp[i]=_SpiReadByte();
		}
		SET_CSN_DATA;
	//	if(tmp[0]==0x00&&tmp[1]==0x01&&
	//	   tmp[2]==0x1a&&tmp[3]==EMN_CONVERTERGY_NO)
	//	{
		  result=0;
	//	}	else  result =1;
#endif 		
 // return result; 
  
//}

#endif






#if 0
UINT_8 EMN_TestTxAddr(UINT_8 net_addr,UINT_8 sub_addr)
{
  UINT_8 tmp[5],i,result;
  result = 0;
  for(i=0;i<5;i++) 	tmp[i]=0xff;

  // enable SPI
  CLEAR_CSN;	  
  _SpiWriteByte(RF_RTA);
  for(i=0; i<4; i++)
  {
	tmp[i]=_SpiReadByte();
  }
  SET_CSN_DATA;

  // 
  //if(tmp[0]==0x00&&tmp[1]==sub_addr&&
//	 tmp[2]==net_addr&&tmp[3]==EMN_CONVERTERGY_NO)
  
  
  //{
  	result=0;
  	// }	
 // else  { result =1; }

  return (result);

}
#endif
#if 0
void EMN_ScanInitNrf905(void)
{
  UINT_8  i;
  //enter standby mode  
  CLEAR_TRX_CE; 
  CLEAR_TXEN;   
  
// write nRF905 configeration registers
  CLEAR_CSN;	// enable SPI  
  _SpiWriteByte(RF_WC);
  for(i=0; i<EMN_ScanRFConf.n; i++)
  {
	_SpiWriteByte(RFConf.buf[i]);
  }
  SET_CSN_DATA;

}
#endif
#if 0
void EMN_PHY_InitNrf905(void)
{
  UINT_8  i;
  
  dicor_dri_init();

 //  SYS_ResetTrg();  
  SET_CSN;	
  //enter standby mode  
  SET_PWR;  
#if (EMN_EMT_ONLY)  
  _time_delay(4);
#else
   Cpu_Delay100US(40);
#endif
  CLEAR_TRX_CE; 
  CLEAR_TXEN;   
// write nRF905 configeration registers
//  SYS_SendTrg();
//while(1)
//{
//    CLEAR_CSN;
//	_SpiWriteByte(0xaa);
//	SET_CSN_DATA;

//}
  CLEAR_CSN;	// enable SPI
  
  _SpiWriteByte(RF_WC);
  for(i=0; i<RFConf.n; i++)
  {
	_SpiWriteByte(RFConf.buf[i]);
  }
  SET_CSN_DATA;


   if(PHY_TestNrf905())
   {
     SYS_Error(8);
   }

}
#endif

#if 1
//add by dick
void EMN_PHY_InitNrf905(void)
{
//  UINT_8  i;
  uchar   send_buffer[32];
  uchar   recv_buffer[32];
  uchar power;
  uint_32 freq;
  uint_32 temp;
  uint_32 ch;

 RFConf.buf[7] = pBaseConfig->net_addr;
 //计算频率和功率选择
if (pBaseConfig->rfpower > 3)
{
	power = 3;
}
else
{
	power = pBaseConfig->rfpower;
}

RFConf.buf[1] =  power<<2;

freq = pBaseConfig->rffreq[0];
freq <<= 24;
temp = pBaseConfig->rffreq[1];
temp <<= 16;
freq += temp;
temp = pBaseConfig->rffreq[2];
temp <<= 8;
freq += temp;
freq += pBaseConfig->rffreq[3];
if (freq < 400000 || freq > 1000000)
{
	freq = 430000;
}
if (freq > 800000)
{
	RFConf.buf[1] |= 0x02;
	ch = 10*(freq/2-422400);
}
else
{
	ch = 10*(freq-422400);
}
ch /= 1000;
RFConf.buf[0] = (uchar)ch;
if (ch & 0x100)
{
	RFConf.buf[1] |= 0x01;
}

  //uint_32                param;  
   /* Open the SPI controller */      
    
  spifd = fopen ("spi0:", NULL);
   if (NULL == spifd) 
   {
      printf ("Error opening SPI driver!\n");
      _time_delay (200L);
     // _task_block ();
     Dicor_Reboot();
   }

   //printf ("IO_IOCTL_SPI_READ_WRITE ... ");
   dicor_dri_init();

	
  
 //  SYS_ResetTrg();  
 // SET_CSN;	
  //enter standby mode  
  SET_PWR;  
#if (EMN_EMT_ONLY)  
  _time_delay(4);
#else
   Cpu_Delay100US(40);
#endif
  CLEAR_TRX_CE; 
  CLEAR_TXEN;   
// write nRF905 configeration registers
//  SYS_SendTrg();
//while(1)
//{
//    CLEAR_CSN;
//	_SpiWriteByte(0xaa);
//	SET_CSN_DATA;

//}
  //CLEAR_CSN;	// enable SPI
	
   send_buffer[0] = RF_WC;
   RFSpiReadWrite(spifd, send_buffer, recv_buffer, 1);

   RFSpiReadWrite(spifd, (UINT_8*)RFConf.buf, recv_buffer, RFConf.n);
   //fflush(spifd);
  //_SpiWriteByte(RF_WC);
 /*rw.BUFFER_LENGTH = 16;
  for(i=0; i<RFConf.n/rw.BUFFER_LENGTH; i++)
  {
	//_SpiWriteByte(RFConf.buf[i]);
	//send_buffer[0] send_buffer= RFConf.buf[i];
	rw.WRITE_BUFFER = (char_ptr)(RFConf.buf + rw.BUFFER_LENGTH*i);
  	if (SPI_OK == ioctl (spifd, IO_IOCTL_SPI_READ_WRITE, &rw)) 
   {
      ;//printf ("OK\n");
   } else {
      printf ("SPI ERROR\n");
   }
  }
  if (RFConf.n % rw.BUFFER_LENGTH != 0)
  	{
  		rw.WRITE_BUFFER = (char_ptr)(RFConf.buf + (RFConf.n/rw.BUFFER_LENGTH)*rw.BUFFER_LENGTH);
  		rw.BUFFER_LENGTH = RFConf.n % rw.BUFFER_LENGTH;

  	if (SPI_OK == ioctl (spifd, IO_IOCTL_SPI_READ_WRITE, &rw)) 
   {
      ;//printf ("OK\n");
   } else {
      printf ("SPI ERROR\n");
   }
  	}
  //SET_CSN_DATA;*/

   fflush(spifd);
   if(PHY_TestNrf905())
   {
     SYS_Error(8);
   }
   else
   	{
   		printf("\r\n--------RFSPI OK!----------\r\n");
   	}
}

void RFWorkReserve(uint_32 f);
void RFWorkReserve(uint_32 f)
{
  uchar   send_buffer[32];
  uchar   recv_buffer[32];
  uchar power;
  uint_32 freq;
  uint_32 ch;
  
  RFConf.buf[7] = 0xFE;
  power = 3;
  RFConf.buf[1] =  power<<2;


	freq = f;

	ch = 10*(freq-422400);

	ch /= 1000;	RFConf.buf[0] = (uchar)ch;
	if (ch & 0x100)
	{
		RFConf.buf[1] |= 0x01;
	}

  

  SET_PWR;  
#if (EMN_EMT_ONLY)  
  _time_delay(4);
#else
   Cpu_Delay100US(40);
#endif
  CLEAR_TRX_CE; 
  CLEAR_TXEN;   

	
   send_buffer[0] = RF_WC;
   RFSpiReadWrite(spifd, send_buffer, recv_buffer, 1);

   RFSpiReadWrite(spifd, (UINT_8*)RFConf.buf, recv_buffer, RFConf.n);

   fflush(spifd);
   if(PHY_TestNrf905())
   {
     SYS_Error(8);
   }
   else
   	{
   		printf("\r\n--------RFSPI OK!----------\r\n");
   	}
}




#endif

/* EOF */

