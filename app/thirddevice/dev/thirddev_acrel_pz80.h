
/**********************************************************************/ /*!
*
* @file thirddevice_poll.h
*
* @author Younger
*
* @date 2012-04-06
*
* @version 0.0.0.0
*
* @brief ACREL PZ80系列电表的相关数据结构
* gateway 上传数据格式
  Uin、Uout、Iout、Tem、P、State
***************************************************************************/

#ifndef _THIRDDEV_ACREL_PZ80_H_
#define _THIRDDEV_ACREL_PZ80_H_


typedef struct 
{
	int_16 voltage_RMS;		//U有效值 电压（单位：V -9999~9999）
	int_16 voltage_index;	//U指数位
	int_16 current_RMS;		//I有效值 电流（单位：A -9999~9999）
	int_16 current_index;	//I指数位
	int_16 reserve1[4];		//保留
	int_16 power_RMS;		//P有效值 功率（单位：W -9999~9999）
	int_16 power_index;		//P指数位
	int_16 reserve2[2];		//保留
	uint_16 Ep_hight;		//正向电能高位 （单位：Wh 0~999999999）
	uint_16 Ep_low;			//正向电能低位
	uint_16 Eq_hight;		//反向电能高位 （单位：varh 0~999999999）
	uint_16 Eq_low;			//反向电能低位
	int_16 pt;				//电压变比
	int_16 ct;				//电流变比
	int_16 alarm_and_io;	//报警及I/O
	int_16 reserve3;		//保留
}ACREL_PZ80_DATA;

//说明：
//电压、电流、功率等数据数值计算方法：
//读数 = 有效值×10(指数位-3)   (注：10的几次方)
//报警及开关量输入/输出状态字:
//15      .... 11 10 9 8 7 6 ... 1 0
//15------保留
//14~12---保留
//11------电流高报警
//10------电流低报警
//9-------电压高报警
//8-------电压低报警
//7-------D11开关量输入
//6-------D12开关量输入
//5~2-----保留
//1-------D01开关量输出
//0-------D02开关量输出

//报警标志位：1为有报警 0为无报警


//***************************************************************************
//***************************Acrel PZ80系列电表相关的结构和宏定义***********

#define		ACREL_PZ80_SEND_BYTE_ADDRESS			0
#define		ACREL_PZ80_SEND_BYTE_FUNCTION			1

#define		ACREL_PZ80_RECV_BYTE_ADDRESS			0
#define		ACREL_PZ80_RECV_BYTE_FUNCTION			1
#define		ACREL_PZ80_RECV_BYTE_DATACOUNT			2

#define		ACREL_PZ80_FUNCTION_READ_REGISTER		0x04
#define		ACREL_PZ80_FUNCTION_WRITE_REGISTER		0x10

#define		ACREL_PZ80_REGISTER_START_ADDR			0
#define		ACREL_PZ80_REGISTER_READ_MAX_LENGHT		01

//***************************************************************************


void Acrel_PZ80_ReadData(void *ptr);
int_8 register_device_Acrel_PZ80(uint_16 addr, uint_16 num_node,uint_32 period);

#endif
