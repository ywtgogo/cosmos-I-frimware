/**********************************************************************
* 
* 	Copyright (c) 2010 ;
* 	All Rights Reserved convertergy
* 	FileName: network_phy.c
* 	Version : 
* 	Date    : 
*
* Comments:
*      
*
*************************************************************************/
#include "network_phy.h"
#include "network_dlc.h"
#include "network_nwk.h"
#include "dicor_spi_dri.h"
#include "network_rf_dri.h"
#include "rs485.h"

//#define   CON_TX_TEST       // only send data device
//#define   CON_RX_TEST

/******************************
  *
  *****************************/
extern DIANDIDITABLE_PTR pDiAnDiDiTable;
static	 SYS_TIMER		EMN_PhyTimer;
EMN_PHY_RX_BUF          EMN_PHY_RxBuf; 

EMN_PHY_ST       EMN_PHY_State;
volatile UINT_8           RF_RxReady; 
volatile UINT_8 RF_PhySendEnd = 0;

UINT_8           RF_PhyGetDataEnd;

UINT_8           RF_RxOverflow;

extern MQX_FILE_PTR           spifd;

extern VMCF522XX_EPORT_STRUCT_PTR RF_eport_ptr;
extern UINT_8 GetSubDateFlag;

/******************************
 *
 ******************************/
static void Delay(uint_8 n)
{
	uint_8 i;
	while(n--)
	for(i=0;i<200;i++)
	{
	   asm(nop);
	}
}
#ifdef CON_TX_TEST
static void PHY_DataConTxTest(void)
{
   
  //send mssage request to PHY layer.  
  EMN_SendMsg.dest_id = EMN_PHY_TASK;
  EMN_SendMsg.msg_id = EMN_PHY_TX_REQ;
  EMN_SendMsg.data[0]= EMN_TXBUF_MACCOM; 
  EMN_SendMsg.data[1]= 1;  
  EMN_SendMsg.data[2] = EMN_NetAddr;
  EMN_SendMsg.data[3]= 0xff;  //Tx address  
  SYS_SendMsg(&EMN_SendMsg);
  
  // start TX timer
  EMN_PhyTimer.value = 1200;
  EMN_PhyTimer.msgID = SYS_TEST_CONTX;
  SYS_StartTimer(&EMN_PhyTimer); 

}
#else
#define PHY_DataConTxTest()         

#endif 



void PHY_WriteRxAddr(UINT_8 net_addr,UINT_8 subnet_adrr)
{
	uchar send_buffer[32];
  	uchar recv_buffer[32];
  EMN_PHY_ST	state;
  state =EMN_PHY_State;

  SET_STANDBY_MODE();
  
	// write nRF905 configeration registers
  	send_buffer[0] = RF_WRA;
	send_buffer[1] = 0x00;
	  send_buffer[2] = subnet_adrr;
	  send_buffer[3] = net_addr;
	  send_buffer[4] = EMN_CONVERTERGY_NO;
	  send_buffer[5] = 0x58;
	  RFSpiReadWrite(spifd, send_buffer, recv_buffer, 6);
	  fflush(spifd);
  
  if(state ==RX_MODE)
  {
  	SET_RX_MODE() ;
  }else if(state ==TX_MODE)
  	{
      SET_TX_MODE() ;
  	}
  	
}

void PHY_WriteFreq(UINT_8 data);
void PHY_WriteFreq(UINT_8 data)
{
	uchar send_buffer[32];
  	uchar recv_buffer[32];
  EMN_PHY_ST	state;
  state =EMN_PHY_State;

  SET_STANDBY_MODE();
  
	// write nRF905 configeration registers
  	send_buffer[0] = RF_WC;
	send_buffer[1] = data;
	  
	  RFSpiReadWrite(spifd, send_buffer, recv_buffer, 2);
	  fflush(spifd);
  
  if(state ==RX_MODE)
  {
  	SET_RX_MODE() ;
  }else if(state ==TX_MODE)
  	{
      SET_TX_MODE() ;
  	}
  	
}




/************************************************************
* 
* Function Name   : 
* Returned Value  : 
* Comments         : ISR will call this function, receive one packet every time 
*  
*
*************************************************************/
static void EMN_PHY_RxPacket(void)
{
	UINT_8 i;
  	uchar send_buffer[32];
  	uchar recv_buffer[32];	
  	if(GET_DR_PIN())       // data ready 
  	{
  		// enter standby mode for SPI operation
    	CLEAR_TRX_CE; 
   		// _time_delay(1);
    	Delay(5);
		send_buffer[0] = RF_RRP;
  		RFSpiReadWrite(spifd, send_buffer, recv_buffer, 1);

		//注意如果EMN_PHY_PAYLOAD_LEN>32需要分包收这里只考虑了小于等于32字节的情况
		RFSpiReadWrite(spifd, send_buffer, recv_buffer, EMN_PHY_PAYLOAD_LEN);
		fflush(spifd);
		for( i=0; i<EMN_PHY_PAYLOAD_LEN; i++)
		{
			EMN_PHY_RxBuf.data[EMN_PHY_RxBuf.tail][i] = recv_buffer[i];
		}
	
/*	printf("\r\n******************\r\n");
	for( i=0; i<32; i++)
    {
	  printf("%02X ", recv_buffer[i]);
	}
    
	printf("\r\n******************\r\n");*/
  //while(GET_DR_PIN()){};  // waite for both  DR and AM changing from high to low 
//	SET_TRX_CE;
  	}
  	SET_RX_MODE();
  	
  #if 0	
  {
  	uint_8* debugbuf;
  	debugbuf = &EMN_PHY_RxBuf.data[EMN_PHY_RxBuf.tail][0];
  	if (GetSubDateFlag == 0)
	{
		
		uint_8 i;
		uint_8* p;
		p = debugbuf;
		printf("\n");
		printf("************************************\n");
		printf("RECV\n");
		for (i=0; i<32; i++)
		{
			printf("%02X ", *p++);
			if (i % 16 == 0 && i !=0)
			{
				printf("\n");
			}
		}
		printf("************************************\n");
	}
  }
  #endif
}




/***********************************************************
* 
* Function Name    : 
* Returned Value    : 
* Comments          : 
*         1) when PHY layer received this message, it must be RX mode
*            NWK layer only send after receiving TX_cfm message,and MAC layer also
*           is reponsible for it.
*         2)parameters: which one location :numbers of PHY frame, transmitter address
*            note: (1)data from NWK.
                       (2)data from internal MAC,
*                     (3) router data from PHY layer
*
*************************************************************/

static void EMN_PHY_TxPacket(SYS_MESSAGE *msg)
{
 	UINT_8 i,repeat_time,frm_num;
 	UINT_8 *data_p, *repeat_p;
 	uchar   send_buffer[32];
 	uchar   recv_buffer[32];
 //	uchar* debugbuf;
 	uchar temp;
   frm_num = msg->data[1];
   temp = frm_num;
	i= msg->data[0]&0x0F;
 if(EMN_TXBUF_NWKTX==i )
 {
     data_p = &NWK_TxBuf[0][0];
	 
 }else if(EMN_TXBUF_MACCOM== i)
  	{
  	  data_p = MAC_ComBuf;
  	}else if(EMN_TXBUF_PHYRX== i)// router data
  	  {
         data_p = &(EMN_PHY_RxBuf.data[msg->data[0]>>4][0]);
  	  } else if(EMN_TXBUF_NWKCOM== i)
  	  	{
           data_p = NWK_ComBuf;
  	  	}else if(EMN_TXBUF_MACCOM2== i)
	     	{
	     	  data_p = MAC_ComBuf2;
	     	}else
 	  	    {  SYS_Error(0x0a);}
#if 0 
 if (GetSubDateFlag == 0)
	 {
		 
		 uint_8 i;
		 uint_8* p;
		 p = debugbuf;
		 printf("\n");
		 printf("************************************\n");
		 printf("SEND\n");
		 for (i=0; i<4*32; i++)
		 {
			 printf("%02X ", *p++);
			 if (i % 16 == 0 && i !=0)
			 {
				 printf("\n");
			 }
		 }
		 printf("************************************\n");
	 }
#endif
 //repeat handle
 if(*data_p & 0x04) 
 {
    repeat_time=2;
    repeat_p=data_p;
 }
   else 
     {
        repeat_time=0;
     }
 EMN_PHY_State = TX_MODE; 
// CLEAR_TRX_CE;  
 //_time_delay(1);	  	
 // resend handle 	  	
 // resend handle 

 
	send_buffer[0] = RF_WTA;
	send_buffer[1] = 0x00;
	send_buffer[2] = msg->data[3];
	send_buffer[3] = msg->data[2];
	send_buffer[4] = EMN_CONVERTERGY_NO;
	RFSpiReadWrite(spifd, send_buffer, recv_buffer, 5);
	fflush(spifd);
	
	do{

	 	   //DEBUG_DIS(printf("\nP:TX\n"));
	 	   
	 	   
 if(*data_p & 0x08) 
 {  
    MAC_ReSendBuf.which=msg->data[0];  
    MAC_ReSendBuf.frm_num=frm_num;
    MAC_ReSendBuf.net_addr=msg->data[2];
    MAC_ReSendBuf.subnet_addr=msg->data[3];
 }else
 	{
	  MAC_ReSendBuf.which= 0x00;
	  MAC_ReSendBuf.frm_num= 0x00;
 	}
 _time_delay(1);
//	Delay(SYS_TIMEOUT_PHY_TX_INTERVAL);

 send_buffer[0] = RF_WTP;
 RFSpiReadWrite(spifd, send_buffer, recv_buffer, 1);

//注意如果EMN_PHY_PAYLOAD_LEN>32需要分包发这里只考虑了小于等于32字节的情况

 RFSpiReadWrite(spifd, data_p, recv_buffer, EMN_PHY_PAYLOAD_LEN);
	fflush(spifd);	

	Delay(3);
// enter tansmiter mode and start to transmitter	

    SET_TXEN; 
	SET_TRX_CE;					// Enable  RF 
	_time_delay(1);
//	Delay(2);
	//Delay(10);
	_time_delay(1);
    while(!(GET_DR_PIN())){};	// waiter for data TX end

	CLEAR_TRX_CE;			    // disable  RF     
                                // chip will enter standby mode automatically  
 //waite peer to handle it 
   if(EMN_TXBUF_PHYRX== msg->data[0])// router data, it is circle buffer
   {
      if(data_p> (&(EMN_PHY_RxBuf.data[EMN_PHY_RXBUF_COUNT-1][EMN_PHY_PAYLOAD_LEN-1])))
	  {
	    data_p =&(EMN_PHY_RxBuf.data[0][0]);
      }	
   }
   if(repeat_time==0)
   {
     msg->data[1]--;
     if(*data_p & 0x04) 
     {
        repeat_time=2;
        repeat_p=data_p;
     }else 
       {
         repeat_time=0;
       }	
   }
   else
   {
   	 repeat_time--;
   	 data_p=repeat_p;
   }
  data_p=data_p+EMN_PHY_PAYLOAD_LEN;
 //  _time_delay(20);
 // Delay(SYS_TIMEOUT_PHY_TX_INTERVAL);//***** this is very important
 temp--;
 if (temp > 0)
 {
 	Delay(2*SYS_TIMEOUT_PHY_TX_INTERVAL);
 }
 else
 	{
 		Delay(SYS_TIMEOUT_PHY_TX_INTERVAL);
 	}
 }while(msg->data[1]>0);
//	Delay(SYS_TIMEOUT_PHY_TX_INTERVAL);//***** this is very important
 }

/**********************************************************
* 
* Function Name    : 
* Returned Value   : 
* Comments          :
*  
***********************************************************/
static 	UINT_8 PHY_PhyFrmNum;  // to adjust whether lost phy frame or not. 
static  UINT_8 PHY_MacFrmHd;
//static  UINT_8 PHY_MacData; 
#ifdef CON_TX_TEST
static  UINT_32  timeout_test;
#endif 


void EMN_PHY_TaskInit(void)
{   
  while((pDiAnDiDiTable->diancount[1]==0xff)&&(pDiAnDiDiTable->diancount[0]==0xff))
  	{
  	 _time_delay(200);
  	}
	//init timer for this task using
	EMN_PhyTimer.id = SYS_TIMER2;
	EMN_PhyTimer.fastfun = NULL;
	EMN_PhyTimer.taskID = EMN_PHY_TASK;
	EMN_PhyTimer.value =0;
	EMN_PhyTimer.msgID = 0;
	 //init buffer
	EMN_PHY_RxBuf.head = 0;
	EMN_PHY_RxBuf.tail = 0;
	EMN_PHY_RxBuf.count = 0;

	//init varible
	PHY_PhyFrmNum=0;
	RF_RxOverflow=0;
	EMN_PHY_InitNrf905();
    SET_RX_MODE();
    //rs485_init();
	RF_RxReady =0;
	RF_PhyGetDataEnd=1;
	// read the following parameters from storage
#if EMN_EMT_ONLY            	
 EMN_DevType = EMN_COORDINATOR;
#else
  EMN_DevType = EMN_DEVICE;
#endif 
if(EMN_DevType==EMN_COORDINATOR)
{
  EMN_DevAddr = 0x00;
  EMN_SubnetAddr = 0x00; 
  EMN_NetAddr = pBaseConfig->uid[3];	
#if (!DICOR_STATIC_REG_TAB)
  EMN_ChildNum=0;
#endif 
  EMN_NetDepth=0;
}else if( EMN_DevType==EMN_DEVICE)
	{
	  EMN_DevAddr = 0xff;
	  EMN_SubnetAddr = 0xff; 
	  EMN_NetAddr =0xff;	
	  EMN_NetDepth=0;
      EMN_ChildNum=0;
   }

#ifdef CON_TX_TEST	
 {
 	
    UINT_8 i;
	for(i =0;i<EMN_PHY_PAYLOAD_LEN;i++)
	{  
	  MAC_ComBuf[i]=i;	
	}  
	EMN_NetAddr =0xff;
	EMN_SubnetAddr =0xff;
	PHY_DataConTxTest();
	timeout_test=0;
 }
#endif 	

}
void EMN_PHY_Task(SYS_MESSAGE *msg)
{
  switch(msg->msg_id)
  {
     case EMN_PHY_TX_REQ: 
	 	  // DEBUG_DIS(printf("\nP:TX"));
           EMN_PHY_TxPacket(msg);		  
		   SET_RX_MODE();	 
		   EMN_SendMsg.dest_id = EMN_MAC_TASK;
		   EMN_SendMsg.msg_id = EMN_PHY_TX_CFM;
		   EMN_SendMsg.data[0]= msg->data[0];  // location
		   EMN_SendMsg.data[1]= msg->data[1];  //number
		   EMN_SendMsg.data[2]= EMN_OK;        // OK
		   SYS_SendMsg(&EMN_SendMsg);

           break;
	case EMN_PHY_RX_RES://for release phy layer's buffer
         if(msg->data[0]!=EMN_PHY_RxBuf.head)
         {
           SYS_Error(5);
         }
		 if(msg->data[1]>0) //release the number of the phy receive buffer
		 {
	   	   EMN_PHY_RxBuf.head += msg->data[1];
		   if(EMN_PHY_RxBuf.head >= EMN_PHY_RXBUF_COUNT)
		   {
		     EMN_PHY_RxBuf.head -= EMN_PHY_RXBUF_COUNT;
		   }
		   if(EMN_PHY_RxBuf.count<msg->data[1])
		   	{
		   	 SYS_Error(5);
		   	}else{
		       EMN_PHY_RxBuf.count -= msg->data[1];
		   	}
	   	  }else
	   	  	{
              SYS_Error(6);
	   	  	}
	   	  break;
#ifdef CON_TX_TEST			  
      case EMN_TIMEOUT :
	  	    timeout_test++;
			 if(msg->data[0] == SYS_TEST_CONTX)
			 {		
			   PHY_DataConTxTest();
			 }	
			 break;
#endif 			 
    case  EMN_RF_RX_READY:  //no parameter. this message send from OS
//          DEBUG_DIS(printf("\nP:R"));
		  EMN_PHY_RxPacket();
		  RF_PhyGetDataEnd=1;

	      PHY_MacFrmHd =EMN_PHY_RxBuf.data[EMN_PHY_RxBuf.tail][0];
	//  PHY_MacData=EMN_PHY_RxBuf.data[EMN_PHY_RxBuf.tail][1];
		
		  if(PHY_PhyFrmNum!=(PHY_MacFrmHd>>5))//discard this Mac frame
		  {
		    if(EMN_PHY_RxBuf.count>=PHY_PhyFrmNum)
		    {
		       EMN_PHY_RxBuf.count-=PHY_PhyFrmNum;
			   if(EMN_PHY_RxBuf.tail>=PHY_PhyFrmNum)
			   {
				 EMN_PHY_RxBuf.tail-= PHY_PhyFrmNum;			 
			   }else
				  {
					EMN_PHY_RxBuf.tail=(UINT_8)(EMN_PHY_RXBUF_COUNT-(PHY_PhyFrmNum-EMN_PHY_RxBuf.tail));
				  }
			   PHY_PhyFrmNum=0;
		    }
			break;
		  }
		  EMN_PHY_RxBuf.tail++;
		  EMN_PHY_RxBuf.count++;
		  if(EMN_PHY_RxBuf.tail== EMN_PHY_RXBUF_COUNT)
		  {
			EMN_PHY_RxBuf.tail =0;
		  }	
		  if(EMN_PHY_RxBuf.count>EMN_PHY_RXBUF_COUNT)//the buffer is full
		  {
		       SYS_Error(6);
		  }
          PHY_PhyFrmNum++; 
		  if(PHY_PhyFrmNum >8)
		  {
             SYS_Error(6);
		  }
		 //if((PHY_MacFrmHd&0x12)&&(PHY_MacData==NETWORK_DIS))//dis network
		 //{
         //   
		 //}
//#ifdef CON_RX_TEST		  
#if  0		  


		   if(PHY_PhyFrmNum ==4)
#else 
       	   if(PHY_MacFrmHd&0x10)//last         
#endif 		  
		  {
		    
			 EMN_SendMsg.dest_id = EMN_MAC_TASK;
			 EMN_SendMsg.msg_id = EMN_PHY_RX_IND;
#if 0		 //for router data 	 
			 if(EMN_PHY_RxBuf.tail>=PHY_PhyFrmNum)
			 {
			   EMN_SendMsg.data[0]=(UINT_8) (EMN_PHY_RxBuf.tail-PHY_PhyFrmNum);			   
			 }else
			    {
			      EMN_SendMsg.data[0]=(UINT_8)(EMN_PHY_RXBUF_COUNT-(PHY_PhyFrmNum-EMN_PHY_RxBuf.tail));
			 	}
#else
             EMN_SendMsg.data[0]=EMN_PHY_RxBuf.head;
#endif 
			 EMN_SendMsg.data[1]= PHY_PhyFrmNum;     //number
			 PHY_PhyFrmNum = 0;
			 SYS_SendMsg(&EMN_SendMsg);  //one logical frame
#ifdef CON_RX_TEST	

		   EMN_SendMsg.dest_id = EMN_PHY_TASK;
           EMN_SendMsg.msg_id = EMN_PHY_RX_RES;
           EMN_SendMsg.data[0]= EMN_SendMsg.data[0];  
           EMN_SendMsg.data[1]= EMN_SendMsg.data[1];  
           SYS_SendMsg(&EMN_SendMsg);
#endif 		   
		  }		  
		  break;
       default:      break;
	}

}



/***********************************************************
*
*************************************************************/

void RFSimpleTxPacket(uint_8* sendbuf,uint_8 sub, uint_8 netaddr)
{
 	UINT_8 *data_p;
 	uchar   send_buffer[32];
 	uchar   recv_buffer[32];


  
	data_p = sendbuf;
    EMN_PHY_State = TX_MODE; 

 	send_buffer[0] = RF_WTA;
	send_buffer[1] = 0x00;
	send_buffer[2] = sub;
	send_buffer[3] = netaddr;
	send_buffer[4] = EMN_CONVERTERGY_NO;
	RFSpiReadWrite(spifd, send_buffer, recv_buffer, 5);
	fflush(spifd);
	
	 	   
	 	   
 
 _time_delay(1);
//	Delay(SYS_TIMEOUT_PHY_TX_INTERVAL);

 send_buffer[0] = RF_WTP;
 RFSpiReadWrite(spifd, send_buffer, recv_buffer, 1);

//注意如果EMN_PHY_PAYLOAD_LEN>32需要分包发这里只考虑了小于等于32字节的情况

 RFSpiReadWrite(spifd, data_p, recv_buffer, EMN_PHY_PAYLOAD_LEN);
	fflush(spifd);	

	Delay(3);
// enter tansmiter mode and start to transmitter	

    SET_TXEN; 
	SET_TRX_CE;					// Enable  RF 
//	_time_delay(1);
//	Delay(2);
	Delay(10);
//	_time_delay(1);
    while(!(GET_DR_PIN())){};	// waiter for data TX end

	CLEAR_TRX_CE;			    // disable  RF     
                                // chip will enter standby mode automatically  



 		Delay(SYS_TIMEOUT_PHY_TX_INTERVAL);

SET_RX_MODE();	
	RF_PhyGetDataEnd=0;
	
	RF_PhySendEnd = 1;	//此时才可以接收
 }
 
 
 /************************************************************
*************************************************************/
void RFSimpleRxPacket(uint_8* recvbuf)
{
	UINT_8 i;
  	uchar send_buffer[32];
  	uchar recv_buffer[32];	
  	_time_delay(10);
  	if(GET_DR_PIN())       // data ready 
  	{
  		// enter standby mode for SPI operation
    	CLEAR_TRX_CE; 
   		// _time_delay(1);
    	Delay(5);
		send_buffer[0] = RF_RRP;
  		RFSpiReadWrite(spifd, send_buffer, recv_buffer, 1);

		//注意如果EMN_PHY_PAYLOAD_LEN>32需要分包收这里只考虑了小于等于32字节的情况
		RFSpiReadWrite(spifd, send_buffer, recv_buffer, EMN_PHY_PAYLOAD_LEN);
		fflush(spifd);
		for( i=0; i<EMN_PHY_PAYLOAD_LEN; i++)
		{
			recvbuf[i] = recv_buffer[i];
		}
	
/*	printf("\r\n******************\r\n");
	for( i=0; i<32; i++)
    {
	  printf("%02X ", recv_buffer[i]);
	}
    
	printf("\r\n******************\r\n");*/
  //while(GET_DR_PIN()){};  // waite for both  DR and AM changing from high to low 
//	SET_TRX_CE;
  	}
  	SET_RX_MODE();
  	RF_PhyGetDataEnd=0;
}



/* EOF */

