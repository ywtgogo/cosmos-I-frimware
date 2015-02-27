//* gateway 上传数据格式
//  Uin、Uout、Iout、Tem、P、State
//***************************************************************************/

#ifndef THIRDDEV_GATEWAY_H_
#define THIRDDEV_GATEWAY_H_

#define HTONS(n) (uint16_t)((((uint16_t) (n)) << 8) | (((uint16_t) (n)) >> 8))


typedef struct {
          uint16_t  id;	
          uint16_t Vin;
	      uint16_t Vout;
          uint16_t Iout;
	      uint16_t Temp;
	      uint16_t Power;
	      uint16_t Err;
          uint16_t Energy;
} GATEWAY_DATA;

//GATEWAY_DATA  GatewayData;



//***************************************************************************
//***************************GATEWAY相关的结构和宏定义***********

#define		GATEWAY_SEND_BYTE_ADDRESS			0
#define		GATEWAY_SEND_BYTE_FUNCTION			1

#define		GATEWAY_RECV_BYTE_ADDRESS			0
#define		GATEWAY_RECV_BYTE_FUNCTION			1
//#define		GATEWAY_RECV_BYTE_DATACOUNT			2

#define		GATEWAY_FUNCTION_READ_REGISTER		0x04
#define		GATEWAY_FUNCTION_WRITE_REGISTER		0x10
#define     ENTER_IAP_MODE_REGISTER             0x21
#define     ERASE_FLASH_REGISTER                0x22
#define     WRITE_FLASH_REGISTER                0x23
#define     END_IAP_REGISTER                    0x24
#define     ENTER_IAP_MODE_REQ_REGISTER         0x25
#define     ERASE_FLASH_REQ_REGISTER            0x26
#define     WRITE_FLASH_REQ_REGISTER            0x27
#define     END_IAP_REQ_REGISTER                0x28
#define     SET_FFD_TABLE_REGISTER              0x66
#define     MODIFY_FFD_NODE_NUM_REGISTER        0x06
#define     FFD_BIND_RFD_REGISTER               0x45
#define     FFD_START_COLLECT_REGISTER          0x42
#define     OPTIMUS_SYNC_REGISTER               0x43
#define     MAGNUS_SHUTDOWN_REGISTER            0x46
#define     LET_FFD_STOP_COLLECT_REGISTER       0x44
#define		GATEWAY_REGISTER_START_ADDR			0
#define		GATEWAY_REGISTER_READ_MAX_LENGHT    0x06

//***************************************************************************


extern void GATEWAY_ReadData(void *ptr);
int_8 register_device_Gateway(uint_16 addr1,uint_16 num_node, uint_32 period);

#endif
