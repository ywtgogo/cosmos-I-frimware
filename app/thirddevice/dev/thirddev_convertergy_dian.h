
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
* @brief Convertergy DiAn��������ݽṹ
*
***************************************************************************/

#ifndef _THIRDDEV_CONVERTERGY_DIAN_H_
#define _THIRDDEV_CONVERTERGY_DIAN_H_


typedef struct 
{
	int_16 uid1;			//Corvus UID1
	int_16 uid2;			//Corvus UID2
	int_16 faultmsg[5];		//ϵͳ������Ϣ
	int_16 current_RMS;		//I��Чֵ ����
	int_16 vbus;			//ĸ�ߵ�ѹ��Чֵ/�����ѹ
	int_16 gridfreq;		//����Ƶ��
	int_16 output_power;	//�������
	int_16 energy;			//���շ�����
	int_16 allenergy;		//�ܷ�����
	int_16 intput_power;	//���빦��
	int_16 grid_Rrms;		//������ѹ��Чֵ
	int_16 intput_currect;	//�������
	int_16 boosttemp;		//Boost1ɢ�����¶�
	int_16 ivntemp;			//Ivnɢ�����¶�
	int_16 temp;			//�����¶�
	int_16 energy1;			//��ʷ������
}CONVERTERGY_DIAN_DATA;

//˵����





//***************************************************************************


void Convertergy_Dian_ReadData(void *ptr);
int_8 register_device_Convertergy_Dian(uint_16 addr, uint_16 num_node,uint_32 period);

#endif
