#ifndef _network_rf_dri_h_
#define _network_rf_dri_h_
/***********************************************************
* 
* 	Copyright (c) 2011  Convertergy;
*	 All Rights Reserved
*	 FileName: network_rf_dri.h
*	 Version : 
*	 Date    : 
* 	 Ower   : peter li
*
*      Comments:
*
**************************************************************/
#include <mqx.h>
#include <bsp.h>
#include "dicor_spi_dri.h"
#include "network_phy.h"

/***********************
 *  definitions of RF905 driver
 ************************/
// command for nRF905's operation by means of SPI, any command start when 
// CSN pin switch from high to low.
#define RF_WC      0x00     // write RF-configuration Register
#define RF_RC      0x10     // read  RF-configuration Register

#define RF_WRA     0x05     // write RX address and crc
#define RF_RRA     0x15  	// read rx address and crc


#define RF_WTP     0x20     // write transmition data to TX-payload register
#define RF_RTP     0x21  	// read transmition data from TX-payload register

#define RF_WTA     0x22  	// write transmittion address to TX-address register 
#define RF_RTA     0x23  	// read transmittion address from TX-address register 

#define RF_RRP     0x24   	// read data from RX-payload register

/*nRF905 configration reg's parameters*/
typedef struct 
{
  UINT_8  n;
  UINT_8  buf[10]; 
}EMN_RF_CONFIG;



#define SET_RX_MODE()     { CLEAR_TXEN;\
							SET_TRX_CE; \
							EMN_PHY_State = RX_MODE; \
							_time_delay(1);}  //delay for mode change(>=650us)
								
#define SET_TX_MODE()     { CLEAR_TRX_CE;\
							SET_TXEN; \
							EMN_PHY_State = TX_MODE; \
							_time_delay(1);\
							}  //delay for mode change(>=650us)
								
		
#define SET_STANDBY_MODE()  { CLEAR_TRX_CE; \
							  EMN_PHY_State = STANDBY_MODE;\
							  _time_delay(1);  \
							}//waiter for 650us

extern const EMN_RF_CONFIG EMN_ScanRFConf;
extern EMN_RF_CONFIG RFConf;


extern void EMN_PHY_InitNrf905(void);
extern void EMN_ScanInitNrf905(void);
extern UINT_8 EMN_TestTxAddr(UINT_8 net_addr,UINT_8 sub_addr);
extern UINT_8 RFSpiReadWrite(MQX_FILE_PTR fd, UINT_8* sendbuf, UINT_8* recvbuf, UINT_8 len);



#endif

