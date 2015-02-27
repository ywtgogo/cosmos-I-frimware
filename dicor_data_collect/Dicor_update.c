/*************************************************************
* 
* Copyright (c) convertergy ;
* All Rights Reserved
* FileName: dicor_update.c
* Version : 
* Date    : 2011-11-18
* ower	  : alex
*
* Comments:* �Զ�����
*�޸ļ�¼ : 2012-4-9�޸��жϰ汾�ŵ��㷨���޸�bug,�����ֶ���������
*
***************************************************************/

#include <mqx.h>
#include <bsp.h>
#include <rtcs.h>
#include <string.h>
#include <lwtimer.h>
#include <shell.h>
#include <mfs.h>
#include "dicor_update.h"
#include "eeprom.h"
#include "utility.h"
#include "logging_public.h"
#include "tftp.h"
#include "rs485.h"

//ÿ�ռ��һ�η������Ƿ����°汾��������ʶ�����Ѿ�������
static uint_8 UpdateChkComplete = 0;
//extern uint_8 UpdateChkComplete = 0; //by younger
extern void dicor_get_logtime(DATE_STRUCT * date);
extern void upload_data_exit(void);
extern void rf_nwk_exit(void);
extern void EepromWriteUpdata(uint_8 *);
extern void EepromEras(_mem_size seek_location, unsigned char *secter_erase);
extern void dicor_close_socket(void);


void Dicor_Reboot(void)
{
	MCF5225_CCM_STRUCT_PTR ccr_reg_addr;
	
	dicor_close_socket();

	ccr_reg_addr =(MCF5225_CCM_STRUCT_PTR) (&((VMCF5225_STRUCT_PTR) _PSP_GET_IPSBAR())->CCM);
	ccr_reg_addr->RCR |= 0x80;
	
}


//��⵱ǰʱ���Ƿ��ڸ���ʱ���
static uint_8 IsUpdateTime(uint_16 hour)
{
	uint_16 endtime;	
	uint_16 h;
	uint_8 result;
	if (UPDATECHKENDTIME < UPDATECHKSTARTTIME)
	{
		endtime = UPDATECHKENDTIME + 24;
	}
	else
	{
		endtime = UPDATECHKENDTIME;
	}
	if (hour < UPDATECHKSTARTTIME)
	{
		h = hour + 24;
	}
	else
	{
		h = hour;
	}
	if (h < endtime)
	{
		result = 1;
	}
	else
	{
		result = 0;
	}
	
	return result;
}


//�ж��Ƿ���Ҫ���£���Ҫ����1������Ҫ����0
static uint_8 ChkUpdate(void)
{
	DATE_STRUCT date;
	uint_8 result;
	dicor_get_logtime(&date);

	if (IsUpdateTime(date.HOUR))
	{
		//printf("Update time\n");
		if (UpdateChkComplete)	//�����Ѿ����¹���
		{
			result = 0;
		}
		else
		{
			//���³ɹ��Ž�UpdateChkComplete��λ
			result = 1;
		}
	}
	else
	{
		UpdateChkComplete = 0;	//�ָ���Ϊ��һ�θ�����׼��
		result = 0;
	}
	
	return result;
}

        
         



//static
 int_8 AnalyseVersionFile(const char * filename)
{
	//����Щ�򵥵Ľ���
	PARAMS_TOBOOTLOADER_PTR pdata;
	unsigned char writebuf[100];
	char file_name[40];
	char s[100];
	char t[20];
	FILE_PTR fd_ptr;
	uint_32 ipaddr;
	uint_8 i;
	uint_8 sfver[3];
	int_8 result = 1;
	unsigned char sector = 32;
	fd_ptr = fopen(filename, "r");
	if (fd_ptr == NULL) 
	{
		printf("open file %s error!\n", filename);
		return -1;
	}

	while (strcmp(s, "[Version]") != 0) 
	{
		fgetline(fd_ptr, s, 64);	//[Version]
		printf("%s\n", s);
	}
	//�����汾����
	fgetline(fd_ptr, s, 64);	//[V2.1.1] 
	fgetline(fd_ptr, s, 64);
	printf("%s\n", s);
	if (cutstr(s, t, '='))
	{
		printf("�ı�������д��ʽ����\n");
		return -1;
	}
	for (i=18; i>0; i--)
	{
		t[i+1] = t[i];
	}
	t[0] = '1';
	t[1] = '.';
	get_dotted_address(t, &ipaddr);
	/*
	sfver[0] = (uint_8) (ipaddr >> 16);
	sfver[1] = (uint_8) (ipaddr >> 8);
	sfver[2] = (uint_8) (ipaddr);
	*/
	sfver[0] = s[9] - 0x30;
	sfver[1] = s[20] - 0x30;
	sfver[2] = s[31] - 0x30;
	printf("sfver[0]=%d,sfver[1]=%d,sfver[2]=%d\n",sfver[0],sfver[1],sfver[2]);
	printf("pBaseConfig->softversion[1]=%d,pBaseConfig->softversion[2]=%d,pBaseConfig->softversion[3]=%d\n",pBaseConfig->softversion[1],pBaseConfig->softversion[2],pBaseConfig->softversion[3]);
	
	EepromEras(32*512, &sector);
	/*
	if (sfver[0] > pBaseConfig->softversion[1])
	{
		result = 0;
	}
	else if (sfver[0] == pBaseConfig->softversion[1])
	{
		if(sfver[1] > pBaseConfig->softversion[2])
		{
			result = 0;
		}
		else if (sfver[1] == pBaseConfig->softversion[2])
		{
	 		if(sfver[2] > pBaseConfig->softversion[3])
			{
				result = 0;
			}
		}
	}
	if (result == 0)
	*/
	{
		printf("must update code\n");
		printf("save params in flash!\n");
		//�������������bootloader����ʹ��
		pdata = (PARAMS_TOBOOTLOADER_PTR)writebuf;
		strncpy(pdata->signature, "Cosmos", 12);//14
		pdata->tobootloader.server = IPADDR(pBaseConfig->datacenter[0],pBaseConfig->datacenter[1],pBaseConfig->datacenter[2],pBaseConfig->datacenter[3]);//TFTPHOSTIPADDR;
		pdata->tobootloader.b_update = 1;
		sprintf(file_name, "CosmosV%d.%d.%d.S19", sfver[0],sfver[1],sfver[2]);;
		strncpy(pdata->tobootloader.file_name, file_name, 40);
		//д��FLASH��
		_time_delay(800);
		EepromWrite((unsigned char *)pdata, 16*1024, sizeof(PARAMS_TOBOOTLOADER));
		//EepromWriteUpdata(&sfver[0]);
	}
	 	result = 0;	
	return result;
}

void Dicor_Update(void)
{  
	DATE_STRUCT date;
	//if (ChkUpdate())   by younger 2013.8.19
	{
		printf(" must download version file...\n");
		
	//	dicor_get_logtime(&date);
	//	sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t �ӷ��������ذ汾�����ļ�\r\n", date.HOUR,date.MINUTE,date.SECOND);
	//	PrintLog(pDiCorLog->logbuf);
	//	fwrite(pDiCorLog->logbuf, strlen(pDiCorLog->logbuf), 1, pDiCorLog->fd_log_ptr);
	//	_lwsem_post(&pDiCorLog->log_sem);
		
		//�������ļ���������������
		if (Dicor_TFTP_client(IPADDR(pBaseConfig->datacenter[0],pBaseConfig->datacenter[1],pBaseConfig->datacenter[2],pBaseConfig->datacenter[3]), "version.txt", "d:\\version.txt") == 0)
		{
		    
			UpdateChkComplete = 1;
			//�����ļ�
			if (AnalyseVersionFile(LOCALVERSIONFILENAME) == 0)     //2014.3.10
			{
				dicor_get_logtime(&date);
		
				sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t ���������°汾�������Ҫ����������������������������\r\n", date.HOUR,date.MINUTE,date.SECOND);
				PrintLog(pDiCorLog->logbuf);
	
				//�����ֳ�����������bootloader ����
				upload_data_exit();
				rf_nwk_exit();
				printf("program will reboot and run at bootloader!\n");
				_time_delay(800);
				Dicor_Reboot();
			}
		}
	}

}




/*FUNCTION*-------------------------------------------------------------
*
*  Function Name :  
*  Returned Value:  none
*  Comments  :  
*  Usage:  
*
*END*-----------------------------------------------------------------*/

int_32  Shell_UpgradeDiCor(int_32 argc, char_ptr argv[])
{
	boolean  print_usage, shorthelp = FALSE;
	int_32   return_code = SHELL_EXIT_SUCCESS;
	char filename[100];
	unsigned char writebuf[100];
	
	PARAMS_TOBOOTLOADER_PTR pdata;

	print_usage = Shell_check_help_request(argc, argv, &shorthelp );

	if (!print_usage)  
	{
		if (argc == 2)  
		{	
			
			strcpy(filename, "Cosmos");
			strcat(filename, argv[1]);
		   //	strcat(filename, "V8.0.0");
			strcat(filename, ".S19");
			if (argv[1][0] != 'V')
			{
				printf("����汾���Ƿ���ȷ\n");
				return return_code;
			}
			if (argv[1][1] > '9' || argv[1][1] < '0')
			{
				printf("����汾���Ƿ���ȷ\n");
				return return_code;
			}
			printf("save params in flash!\n");
			//�������������bootloader����ʹ��
			pdata = (PARAMS_TOBOOTLOADER_PTR)writebuf;
			strncpy(pdata->signature, "Cosmos", 12);
			pdata->tobootloader.server = TFTPHOSTIPADDR;
			pdata->tobootloader.b_update = 1;	//��Ҫ����
	
			strncpy(pdata->tobootloader.file_name, filename, 40);
			//д��FLASH��
			EepromWrite(writebuf, 16*1024, sizeof(PARAMS_TOBOOTLOADER));
			//�����ֳ�����������bootloader ����
			upload_data_exit();
			rf_nwk_exit();
			printf("program will reboot and run at bootloader!\n");
			Dicor_Reboot();
		
		} 
		else  
		{
			printf("Error, %s invoked with incorrect number of arguments\n", argv[0]);
			print_usage = TRUE;
		}
	}

	if (print_usage)  
	{
		if (shorthelp)  
		{
			printf("%s version number\n", argv[0]);
		} 
		else  
		{
			printf("Usage: %s VX.X.X\n", argv[0]);
		}
	}

	return return_code;
} 




/* EOF */
