
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
* @brief ACREL PZ80ϵ�е���������ݽṹ
* gateway �ϴ����ݸ�ʽ
  Uin��Uout��Iout��Tem��P��State
***************************************************************************/

#ifndef _THIRDDEV_ACREL_PZ80_H_
#define _THIRDDEV_ACREL_PZ80_H_


typedef struct 
{
	int_16 voltage_RMS;		//U��Чֵ ��ѹ����λ��V -9999~9999��
	int_16 voltage_index;	//Uָ��λ
	int_16 current_RMS;		//I��Чֵ ��������λ��A -9999~9999��
	int_16 current_index;	//Iָ��λ
	int_16 reserve1[4];		//����
	int_16 power_RMS;		//P��Чֵ ���ʣ���λ��W -9999~9999��
	int_16 power_index;		//Pָ��λ
	int_16 reserve2[2];		//����
	uint_16 Ep_hight;		//������ܸ�λ ����λ��Wh 0~999999999��
	uint_16 Ep_low;			//������ܵ�λ
	uint_16 Eq_hight;		//������ܸ�λ ����λ��varh 0~999999999��
	uint_16 Eq_low;			//������ܵ�λ
	int_16 pt;				//��ѹ���
	int_16 ct;				//�������
	int_16 alarm_and_io;	//������I/O
	int_16 reserve3;		//����
}ACREL_PZ80_DATA;

//˵����
//��ѹ�����������ʵ�������ֵ���㷽����
//���� = ��Чֵ��10(ָ��λ-3)   (ע��10�ļ��η�)
//����������������/���״̬��:
//15      .... 11 10 9 8 7 6 ... 1 0
//15------����
//14~12---����
//11------�����߱���
//10------�����ͱ���
//9-------��ѹ�߱���
//8-------��ѹ�ͱ���
//7-------D11����������
//6-------D12����������
//5~2-----����
//1-------D01���������
//0-------D02���������

//������־λ��1Ϊ�б��� 0Ϊ�ޱ���


//***************************************************************************
//***************************Acrel PZ80ϵ�е����صĽṹ�ͺ궨��***********

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
