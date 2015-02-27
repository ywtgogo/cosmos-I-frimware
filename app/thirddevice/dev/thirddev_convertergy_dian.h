
/**********************************************************************/ /*!
*
* @file thirddevice_convertergy_dian.h
*
* @author Younger
*
* @date 2012-04-16
*
* @version 0.0.0.0
*
* @brief Convertergy DiAn的相关数据结构
*
***************************************************************************/

#ifndef _THIRDDEV_CONVERTERGY_DIAN_H_
#define _THIRDDEV_CONVERTERGY_DIAN_H_


typedef struct 
{
	int_16 uid1;			//Corvus UID1
	int_16 uid2;			//Corvus UID2
	int_16 faultmsg[5];		//系统错误信息
	int_16 current_RMS;		//I有效值 电流
	int_16 vbus;			//母线电压有效值/输入电压
	int_16 gridfreq;		//电网频率
	int_16 output_power;	//输出功率
	int_16 energy;			//当日发电量
	int_16 allenergy;		//总发电量
	int_16 intput_power;	//输入功率
	int_16 grid_Rrms;		//电网电压有效值
	int_16 intput_currect;	//输入电流
	int_16 boosttemp;		//Boost1散热器温度
	int_16 ivntemp;			//Ivn散热器温度
	int_16 temp;			//环境温度
	int_16 energy1;			//历史发电量
}CONVERTERGY_DIAN_DATA;

//说明：





//***************************************************************************


void Convertergy_Dian_ReadData(void *ptr);
int_8 register_device_Convertergy_Dian(uint_16 addr, uint_16 num_node,uint_32 period);

#endif
