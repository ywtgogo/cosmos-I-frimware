/*************************************************************
* 
* Copyright (c) convertergy ;
* All Rights Reserved
* FileName: upgradedidi.c
* Version : 
* Date    : 2011/12/21
* ower    : Younger
*
* Comments:*TFTP����DiDi����SD�����������ļ���SRAM���ٽ�SRAM�еĳ��򷢸�DiDi
*		
*
***************************************************************/


#include <string.h>
#include <mqx.h>
#include <bsp.h>
#include <shell.h>
#include <rtcs.h>
#include "sh_rtcs.h"
#include "modbus.h"
#include "utility.h"
#include <stdlib.h>
#include "rs485.h"
#include "network_nwk.h"
#include "dicor_upload.h"
#include "upgradedidi.h"
#include "tftp.h"
#include "network_rf_dri.h"
#include <Watchdog.h>
#include "dicor_update.h"
#define TFTPHOSTIPADDR2		IPADDR(115,29,192,154)

extern UINT_8 SERVER_MODE;
extern UINT_8 GetSubDateFlag;
extern LWSEM_STRUCT GetSubDateSem;
extern LWSEM_STRUCT NetworkRfSem;
INT_8 UpgradeState = 0;
extern LWSEM_STRUCT UpgradeStateSem;
UINT_8* RF_Data_From;
UINT_16 DiDiFlashStartAddr;
UINT_16 DiDiSendDataLen;
UINT_16 ProgramCrc;
//static DIMO_HT_DATA  Dimo_ht_data; 
//DIMO_RES_DATA Dimo_response_data;
//DIMO_HB_DATA  Dimo_hb_data;  
//DIMO_HT_DATA  Dimo_ht_data;

extern int_32 Shell_SetRFFreq(int_32 argc, char_ptr argv[]);


char DiDiFileName[100] = 
{
	'\0'
};

char MagnusFileName[100] =
{
	'\0'	
};

static int_8 AnalysisSRecord(char *filename, uint_8* rambuf);
static uint_8 SendStartCommand(uint_8 sub, uint_8 dev);
static uint_8 BlankCheck(uint_8* databuf, UINT_16 len);
static uint_8 SendDataToRf(uint_8 sub, uint_8 dev, uint_8* rambuf, uint_8 netaddr);
extern void EMN_APL_GetSampleData(void);
	
//�հ����ݰ����
static uint_8 BlankCheck(uint_8* databuf, UINT_16 len)
{
	uint_8 result = 1;
	UINT_16 i;
	
	for (i=0; i<len; i++)
	{
		if (*(databuf+i) != 0xFF)
		{
			result = 0;
			break;
		}
	}
	
	return result;
}

//��get����ֹͣ
void StopGetData(void)
{
	printf("stop get data\n");
	_lwsem_wait(&GetSubDateSem);
	GetSubDateFlag = 0; 
	_lwsem_post(&GetSubDateSem); 
}

//��get����ֹͣ
void StartGetData(void)
{
	printf("start get data\n");
	_lwsem_wait(&GetSubDateSem);
	GetSubDateFlag = 1; 
	_lwsem_post(&GetSubDateSem); 
}



//���Ϳ�ʼ��������
static uint_8 SendStartCommand(uint_8 sub, uint_8 dev)
{
	uint_8 result = 0;
	uint_32 time = 0;
	printf("wait networkrfsem\n");
	_lwsem_wait(&NetworkRfSem);
	EMN_SendMsg.dest_id = EMN_APL_TASK;
	EMN_SendMsg.msg_id = EMN_APL_START_UPGRADE_CMD;
	EMN_SendMsg.data[0] = sub;
	EMN_SendMsg.data[1] = dev;
	EMN_SendMsg.data[2] = EMN_UPGRADE_CMD;
	EMN_SendMsg.data[3] = EMN_UPGRADE_NOROUTER_CMD;
	DEBUG_DIS(printf("EMN_NWK_START_UPGRADE\n"));		 
	SYS_SendMsg(&EMN_SendMsg);
	_lwsem_post(&NetworkRfSem); 
	
	//�ȴ��ɹ����߳�ʱ��־
	while (1)
	{
		_lwsem_wait(&UpgradeStateSem);
		if (UpgradeState != 0)
		{
			//��ʱ����
			if (UpgradeState == -1)
			{
				result = 1;
				printf("timeout SendStartCommand\n");
			}
			UpgradeState = 0;
			break;
		}		
		_lwsem_post(&UpgradeStateSem); 
		_time_delay(1);
		time++;
	}
	_lwsem_post(&UpgradeStateSem);
	printf("send start upgrade command take time [%dms]\n", time);
	return result;
}


static void Setdidipar(uint_8 bak_sub, uint_8 bak_dev, uint_8 sub, uint_8 dev)
{
	uint_16 i;
	
	SetParameterData.brd=0x03;
	SetParameterData.index=8;
 	SetParameterData.len=5;
 	SetParameterData.dev=bak_dev;
 	
 	SetParameterData.data[0]= 0x01;
 	SetParameterData.data[1]= 0x01;
 	SetParameterData.data[2]= 0x54;
 	SetParameterData.data[3]= sub;
 	SetParameterData.data[4]= dev;
 	SetParameterData.flag = 0;
 	
 	EMN_APL_SetParameter(bak_sub, bak_dev, &SetParameterData);
	i = 0;
	while (i < 400)
	{
		_time_delay(10);
		if (SetParameterData.flag)
	{
		break;
	}
		i++;
	}
	if(i != 400)
	{
	  	printf("i = %d\n",i*10);
	}
	else
	{
	  	printf("timeout set parameter\n");
	}

		 
}


int_32  Shell_Downloaddidiprogram(int_32 argc, char_ptr argv[])
{
	boolean  print_usage, shorthelp = FALSE;
	int_32   return_code = SHELL_EXIT_SUCCESS;
	char filename[100]; 
//	unsigned char *didiflash;
	

	filename[99] = '\0';
	argv[1]="V1.0.2_45.S";

	print_usage = Shell_check_help_request(argc, argv, &shorthelp );

	if (!print_usage)  
	{
		if (argc == 2)  
		{	
			strcpy(filename, "d:\\didiprogram\\");
			strcat(filename, argv[1]);
			if (Dicor_TFTP_client(TFTPHOSTIPADDR2, argv[1], filename) == 0)
			//if (Dicor_TFTP_client(IPADDR(192,168,8,98), argv[1], filename) == 0)
			{
				printf("downloade file succeed\n");
				strcpy(DiDiFileName, filename);
			}
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
			printf("%s <0xaddr_f> <0xaddr_r>\n", argv[0]);
		} 
		else  
		{
			printf("Usage: %s <0xaddr_f> <0xaddr_r>\n", argv[0]);
		}
	}

	return return_code;
} 


int_32  Shell_Getdidiprogramfilename(int_32 argc, char_ptr argv[])
{
	boolean  print_usage, shorthelp = FALSE;
	int_32   return_code = SHELL_EXIT_SUCCESS;
//	unsigned char *didiflash;
	

	

	print_usage = Shell_check_help_request(argc, argv, &shorthelp );

	if (!print_usage)  
	{
		if (argc == 1)  
		{	
			printf("didi program filename id [%s]\n", DiDiFileName);
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
			printf("%s <0xsubnet> <0xdevaddr>\n", argv[0]);
		} 
		else  
		{
			printf("Usage: %s <0xsubnet> <0xdevaddr>\n", argv[0]);
		}
	}

	return return_code;
} 

int_32  Shell_Setdidiprogramfilename(int_32 argc, char_ptr argv[])
{
	boolean  print_usage, shorthelp = FALSE;
	int_32   return_code = SHELL_EXIT_SUCCESS;
	char filename[100]; 
//	unsigned char *didiflash;
	

	filename[99] = '\0';
	
   argv[1]=	"V1.0.2_45.elf.S";
   
	print_usage = Shell_check_help_request(argc, argv, &shorthelp );

	if (!print_usage)  
	{
		if (argc == 2)  
		{	
			strcpy(filename, "d:\\didiprogram\\");
			strcat(filename, argv[1]);
			strcpy(DiDiFileName, filename);
			printf("set didi program filename [%s]\n", DiDiFileName);
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
			printf("%s <0xsubnet> <0xdevaddr>\n", argv[0]);
		} 
		else  
		{
			printf("Usage: %s <0xsubnet> <0xdevaddr>\n", argv[0]);
		}
	}

	return return_code;
} 





//////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
//��Bootloader��ʽ����DiDi

//extern void RFSimpleTxPacket(uint_8* sendbuf,uint_8 sub, uint_8 netaddr);
//extern void RFSimpleRxPacket(uint_8* recvbuf);

Package PackageSendbuf;
Package2 PackageSendbuf2;
Package1 PackageRecvbuf;
extern LWEVENT_STRUCT RF_NET_event;
static void print_rfrecv_log(uint_8* buf);

static void print_rfrecv_log(uint_8* buf)
{
	#if 0
	uint_8* p;
	uint_8 j;
	p = buf;
	printf("\n");
	printf("************************************\n");
	printf("RECV\n");
	for (j=0; j<32; j++)
	{
		if (j % 16 == 0 && j !=0)
		{
			printf("\n");
		}
		printf("%02X ", *p++);
	}
	printf("\n************************************\n");
	#endif
}


//��HEX��ʽ�ļ��������������浽SRAM��
 int_32 Boot_AnalysisHEX(char *filename, uint_8* rambuf,uint_8* data_buffer)
{
	FILE_PTR fd_ptr;
	char* linebuf;
	char* p;
	char hex[3];
	uint_8 len;
	uint_16 addr;
	uint_8 addrl;
	uint_8 addrh;
	uint_8 type;
	uint_16 i,k;
	uint_16 offset;
	uint_16 crc; 
	uint_32 error;
    uint_16 version;
	
	uint_16 f;
	int_8 result = 1;
    uint_8  j;
    uint_16 addr_f, addr_r;
    
       addr_f = (uint_16)data_buffer[2];
       addr_f = addr_f << 8;
       addr_f = addr_f & 0xFF00 ;
       addr_f|= data_buffer[3];
       addr_r = (uint_16)data_buffer[4];
       addr_r = addr_r << 8;
       addr_r = addr_r & 0xFF00 ;
       addr_r|= data_buffer[5];
        version = (uint_16)data_buffer[6];
       version = version << 8;
       version = version & 0xFF00 ;
       version|= data_buffer[7];
    filename ="d:\\didiprogram\\magnus.hex";
	hex[2] = '\0';
	fd_ptr = fopen("d:\\didiprogram\\magnus.hex", "r");
	if (fd_ptr == NULL) 
	{
		printf("open file %s error!\n", filename);
		return -1;
	}
	//ģ��magnus����FLASH
	memset(rambuf, 0xFF, BOOT_DIDIPROGRAMABLESIZE_MAGNUS);
	
	
	linebuf = (char *) _mem_alloc_zero_from(_user_pool_id, 1024);
    if (linebuf == NULL)
    {
    	printf("error when mem alloc line buf\r\n");	
    	return -1;
    }
	memset(linebuf, 0, 1024);
//	FFD_BIND_RFD_Command(addr_f,data_buffer);
	while (!feof(fd_ptr))
	{
		fgetline(fd_ptr, linebuf, 1024);
		p = linebuf;

	   	if (*p++ == ':')
			//��ʼ������
		{
			
			hex[0] = *p++;
			hex[1] = *p++;
			len = ahextoi(hex);

	    	if (len ==0x10)
	    	{
				hex[0] = *p++;
				hex[1] = *p++;
				addrh = ahextoi(hex);
				hex[0] = *p++;
				hex[1] = *p++;
				addrl = ahextoi(hex);
				addr = addrh;
				addr <<= 8;
				addr += addrl;
				if (addr >= BOOT_CC2530_END)
				{
					continue;
				}
				offset = (addr - BOOT_CC2530_BEGIN)*2;
				hex[0] = *p++;
				hex[1] = *p++;
				type = ahextoi(hex);
				for (i=0; i<16; i++)
				{
					hex[0] = *p++;
					hex[1] = *p++;
					*(rambuf+offset) = ahextoi(hex);
					offset++;
				}			
	    	}		 		 
		}	
	} 

	if (Modbus_ENTER_IAP_MODE_Command(addr_f,addr_r,data_buffer) == -1)
	{
		goto END;
	}SEND_INFO_DEL(data_buffer[3]|data_buffer[2]<<8, 0x31, 0x00);


	if (MAGNUS_LOAD_PREAMBLE_Command(addr_f,addr_r,data_buffer,version) == -1)
	{
		goto END;
	}SEND_INFO_DEL(data_buffer[3]|data_buffer[2]<<8, 0x35, 0x00);
	
	
	if (Modbus_ERASE_FLASH_Command(addr_f,addr_r,data_buffer) == -1)
	{
		goto END;
		//printf("\n____32 error____\n");
		//return -1;	
	}SEND_INFO_DEL(data_buffer[3]|data_buffer[2]<<8, 0x32, 0x00);
   
    for (k=0;k<((offset/160)+1);k++)
    {
    	f=160*k;
   		memset(PackageSendbuf2.data, 0, 80);
   	
		for(j=0;j<16;j++)
		{			 
			PackageSendbuf2.data[j]=rambuf[f+j];			
		}		    
	    for(j=0;j<16;j++)
		{		
			PackageSendbuf2.data[16+j]=rambuf[32+f+j];
		}
		for(j=0;j<16;j++)
		{
			PackageSendbuf2.data[32+j]=rambuf[64+f+j];
		}
		for(j=0;j<16;j++)
		{
			PackageSendbuf2.data[48+j]=rambuf[96+f+j];
		}
		for(j=0;j<16;j++)
		{
			PackageSendbuf2.data[64+j]=rambuf[128+f+j];			
		}		      
		PackageSendbuf2.cos_addr = 0x8001;
        PackageSendbuf2.addr_f = addr_f;
	    PackageSendbuf2.function= 0x33;
		PackageSendbuf2.addr_r = addr_r;
		PackageSendbuf2.len=0x50;
		PackageSendbuf2.start_addr=k * 0x50; 

		if (Modbus_WRITE_FLASH_Command(addr_f,addr_r,(uint_8*)&PackageSendbuf2,90)==-1)
		{
			printf("\n____33 break for(offset=%d)____\n", k);
			goto END; 
		} 	 	 
    }SEND_INFO_DEL(data_buffer[3]|data_buffer[2]<<8, 0x33, 0x00);
    
  	crc = Modbus_calcrc16(rambuf, k * 0x50);	 
	if (END_IAP_Command(addr_f, addr_r, crc, data_buffer) == -1)
	{
		goto END;
	}SEND_INFO_DEL(data_buffer[3]|data_buffer[2]<<8, 0x34, 0x00);
	_time_delay(20000);

END:
	_mem_free(linebuf);
	_mem_free(rambuf);
	fclose(fd_ptr);
	_time_delay(20000);
	//Dicor_Reboot();        
    return 1;
}



//��S-Rcord��ʽ�ļ��������������浽SRAM��
 int_32 Boot_AnalysisSRecord_8245(char *filename, uint_8* rambuf,uint_8* data_buffer)
{
    FILE_PTR fd_ptr;
	char* linebuf;
	char* p;
	char hex[3];
	uint_8 len;
	uint_16 addr;
	uint_8 addrl;
	uint_8 addrh;
	uint_8 type;
	uint_16 i,k;
	uint_16 offset;
	uint_16 crc; 
	uint_32 error;
    uint_16 version;
	
    //uint_8  offset;
    //uint_8 data_buffer[7];
	uint_16 f;
	int_8 result = 1;
    uint_8  j;
    uint_16 addr_f, addr_r;
    
       addr_f = (uint_16)data_buffer[2];
       addr_f = addr_f << 8;
       addr_f = addr_f & 0xFF00 ;
       addr_f|= data_buffer[3];
       addr_r = (uint_16)data_buffer[4];
       addr_r = addr_r << 8;
       addr_r = addr_r & 0xFF00 ;
       addr_r|= data_buffer[5];
        version = (uint_16)data_buffer[6];
       version = version << 8;
       version = version & 0xFF00 ;
       version|= data_buffer[7];
    filename ="d:\\didiprogram\\8245V1.0.S";
	hex[2] = '\0';
	fd_ptr = fopen("d:\\didiprogram\\8245V1.0.S", "r");
	
		   		_time_delay(200);

	 OPT_Modbus_ENTER_IAP_MODE_Command(addr_f,addr_r,data_buffer);
	 _time_delay(500);
	 
	  
    //MAGNUS_LOAD_PREAMBLE_Command(addr_f,addr_r,data_buffer,version);
	 //_time_delay(800);
	 
	 OPT_Modbus_ERASE_FLASH_Command(addr_f,addr_r,data_buffer);
     //printf("32\n");
     _time_delay(2000);
	if (fd_ptr == NULL) 
	{
		printf("open file %s error!\n", filename);
		return -1;
	}
	//ģ��DiDi����FLASH
	memset(rambuf, 0xFF, BOOT_DIDIPROGRAMABLESIZE_FOR8245);
	
	linebuf = (char *) _mem_alloc_zero_from(_user_pool_id, 1024);
    if (linebuf == NULL)
    {
    	printf("error when mem alloc line buf\r\n");	
    	return -1;
    }
	memset(linebuf, 0, 1024);
	
	while (!feof(fd_ptr))
	{
		fgetline(fd_ptr, linebuf, 1024);
		p = linebuf;
		if (*p++ != 'S')
		{
			continue;
		}
		if (*p == '7')
		{
			//S-Rcord��ʽ�ļ�����
			printf("Analysis S-Record end\n");
			result = 0;
			break;
		}
		else if (*p++ == '3')
		{
			//��ʼ������
			hex[0] = *p++;
			hex[1] = *p++;
			len = ahextoi(hex);
		//	printf("len=%d\n", len);
			
			//����ַ
			p++;
			p++;
			p++;
			p++;
			hex[0] = *p++;
			hex[1] = *p++;
			addrh = ahextoi(hex);
			hex[0] = *p++;
			hex[1] = *p++;
			addrl = ahextoi(hex);
			addr = addrh;
			addr <<= 8;
			addr += addrl;
			if (addr >= BOOT_PROGRAMENABEEND)
			{
				continue;
			}
		//	printf("addr=%04X\n", addr);
			offset = (addr - BOOT_PROGRAMENABEBEGIN_FOR8245)*2;
			f= offset;
			len = len - 4;
		//	printf("len=%d\n", len);
			*(rambuf+offset)=len+4;
		//	printf("%02X\n", *(rambuf+offset));
			*(rambuf+offset+1)=0x00;
		//	printf("%02X\n", *(rambuf+offset+1));
			*(rambuf+offset+2)=0x00;
		//	printf("%02X\n", *(rambuf+offset+2));
			*(rambuf+offset+3)=addrh;
		//	printf("%02X\n", *(rambuf+offset+3));
			*(rambuf+offset+4)=addrl;
		//	printf("%02X\n", *(rambuf+offset+4));
		//	*(rambuf+offset+5)=len+5;
			for (i=0; i<len; i++)
			{ 
			   
				hex[0] = *p++;
				hex[1] = *p++;
			
			//	printf("%02X\n", *(rambuf+offset));
				*(rambuf+offset+5) = ahextoi(hex);
		//		printf("%02X\n", *(rambuf+offset+5));
				offset++;
			}
			
		 
		 // 	pPackage = (Package *)malloc(len+12);
		  	
			memset(PackageSendbuf.data, 0, len+5);
		        
		//PackageSendbuf2.cos_addr = 0x8001;
        PackageSendbuf.addr_f = addr_f;
	    PackageSendbuf.function= 0x23;
		PackageSendbuf.addr_r = addr_r;
	//	memcpy((uint_8 *)(&PackageSendbuf.databuffer),(uint_8 *)&rambuf,len+5);
	    // PackageSendbuf.databuffer=rambuf;
			for(j=0;j<len+5;j++)
			{
			 
		
	        	PackageSendbuf.data[j]=rambuf[f+j];
			 	
			}
		       
		      _time_delay(50);
		OPT_Modbus_WRITE_FLASH_Command(addr_f,addr_r,(uint_8*) &PackageSendbuf,len+12);  
		 		    _time_delay(400);
		       
		}
	}
	OPT_END_IAP_Command(addr_f, addr_r, crc,data_buffer);
	Dimo_ht_data.Dicor_UID[0]= Dimo_response_data.Dicor_UID[0];
	Dimo_ht_data.Dicor_UID[1]= Dimo_response_data.Dicor_UID[1];
	Dimo_ht_data.Dicor_UID[2]= Dimo_response_data.Dicor_UID[2];
	Dimo_ht_data.Dicor_UID[3]= Dimo_response_data.Dicor_UID[3];
	Dimo_ht_data.pack_obj=Dimo_response_data.pack_obj;
	Dimo_ht_data.pack_type=Dimo_response_data.pack_type;
	Dimo_ht_data.state=0x00;
	if (SERVER_MODE == 2)	
		error = send(upload_buffer.sock_mux,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]), sizeof(DIMO_HT_DATA), 0);
	error = send(upload_buffer.sock,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]), sizeof(DIMO_HT_DATA), 0);
	_mem_free(rambuf);
	_time_delay(2000);
	if (SERVER_MODE == 2)
		error = send(upload_buffer.sock_mux,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]), sizeof(DIMO_HT_DATA), 0); 
	error = send(upload_buffer.sock,(char_ptr)(&Dimo_ht_data.Dicor_UID[0]), sizeof(DIMO_HT_DATA), 0); 
	Dicor_Reboot();        
	_mem_free(linebuf);
	fclose(fd_ptr);
	return result;
}



extern void SYS_Init(void);
extern MQX_FILE_PTR spifd;
extern void EMN_PHY_InitNrf905(void);





//static uint_8 SendDataToRf_8245(uint_8* rambuf, uint_16 addr_f,uint_16 addr_r)
static uint_8 SendDataToRf_8245(uint_16 addr_f, uint_8* rambuf, uint_16 addr_r)
{
	uint_16 i,j;
	uint_8 state = 0;
//	TIME_STRUCT time1, time2;
//	uint_32 dt;
//	uint_32 minute, second, mil;
	uint_32 time = 0;
	UINT_8* sendbuf;
	uint_16 addr;
//	uint_16 eraseaddr;
//	uint_16 erasesize;
	uint_8* p;
	uint_16 size;
	uint_8 trytimes = 0;
	_mqx_uint result;
//	_time_get(&time1);
	uint_8 dev;
	
//	dicor_waite_rf(RF_GET_DATA_END,100); 
//	dicor_waite_rf(RF_GET_DATA_END,100); 
//	dicor_waite_rf(RF_GET_DATA_END,100); 
	//	printf("%02X\n", *rambuf);
	////��ʼ����

//	for (trytimes=0; trytimes<5; trytimes++)
for (trytimes=0; trytimes<1; trytimes++)
	{
		if (trytimes != 0)
		{
			printf("retry! trytimes=%d\n", trytimes);
		}
	
		//�ȷ��Ϳ�ʼ����
	//	memset(PackageSendbuf.data, 0, VALIDDATALEN);
	//	memset(&PackageRecvbuf, 0, VALIDDATALEN);
//		PackageSendbuf.addr_f = 0x0101;
//		PackageSendbuf.funcode = 0x23;
//		PackageSendbuf.addr_r = 0x0202;
		//PackageSendbuf.data[0] = EMN_SubnetAddr;
		
//		sendbuf = (uint_8*) &PackageSendbuf;
		printf("���Ϳ�ʼ����\n");
	//	RFSimpleTxPacket(sendbuf,sub, netaddr);
	//	Modbus_WRITE_FLASH_Command(addr_f,addr_r,sendbuf,60);
		printf("�ȴ���Ӧ...\n");
	//	result = _lwevent_wait_ticks(&RF_NET_event, RF_GET_DATA_END, TRUE, 1000);
	//	_lwevent_clear(&RF_NET_event, RF_GET_DATA_END);

		/*
		if (result == LWEVENT_WAIT_TIMEOUT)
		{
			printf("��ʱ��û���յ���Ӧ��\n");
			if (trytimes < 4)
			{
				//����
				printf("timeout! sleep 1s\n");
				_time_delay(1000);
				continue;
			}
		
			else
			{
				printf("timeout! stop send!\n");
				printf("��ʱ��û���յ���Ӧ������ʧ��\n");
				state = 1;
				goto END;
			}
		
		}
		
			*/
	//	RFSimpleRxPacket((uint_8*) &PackageRecvbuf);
	    /* 
		p = (uint_8*) &PackageRecvbuf;
		print_rfrecv_log(p);
		if (PackageRecvbuf.head1 != PACKAGEHEAD1)
		{
			printf("û�ҵ����ݰ�ͷ���������ݣ�����\n");
			if (trytimes < 4)
			{
				//����
				printf("sleep 1s\n");
				_time_delay(1000);
				continue;
			}
		
			else
			{
				printf("stop send!\n");
				printf("����ʧ��\n");
				state = 1;
				goto END;
			}
			
		}
		if (PackageRecvbuf.head2 != PACKAGEHEAD2)
		{
		//	printf("��ʼ����!�������%d\n", PackageRecvbuf.head2);
			if (trytimes < 4)
			{
				//����
				printf("sleep 1s\n");
				_time_delay(1000);
				continue;
			}
			/*
			else
			{
				printf("stop send!\n");
				printf("����ʧ��\n");
				state = 1;
				goto END;
			}
			*/
			/*
		}
		else
		{
			printf("��ʼ�ɹ���\n");
			printf("DiDi Bootloader�汾�ţ�%s\n", PackageRecvbuf.data);
		}
		if (result == MQX_OK)
		{
			break;
		}
		
		*/
	}
	////��ʼ�������
	//////////////////////////////////////////////////////////////////////////
	
	////������ʼ
	///////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////
//	for (trytimes=0; trytimes<5; trytimes++)
   for (trytimes=0; trytimes<1; trytimes++)
	{
		if (trytimes != 0)
		{
			printf("retry! trytimes=%d\n", trytimes);
		}	
		//�ȷ��Ͳ�������
	//	memset(PackageSendbuf.data, 0, VALIDDATALEN);
	//	memset(&PackageRecvbuf, 0, 30);
//		PackageSendbuf.addr_f = addr_f;
//		PackageSendbuf.funcode = 0x23;
//		PackageSendbuf.addr_r = addr_r;
//		PackageSendbuf.start_addr=0;
	    //PackageSendbuf.len=0x3d;
	//	PackageSendbuf.addr=0;
//		eraseaddr = FLASH_LO_FOR8245;
//		erasesize = FLASH_LAST_PAGE - FLASH_LO_FOR8245;
//		PackageSendbuf.data[0] = (uint_8)(eraseaddr >> 8);
//		PackageSendbuf.data[1] = (uint_8)eraseaddr;
//		PackageSendbuf.data[2] = (uint_8)(erasesize >> 8);
//		PackageSendbuf.data[3] = (uint_8)erasesize;
//		sendbuf = (uint_8*) &PackageSendbuf;
		printf("���Ͳ���FLASH����\n");
	//	RFSimpleTxPacket(sendbuf,sub, netaddr);
	//	Modbus_WRITE_FLASH_Command(addr_f,addr_r,sendbuf,60);
		printf("�ȴ���Ӧ...\n");
	//	result = _lwevent_wait_ticks(&RF_NET_event, RF_GET_DATA_END, TRUE, 1000);
	//	_lwevent_clear(&RF_NET_event, RF_GET_DATA_END);

/*
		if (result == LWEVENT_WAIT_TIMEOUT)
		{
			printf("��ʱ��û���յ���Ӧ��\n");
			if (trytimes < 4)
			{
				//����
				printf("timeout! sleep 1s\n");
				_time_delay(1000);
				continue;
			}
			else
			{
				printf("timeout! stop send!\n");
				printf("��ʱ��û���յ���Ӧ������ʧ��\n");
				state = 1;
				goto END;
			}
		}
*/		

  /*
		RFSimpleRxPacket((uint_8*) &PackageRecvbuf);
		p = (uint_8*) &PackageRecvbuf;
		print_rfrecv_log(p);
		if (PackageRecvbuf.head1 != PACKAGEHEAD1)
		{
			printf("û�ҵ����ݰ�ͷ���������ݣ�����\n");
			if (trytimes < 4)
			{
				//����
				printf("sleep 1s\n");
				_time_delay(1000);
				continue;
			}
			
			else
			{
				printf("stop send!\n");
				printf("����ʧ��\n");
				state = 1;
				goto END;
			}
			
		}
		if (PackageRecvbuf.head2 != PACKAGEHEAD2)
		{
			printf("��������!�������%d\n", PackageRecvbuf.head2);
			if (trytimes < 4)
			{
				//����
				printf("sleep 1s\n");
				_time_delay(1000);
				continue;
			}
			
			else
			{
				printf("stop send!\n");
				printf("����ʧ��\n");
				state = 1;
				goto END;
			}
			
		}
		else
		{
			printf("����FLASH�ɹ���\n");
		}
		if (result == MQX_OK)
		{
			break;
		}
	*/	
		
	}
	////��������
	//////////////////////////////////////////////////////////////////////////
	
	
	////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////
    	printf("��ʼдFLASH�����Ե�...\n");
	    sendbuf = rambuf;
	//	PackageSendbuf.addr_f = 0x0101;
	//	PackageSendbuf.funcode = 0x23;
	//	PackageSendbuf.addr_r = 0x0202;
	//	PackageSendbuf.start_addr=0x0000;
	
//	printf("BOOT_DIDIPROGRAMABLESIZE_FOR8245=%d BOOT_DIDIPROGRAMABLESIZE/VALIDDATALEN=%d\n",BOOT_DIDIPROGRAMABLESIZE_FOR8245,BOOT_DIDIPROGRAMABLESIZE/VALIDDATALEN);
	
	for (i=0; i<(BOOT_DIDIPROGRAMABLESIZE_FOR8245/VALIDDATALEN); i++)
	{
	    printf("%2d\n",i);
		addr = BOOT_PROGRAMENABEBEGIN_FOR8245 + i*VALIDDATALEN/2;
		
		
		if (i != BOOT_DIDIPROGRAMABLESIZE_FOR8245/VALIDDATALEN)
		{
			size = VALIDDATALEN;
		}
		else
		{
			if (BOOT_DIDIPROGRAMABLESIZE_FOR8245 % VALIDDATALEN != 0)
			{
				size = BOOT_DIDIPROGRAMABLESIZE_FOR8245 % VALIDDATALEN;
			}
			else
			{
				break;
			}
		}
		
		//�հװ�
		if (BlankCheck(sendbuf, size))
		{
			;
		//	printf("blank package, [%d] bytes skip ID[%03d/%d]\n", size, i, BOOT_DIDIPROGRAMABLESIZE/VALIDDATALEN);
		}
		else
		{
			for (trytimes=0; trytimes<1; trytimes++)
			{
					
				//printf("send program data\n");
				//_time_get(&time1);
				if (trytimes != 0)
				{
					printf("retry! trytimes=%d\n", trytimes);
				}
				
			//	memset(PackageSendbuf.data, 0, VALIDDATALEN);
			//	memset(&PackageRecvbuf, 0, 30);
				
		//		PackageSendbuf.head1 = dev;
		//		PackageSendbuf.head2 = PACKAGEHEAD2;
		//		PackageSendbuf.addr = addr;
			//	for (j=0; j<VALIDDATALEN; j++)
			//	{
			//		PackageSendbuf.data[j] = sendbuf[j];
			//	}

				//printf("����дFLASH����\n");
			//	RFSimpleTxPacket((uint_8*) &PackageSendbuf,sub, netaddr);
		//		Modbus_WRITE_FLASH_Command(addr_f,addr_r,(uint_8*) &PackageSendbuf,66);
				//printf("�ȴ���Ӧ...\n");
			//	dicor_waite_rf(RF_GET_DATA_END,0);    
			//	result = _lwevent_wait_ticks(&RF_NET_event, RF_GET_DATA_END, TRUE, 1000);
				_lwevent_clear(&RF_NET_event, RF_GET_DATA_END);
				
				//�����ʱ
				
				/*
				
				if (result == LWEVENT_WAIT_TIMEOUT)
				{ 
					if (trytimes < 4)
					{
						//����
						printf("timeout! sleep 1s\n");
						_time_delay(1000);
						continue;
					}
					
					else
					{
						printf("timeout! stop send!\n");
						state = 1;
						goto END;
					}
					
				}

						
				RFSimpleRxPacket((uint_8*) &PackageRecvbuf);
				
				p = (uint_8*) &PackageRecvbuf;
				print_rfrecv_log(p);
				
				if (PackageRecvbuf.head1 != PACKAGEHEAD1)
				{
					printf("û�ҵ����ݰ�ͷ���������ݣ�����\n");
					if (trytimes < 4)
					{
						//����
						printf("sleep 1s\n");
						_time_delay(1000);
						continue;
					}
					else
					{
						printf("���Դ������ֹ࣬ͣ����\n");
						state = 1;
						goto END;
					}
				}
				if (PackageRecvbuf.head2 != PACKAGEHEAD2)
				{
					printf("��̳���!�������%d\n", PackageRecvbuf.head2);
				
					if (trytimes < 4)
					{
						//����
						printf("sleep 1s\n");
						_time_delay(1000);
						continue;
					}
					else
					{
						printf("���Դ������ֹ࣬ͣ����\n");
						state = 1;
						goto END;
					}
				}
				else
				
				*/
			//	{
				//	for (j=0; j<size; j++)
				//	{
				//		if (PackageRecvbuf.data[j] != PackageSendbuf.data[j])
				//		{
			//				break;
			//			}
			//		}
			//		if (j == size)
			//		{
		//				;
		//			//	printf("дFLASH�ɹ���\n");
		//			}
					/*
					else
					{
						printf("дFLASHʧ�ܣ�\n");
						if (trytimes < 4)
						{
							//����
							printf("sleep 1s\n");
							_time_delay(1000);
							continue;
						}
						else
						{
							printf("���Դ������ֹ࣬ͣ����\n");
							state = 1;
							goto END;
						}
					}
					*/
			//	}
				
			//	_time_get(&time2);
			//	dt = (time2.SECONDS*1000+time2.MILLISECONDS-(time1.SECONDS*1000+time1.MILLISECONDS));
			//	printf("send [%d] bytes program data take time [%4dms] ID[%03d/%d]\n", size, dt, i, BOOT_DIDIPROGRAMABLESIZE/VALIDDATALEN);
				_time_delay(1);
				//��������
				if (result == MQX_OK)
				{
					break;
				}
			}
		}
		sendbuf += size;
		printf("����һ�Σ�\n");
	
	}
	printf("��DiDi Flash ���ȫ���ɹ���\n");
	//////////////////////////////////////////////////////
	//////////////////////////////////////////////////////
	
	
	
	////////////////////////////////////////////////////////////////////
	//��д�������֪ͨDiDi�޸ı�־λ
//	for (trytimes=0; trytimes<5; trytimes++)
//	for (trytimes=0; trytimes<2; trytimes++)	
//	{	
	//	if (trytimes != 0)
	//	{
	//		printf("retry! trytimes=%d\n", trytimes);
	//	}
	//	memset(PackageSendbuf.data, 0, VALIDDATALEN);
	//	memset(&PackageRecvbuf, 0, 32);
	//	PackageSendbuf.addr_r = addr_r;
	//	PackageSendbuf.head2 = PACKAGEHEAD2;
//		PackageSendbuf.addr = 1;
		
	//	sendbuf = (uint_8*) &PackageSendbuf;
//		printf("���ͽ����������\n");
	//	RFSimpleTxPacket(sendbuf,sub, netaddr);
	//	Modbus_WRITE_FLASH_Command(addr_f,addr_r,sendbuf,60);	
//		printf("�ȴ���Ӧ...\n");
	//	result = _lwevent_wait_ticks(&RF_NET_event, RF_GET_DATA_END, TRUE, 1000);
	//	_lwevent_clear(&RF_NET_event, RF_GET_DATA_END);
		
		
		/*	
		if (result == LWEVENT_WAIT_TIMEOUT)
		{ 
			printf("��ʱ��û���յ���Ӧ������ʧ��\n");
			if (trytimes < 4)
			{
				//����
				printf("timeout! sleep 1s\n");
				_time_delay(1000);
				continue;
			}
			else
			{
				printf("timeout! stop send!\n");
				state = 1;
				goto END;
			}	
		}   
				
		RFSimpleRxPacket((uint_8*) &PackageRecvbuf);
		p = (uint_8*) &PackageRecvbuf;
		print_rfrecv_log(p);
		if (PackageRecvbuf.head1 != PACKAGEHEAD1)
		{
			printf("û�ҵ����ݰ�ͷ���������ݣ�����\n");
			if (trytimes < 4)
			{
				//����
				printf("sleep 1s\n");
				_time_delay(1000);
				continue;
			}
			else
			{
				printf("stop send!\n");
				state = 1;
				goto END;
			}	
		}
		if (PackageRecvbuf.head2 != PACKAGEHEAD2)
		{
			printf("������ɳ���!�������%d\n", PackageRecvbuf.head2);
			if (trytimes < 4)
			{
				//����
				printf("sleep 1s\n");
				_time_delay(1000);
				continue;
			}
			else
			{
				printf("stop send!\n");
				state = 1;
				goto END;
			}	
		}
		else
		{
			printf("�ɹ���ɣ�\n");
		}
		if (result == MQX_OK)
		{
			break;
		}
		*/
//	}


//END:
	/*
	RF_PhyGetDataEnd=1;
	StartGetData();
	_lwsem_wait(&NetworkRfSem);
	fclose(spifd);
	SYS_Init();
//	EMN_PHY_InitNrf905();
    SET_RX_MODE();
	RF_RxReady=0;
	RF_PhyGetDataEnd=1;
	EMN_APL_GetSampleData();
	_lwsem_post(&NetworkRfSem);
	*/
	return state;
}

	
int_32  Shell_StopGetData(int_32 argc, char_ptr argv[])
{
	boolean  print_usage, shorthelp = FALSE;
	int_32   return_code = SHELL_EXIT_SUCCESS;
//	uint_8	sub, dev;
	char hex[3]; 
//	unsigned char *didiflash;

	hex[2] = '\0';

	print_usage = Shell_check_help_request(argc, argv, &shorthelp );

	if (!print_usage)  
	{
		if (argc == 1)  
		{	
			StopGetData();
	
			_lwsem_wait(&UpgradeStateSem);
			UpgradeState = 0;
			_lwsem_post(&UpgradeStateSem);
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
			printf("%s <0xsubnet> <0xdevaddr>\n", argv[0]);
		} 
		else  
		{
			printf("Usage: %s <0xsubnet> <0xdevaddr>\n", argv[0]);
		}
	}

	return return_code;
} 



//Bootloader ģʽCosmos��optimus������
static uint_8 Boot_UpgradeSubDiDi(uint_8 sub, uint_8 dev, uint_8* rambuf, uint_8 netaddr)
{
	uint_8 state = 0;
	TIME_STRUCT time1, time2;
	uint_32 dt;
	uint_32 minute, second, mil;
	uint_32 time = 0;
    uint_16 addr_f,addr_r;

	_time_get(&time1);
	
	
#if 0
	state = SendStartCommand(sub, dev);
	if (state != 0)
	{
		printf("error send start command!\n");
	/*
		StartGetData();
		_lwsem_wait(&NetworkRfSem);
		EMN_APL_GetSampleData();
		_lwsem_post(&NetworkRfSem);
		return state;*/
	}
#endif
	_time_delay(1000);
	_lwsem_wait(&UpgradeStateSem);
	UpgradeState = 0;
	_lwsem_post(&UpgradeStateSem);
	state = SendStartCommand(sub, dev);
	if(state != 0)
	{  		
	   printf("protocol stack send start command no response!\n");
	}
	StopGetData();	
//	state = SendDataToRf(sub, dev,  rambuf,netaddr);
	_time_get(&time2);
	dt = (time2.SECONDS*1000+time2.MILLISECONDS)-(time1.SECONDS*1000+time1.MILLISECONDS);
	minute = dt / 60000;
	mil = dt % 1000;
	second = (dt-minute*60000) / 1000;
	printf("total time [%dms]=[%dm %ds %dms]\n", dt, minute, second, mil);
	StartGetData();
	_lwsem_wait(&NetworkRfSem);
	EMN_APL_GetSampleData();
	_lwsem_post(&NetworkRfSem);
	return state;
}

//Bootloader ģʽDiCor��DiDi������
 uint_8 Boot_UpgradeSubDiDi_8245(uint_16 addr_f, uint_8* rambuf, uint_16 addr_r)
{
	uint_8 state = 0;
	TIME_STRUCT time1, time2;
	uint_32 dt;
	uint_32 minute, second, mil;
	uint_32 time = 0;
  //  uint_16 addr_f,addr_r;

	_time_get(&time1);
	
	addr_f=ADDR_F+0x8000;
	addr_r=ADDR_R;
//#if 0
////	state = SendStartCommand(sub, dev);
//	if (state != 0)
//	{
//		printf("error send start command!\n");
	/*
		StartGetData();
		_lwsem_wait(&NetworkRfSem);
		EMN_APL_GetSampleData();
		_lwsem_post(&NetworkRfSem);
		return state;*/
//	}
//#endif

   //	printf("%02X\n", *rambuf);
	_time_delay(1000);
	_lwsem_wait(&UpgradeStateSem);
	UpgradeState = 0;
	_lwsem_post(&UpgradeStateSem);
//	state = SendStartCommand(sub, dev);
	if(state != 0)
	{  		
	   printf("protocol stack send start command no response!\n");
	}
	StopGetData();	
	state = SendDataToRf_8245(addr_f, rambuf, addr_r);
	_time_get(&time2);
	dt = (time2.SECONDS*1000+time2.MILLISECONDS)-(time1.SECONDS*1000+time1.MILLISECONDS);
	minute = dt / 60000;
	mil = dt % 1000;
	second = (dt-minute*60000) / 1000;
	printf("total time [%dms]=[%dm %ds %dms]\n", dt, minute, second, mil);
	StartGetData();
	_lwsem_wait(&NetworkRfSem);
	EMN_APL_GetSampleData();
	_lwsem_post(&NetworkRfSem);
	return state;
}


#define MAXUPGRADETIMEOUT	5*60*1000
//DiDi��DiDi����������
static uint_8 Boot_UpgradeRouteDiDi(uint_8 sub, uint_8 dev)
{
	uint_8 state = 0;
	TIME_STRUCT time1;
	TIME_STRUCT time2;
	int_8 result = 0;
	uint_32 dt;
	uint_32 minute, second, mil;
	_time_get(&time1);
	
	StopGetData();
	_lwsem_wait(&UpgradeStateSem);
	UpgradeState = 0;	//��ȷ1����������0�ȴ�
	_lwsem_post(&UpgradeStateSem);


	printf("wait networkrfsem\n");
	_lwsem_wait(&NetworkRfSem);
	EMN_SendMsg.dest_id = EMN_APL_TASK;
	EMN_SendMsg.msg_id = EMN_APL_START_UPGRADE_CMD;
	EMN_SendMsg.data[0] = sub;
	EMN_SendMsg.data[1] = dev;
	EMN_SendMsg.data[2] = EMN_UPGRADE_CMD;
	EMN_SendMsg.data[3] = EMN_UPGRADE_ROUTER_CMD;
	DEBUG_DIS(printf("EMN_NWK_START_UPGRADE\n"));		 
	SYS_SendMsg(&EMN_SendMsg);
	_lwsem_post(&NetworkRfSem); 
	StopGetData();	
	//�ȴ��ɹ����߳�ʱ��־
	printf("DiDi���������������1���ӣ����Ե�...\n");
	while (1)
	{
		_lwsem_wait(&UpgradeStateSem);
		if (UpgradeState != 0)
		{
			result = UpgradeState;
			if (result == 1)
			{
				state = 0;
			}
			else
			{
				state = 1;
				printf("ERROR CODE %d\n", result);
			}
			
			_lwsem_post(&UpgradeStateSem);
			
			break;
		}		
		_lwsem_post(&UpgradeStateSem); 
		_time_get(&time2);
		dt = (time2.SECONDS*1000+time2.MILLISECONDS)-(time1.SECONDS*1000+time1.MILLISECONDS);
		if (dt > MAXUPGRADETIMEOUT)
		{
			printf("timeout upgrade didi!\n");
			state = 1;
			break;
		}
		
		_time_delay(1);
	}
	
	_time_get(&time2);
	dt = (time2.SECONDS*1000+time2.MILLISECONDS)-(time1.SECONDS*1000+time1.MILLISECONDS);
	minute = dt / 60000;
	mil = dt % 1000;
	second = (dt-minute*60000) / 1000;
	printf("total time [%dms]=[%dm %ds %dms]\n", dt, minute, second, mil);

	
	return state;
}
	
	
/*FUNCTION*-------------------------------------------------------------
*
*  Function Name :  
*  Returned Value:  none
*  Comments  :  
*  Usage:  
*
*END*-----------------------------------------------------------------*/

int_32  Shell_UpgradeDiDi(int_32 argc, char_ptr argv[])
{
	boolean  print_usage, shorthelp = FALSE;
	int_32   return_code = SHELL_EXIT_SUCCESS;
	uint_8	sub, dev;
	uint_8  i;
	uint_16 addr_f,addr_r;
	uint_8 netaddr;
	char hex[3]; 
	unsigned char *didiflash;
	TIME_STRUCT time1, time2;
	uint_32 dt;
	uint_32 minute, second, mil;
    EEPROMDATA_PTR p;
	p = (EEPROMDATA_PTR) base_addr;

	hex[2] = '\0';

	print_usage = Shell_check_help_request(argc, argv, &shorthelp );

	if (!print_usage)  
	{
		if (argc == 3)  
		{	
			netaddr = pBaseConfig->net_addr;
			
			//������ַ��
			hex[0] = argv[1][2];
			hex[1] = argv[1][3];
			strupr(hex);
			sub = ahextoi(hex);

			//�豸��ַ
			hex[0] = argv[2][2];
			hex[1] = argv[2][3];
			strupr(hex);
			dev = ahextoi(hex);
			printf("DIDI subaddr=%02X devaddr=%02X", sub, dev);
			//sub = 0x02;
			//dev = 0x00;
			
			StopGetData();
	
			_lwsem_wait(&UpgradeStateSem);
			UpgradeState = 0;
			_lwsem_post(&UpgradeStateSem);
			
		//	strcpy(DiDiFileName, "d:\\didiprogram\\TEST.S");
		    if((sub !=0)&&(dev == 0))
			{
			  for(i = 0;i < p->RouterTable[2];i ++)
			 {
			   if(sub == EMN_DownTab[i].addr)
			   {
				  break;
			   }
			  }			  
			}
			if((sub != 0)&&(dev != 0))
				{
				 i = p->RouterTable[2];
				}
		    if((i != p->RouterTable[2])||sub == 0)
			//DiCor�µ�����
			{
			   //send begin command
			  	//���������ⲿ�ڴ�
					didiflash = (unsigned char *) _mem_alloc_zero_from(_user_pool_id, BOOT_DIDIPROGRAMABLESIZE);
					if (didiflash == NULL)
					{
						printf("error when mem alloc didiflash buf\r\n");	
						return -1;
					}
					//����TFTP���س���SD����
					
					printf("���ڴ�SD��������DiDi�����ļ����ڴ沢���������Ե�...\n");
				//	if (!Boot_AnalysisSRecord(DiDiFileName, didiflash))
					{
						//
						printf("����DiDi�����ļ�[%s]���ڴ���ɣ��������ɹ���\n", DiDiFileName);
						_time_get(&time1);
		
						if (Boot_UpgradeSubDiDi(sub, dev, didiflash, netaddr))
						{
							printf("Failed in Upgrade DiDi 0x%02X 0x%02X\n",sub, dev);
							printf("����DiDi 0x%02X 0x%02X ʧ�ܣ������ԣ�\n", sub, dev);
						}
						else
						{
							printf("Succeed Upgrade DiDi 0x%02X 0x%02X\n", sub, dev);
							printf("����DiDi 0x%02X 0x%02X �ɹ���\n", sub, dev);
						}
						_time_get(&time2);
						dt = (time2.SECONDS*1000+time2.MILLISECONDS)-(time1.SECONDS*1000+time1.MILLISECONDS);
						minute = dt / 60000;
						mil = dt % 1000;
						second = (dt-minute*60000) / 1000;
						printf("total time [%dms]=[%dm %ds %dms]\n", dt, minute, second, mil);
					}
				//	else
					{
						printf("error when analysis didi program file\n");
						printf("��������DiDi�����ļ���������ls,dir������������ļ�[%s]�Ƿ���ڻ��ʽ�Ƿ���ȷ��\n", DiDiFileName);
					}
					_mem_free(didiflash);
			/*	else
				{
				    printf("error send start command!\n");
					StartGetData();
					_lwsem_wait(&NetworkRfSem);
					EMN_APL_GetSampleData();
					_lwsem_post(&NetworkRfSem);
					return FALSE;
				}*/
				
			}

			else
			{
				if (Boot_UpgradeRouteDiDi(sub, dev))
				{
					printf("Failed in Upgrade DiDi 0x%02X 0x%02X\n",sub, dev);
					printf("����DiDi 0x%02X 0x%02X ʧ�ܣ������ԣ�\n", sub, dev);
				}
				else
				{
					printf("Succeed Upgrade DiDi 0x%02X 0x%02X\n", sub, dev);
					printf("����DiDi 0x%02X 0x%02X �ɹ���\n", sub, dev);
				}
				
				StartGetData();
				_lwsem_wait(&NetworkRfSem);
				EMN_APL_GetSampleData();
				_lwsem_post(&NetworkRfSem);
			}
			
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
			printf("%s <0xsubnet> <0xdevaddr>\n", argv[0]);
		} 
		else  
		{
			printf("Usage: %s <0xsubnet> <0xdevaddr>\n", argv[0]);
		}
	}

	return return_code;
} 



/*FUNCTION*-------------------------------------------------------------
*
*  Function Name :  
*  Returned Value:  none
*  Comments  :  
*  Usage:  
*
*END*-----------------------------------------------------------------*/

int_32  Shell_UpgradeDiDi8245(int_32 argc, char_ptr argv[],uint_8* data_buffer)
{
	boolean  print_usage, shorthelp = FALSE;
	int_32   return_code = SHELL_EXIT_SUCCESS;
//	uint_8	sub, dev;
	uint_8  i;
	uint_8  len;
//	uint_8 netaddr;
	uint_16 addr_f,addr_r;
  //  uint_16 node_id;
	char hex[3]; 
	unsigned char *didiflash;
	TIME_STRUCT time1, time2;
	uint_32 dt;
	uint_32 minute, second, mil;
    EEPROMDATA_PTR p;
	p = (EEPROMDATA_PTR) base_addr;

	hex[2] = '\0';

	print_usage = Shell_check_help_request(argc, argv, &shorthelp );

	if (!print_usage)  
	{
		if (argc == 3)  
		{	
		//	netaddr = pBaseConfig->net_addr;
			//������ַ
			hex[0] = argv[1][2];
			hex[1] = argv[1][3];
			strupr(hex);
			addr_f = ahextoi(hex);

			//�豸��ַ
			hex[0] = argv[2][2];
			hex[1] = argv[2][3];
			strupr(hex);
			addr_r = ahextoi(hex);
			printf("optimus addr_f=%04X addr_r=%04X", addr_f, addr_r);
	//	printf("Magnus addr=%04X",node_id)
			//sub = 0x02;
			//dev = 0x00;
			
			StopGetData();
	
			_lwsem_wait(&UpgradeStateSem);
			UpgradeState = 0;
			_lwsem_post(&UpgradeStateSem);
			
		//	strcpy(DiDiFileName, "d:\\didiprogram\\TEST.S");
		 //   if((sub !=0)&&(dev == 0))
		//	{
		//	  for(i = 0;i < p->RouterTable[2];i ++)
		//	 {
		//	   if(sub == EMN_DownTab[i].addr)
		//	   {
		//		  break;
		//	   }
	//		  }			  
	//		}
		//	if((sub != 0)&&(dev != 0))
		//		{
		//		 i = p->RouterTable[2];
		//		}
	//	    if((i != p->RouterTable[2])||sub == 0)
			//DiCor�µ�����
			{
			   //send begin command
			  	//���������ⲿ�ڴ�
					didiflash = (unsigned char *) _mem_alloc_zero_from(_user_pool_id, BOOT_DIDIPROGRAMABLESIZE_FOR8245);
					if (didiflash == NULL)
					{
						printf("error when mem alloc didiflash buf\r\n");	
						return -1;
					}
					//����TFTP���س���SD����
					
					printf("���ڴ�SD��������DiDi�����ļ����ڴ沢���������Ե�...\n");
					if (!Boot_AnalysisSRecord_8245(DiDiFileName, didiflash, data_buffer))
					{
						//
						printf("����DiDi�����ļ�[%s]���ڴ���ɣ��������ɹ���\n", DiDiFileName);
						_time_get(&time1);
		
						if (Boot_UpgradeSubDiDi_8245(addr_f, didiflash, addr_r))
						{
							printf("Failed in Upgrade DiDi 0x%04X 0x%04X\n",addr_f, addr_r);
							printf("����DiDi 0x%04X 0x%04X ʧ�ܣ������ԣ�\n", addr_f, addr_r);
						}
						else
						{
							printf("Succeed Upgrade DiDi 0x%04X 0x%04X\n", addr_f, addr_r);
							printf("����DiDi 0x%04X 0x%04X �ɹ���\n", addr_f, addr_r);
						}
						_time_get(&time2);
						dt = (time2.SECONDS*1000+time2.MILLISECONDS)-(time1.SECONDS*1000+time1.MILLISECONDS);
						minute = dt / 60000;
						mil = dt % 1000;
						second = (dt-minute*60000) / 1000;
						printf("total time [%dms]=[%dm %ds %dms]\n", dt, minute, second, mil);
					}
					else
					{
						printf("error when analysis didi program file\n");
						printf("��������DiDi�����ļ���������ls,dir������������ļ�[%s]�Ƿ���ڻ��ʽ�Ƿ���ȷ��\n", DiDiFileName);
					}
					_mem_free(didiflash);
			/*	else
				{
				    printf("error send start command!\n");
					StartGetData();
					_lwsem_wait(&NetworkRfSem);
					EMN_APL_GetSampleData();
					_lwsem_post(&NetworkRfSem);
					return FALSE;
				}*/
				
			}

		//	else
			{
			//	if (Boot_UpgradeRouteDiDi(sub, dev))
			//	{
			//		printf("Failed in Upgrade DiDi 0x%02X 0x%02X\n",sub, dev);
			//		printf("����DiDi 0x%02X 0x%02X ʧ�ܣ������ԣ�\n", sub, dev);
			//	}
			//	else
				{
					printf("Succeed Upgrade DiDi 0x%04X 0x%04X\n", addr_f, addr_r);
					printf("����DiDi 0x%04X 0x%04X �ɹ���\n", addr_f, addr_r);
				}
				
				StartGetData();
				_lwsem_wait(&NetworkRfSem);
				EMN_APL_GetSampleData();
				_lwsem_post(&NetworkRfSem);
			}
			
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
			printf("%s <0xaddr_f> <0xaddr_r>\n", argv[0]);
		} 
		else  
		{
			printf("Usage: %s <0xaddr_f> <0xaddr_r>\n", argv[0]);
		}
	}

	return return_code;
} 


//̽���豸
extern void RFWorkReserve(uint_32 f);
/*FUNCTION*-------------------------------------------------------------
*
*  Function Name :  
*  Returned Value:  none
*  Comments  :  
*  Usage:  
*
*END*-----------------------------------------------------------------*/

int_32  Shell_DetectDevice(int_32 argc, char_ptr argv[])
{
	boolean  print_usage, shorthelp = FALSE;
	int_32   return_code = SHELL_EXIT_SUCCESS;
	uint_32 retrytimes;
	uint_8  uid[4];
	uint_8  i;
	UINT_8* sendbuf;
	char hex[3]; 
	uint_8 state = 0;
	_mqx_uint result;
	uint_8* p;


	hex[2] = '\0';

	print_usage = Shell_check_help_request(argc, argv, &shorthelp );

	if (!print_usage)  
	{
		if (argc == 3 || argc == 2)  
		{	
		
			StopGetData();
	
			_lwsem_wait(&UpgradeStateSem);
			UpgradeState = 0;
			_lwsem_post(&UpgradeStateSem);
			
			for (i=0; i<4; i++)
			{
				hex[0] = argv[1][i*2];
				hex[1] = argv[1][i*2+1];
				strupr(hex);
				uid[i] = ahextoi(hex);
			}
			
			
			if (argc == 2)
			{
				retrytimes = 1;
			}
			else
			{
				retrytimes = atoi(argv[2]);
			}
			printf("��̽����=%d\n", retrytimes);
			
			//���������ַΪ0xFE,Ƶ��Ϊ434.6M
			RFWorkReserve(434600);
			
			dicor_waite_rf(RF_GET_DATA_END,100); 
			dicor_waite_rf(RF_GET_DATA_END,100); 
			dicor_waite_rf(RF_GET_DATA_END,100); 
				
			for (i=0; i<retrytimes; i++)
			{
				if (i != 0)
				{
					printf("retry! retrytimes=%d\n", i+1);
				}
			
				//�ȷ���̽������
			//	memset(PackageSendbuf.data, 0, VALIDDATALEN);
			//	memset(&PackageRecvbuf, 0, 32);
			//	PackageSendbuf.head1 = 0xFE;
			//	PackageSendbuf.head2 = PACKAGEHEAD2;
			//	PackageSendbuf.addr = 3;
			//	PackageSendbuf.data[0] = uid[0];
			//	PackageSendbuf.data[1] = uid[1];
			//	PackageSendbuf.data[2] = uid[2];
			//	PackageSendbuf.data[3] = uid[3];
				sendbuf = (uint_8*) &PackageSendbuf;
				printf("����̽���豸����\n");
				RFSimpleTxPacket(sendbuf,0x00, 0xFE);
				printf("�ȴ���Ӧ...\n");
				result = _lwevent_wait_ticks(&RF_NET_event, RF_GET_DATA_END, TRUE, 1000);
				_lwevent_clear(&RF_NET_event, RF_GET_DATA_END);

				if (result == LWEVENT_WAIT_TIMEOUT)
				{
					printf("��ʱ��û���յ���Ӧ��\n");
					if (i < retrytimes-1)
					{
						//����
						printf("timeout!\n");
					//	_time_delay(500);
						continue;
					}
					else
					{
						printf("timeout! stop send!\n");
						printf("��ʱ��û���յ���Ӧ��û��̽�⵽�豸\n");
						state = 1;
						goto DetectDeviceEND;
					}
				}
						
				RFSimpleRxPacket((uint_8*) &PackageRecvbuf);
				p = (uint_8*) &PackageRecvbuf;
				print_rfrecv_log(p);
			//	if (PackageRecvbuf.head1 != PACKAGEHEAD1)
				{
					printf("û�ҵ����ݰ�ͷ���������ݣ�����\n");
					if (i < retrytimes-1)
					{
						//����
						printf("sleep 1s\n");
						_time_delay(1000);
						continue;
					}
					else
					{
						printf("stop send!\n");
						printf("̽��ʧ��\n");
						state = 1;
						goto DetectDeviceEND;
					}
				}
				/*
				if (PackageRecvbuf.head2 != PACKAGEHEAD2)
				{
					printf("̽�����!�������%d\n", PackageRecvbuf.head2);
					if (i < retrytimes - 1)
					{
						//����
						printf("sleep 1s\n");
						_time_delay(1000);
						continue;
					}
					else
					{
						printf("stop send!\n");
						printf("̽��ʧ��\n");
						state = 1;
						goto DetectDeviceEND;
					}
				}
				else
				*/
				{
					printf("̽��ɹ���\n");
					printf("��Ϣ���£�\n");
					printf("================================================\n");
					printf("[���е�ַ] = 0x%02X\n", PackageRecvbuf.data[0]);
					if (PackageRecvbuf.data[1])
					{
						printf("[�豸����] = ���豸\n");
					}
					else
					{
						printf("[�豸����] = ��ͷ�豸\n");
					}
					printf("[�����ַ] = 0x%02X\n", PackageRecvbuf.data[2]);
					printf("[������ַ] = 0x%02X\n", PackageRecvbuf.data[3]);
					printf("[�豸��ַ] = 0x%02X\n", PackageRecvbuf.data[4]);
					printf("[���豸��] = %d\n", PackageRecvbuf.data[5]);
					printf("[UID] = %02X-%02X-%02X-%02X\n", PackageRecvbuf.data[7], PackageRecvbuf.data[8], PackageRecvbuf.data[9], PackageRecvbuf.data[10]);
					printf("[������־λ] = %d\n", PackageRecvbuf.data[11]);
					printf("================================================\n");
				}
				if (result == MQX_OK)
				{
					break;
				}
			}

			
			
			
			
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
			printf("%s <0xsubnet> <0xdevaddr>\n", argv[0]);
		} 
		else  
		{
			printf("Usage: %s <0xsubnet> <0xdevaddr>\n", argv[0]);
		}
	}

DetectDeviceEND:

	return return_code;
}



//̽���豸

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name :  
*  Returned Value:  none
*  Comments  :  
*  Usage:  
*
*END*-----------------------------------------------------------------*/
/*
int_32  Shell_DetectDevice430(int_32 argc, char_ptr argv[])
{
	boolean  print_usage, shorthelp = FALSE;
	int_32   return_code = SHELL_EXIT_SUCCESS;
	uint_32 retrytimes;
	uint_8  uid[4];
	uint_8  i;
	UINT_8* sendbuf;
	char hex[3]; 
	uint_8 state = 0;
	_mqx_uint result;
	uint_8* p;


	hex[2] = '\0';

	print_usage = Shell_check_help_request(argc, argv, &shorthelp );

	if (!print_usage)  
	{
		if (argc == 3 || argc == 2)  
		{	
		
			StopGetData();
	
			_lwsem_wait(&UpgradeStateSem);
			UpgradeState = 0;
			_lwsem_post(&UpgradeStateSem);
			
			for (i=0; i<4; i++)
			{
				hex[0] = argv[1][i*2];
				hex[1] = argv[1][i*2+1];
				strupr(hex);
				uid[i] = ahextoi(hex);
			}
			
			
			if (argc == 2)
			{
				retrytimes = 1;
			}
			else
			{
				retrytimes = atoi(argv[2]);
			}
			printf("��̽����=%d\n", retrytimes);
			
			//���������ַΪ0xFE,Ƶ��Ϊ430M
			RFWorkReserve(430000);
			
			dicor_waite_rf(RF_GET_DATA_END,100); 
			dicor_waite_rf(RF_GET_DATA_END,100); 
			dicor_waite_rf(RF_GET_DATA_END,100); 
				
			for (i=0; i<retrytimes; i++)
			{
				if (retrytimes != 0)
				{
					printf("retry! retrytimes=%d\n", i+1);
				}
			
				//�ȷ���̽������
				memset(PackageSendbuf.data, 0, VALIDDATALEN);
				memset(&PackageRecvbuf, 0, 32);
				PackageSendbuf.head1 = 0xFE;
				PackageSendbuf.head2 = PACKAGEHEAD2;
				PackageSendbuf.addr = 3;
				PackageSendbuf.data[0] = uid[0];
				PackageSendbuf.data[1] = uid[1];
				PackageSendbuf.data[2] = uid[2];
				PackageSendbuf.data[3] = uid[3];
				sendbuf = (uint_8*) &PackageSendbuf;
				printf("����̽���豸����\n");
				RFSimpleTxPacket(sendbuf,0x00, 0xFE);
				printf("�ȴ���Ӧ...\n");
				result = _lwevent_wait_ticks(&RF_NET_event, RF_GET_DATA_END, TRUE, 1000);
				_lwevent_clear(&RF_NET_event, RF_GET_DATA_END);

				if (result == LWEVENT_WAIT_TIMEOUT)
				{
					printf("��ʱ��û���յ���Ӧ��\n");
					if (i < retrytimes-1)
					{
						//����
						printf("timeout!\n");
					//	_time_delay(500);
						continue;
					}
					else
					{
						printf("timeout! stop send!\n");
						printf("��ʱ��û���յ���Ӧ��û��̽�⵽�豸\n");
						state = 1;
						goto DetectDevice430END;
					}
				}
						
				RFSimpleRxPacket((uint_8*) &PackageRecvbuf);
				p = (uint_8*) &PackageRecvbuf;
				print_rfrecv_log(p);
				if (PackageRecvbuf.head1 != PACKAGEHEAD1)
				{
					printf("û�ҵ����ݰ�ͷ���������ݣ�����\n");
					if (i < retrytimes-1)
					{
						//����
						printf("sleep 1s\n");
						_time_delay(1000);
						continue;
					}
					else
					{
						printf("stop send!\n");
						printf("̽��ʧ��\n");
						state = 1;
						goto DetectDevice430END;
					}
				}
				if (PackageRecvbuf.head2 != PACKAGEHEAD2)
				{
					printf("̽�����!�������%d\n", PackageRecvbuf.head2);
					if (i < retrytimes - 1)
					{
						//����
						printf("sleep 1s\n");
						_time_delay(1000);
						continue;
					}
					else
					{
						printf("stop send!\n");
						printf("̽��ʧ��\n");
						state = 1;
						goto DetectDevice430END;
					}
				}
				else
				{
					printf("̽��ɹ���\n");
					printf("��Ϣ���£�\n");
					printf("================================================\n");
					printf("[���е�ַ] = 0x%02X\n", PackageRecvbuf.data[0]);
					if (PackageRecvbuf.data[1])
					{
						printf("[�豸����] = ���豸\n");
					}
					else
					{
						printf("[�豸����] = ��ͷ�豸\n");
					}
					printf("[�����ַ] = 0x%02X\n", PackageRecvbuf.data[2]);
					printf("[������ַ] = 0x%02X\n", PackageRecvbuf.data[3]);
					printf("[�豸��ַ] = 0x%02X\n", PackageRecvbuf.data[4]);
					printf("[���豸��] = %d\n", PackageRecvbuf.data[5]);
					printf("[UID] = %02X-%02X-%02X-%02X\n", PackageRecvbuf.data[7], PackageRecvbuf.data[8], PackageRecvbuf.data[9], PackageRecvbuf.data[10]);
					printf("[������־λ] = %d\n", PackageRecvbuf.data[11]);
					printf("================================================\n");
				}
				if (result == MQX_OK)
				{
					break;
				}
			}

			
			
			
			
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
			printf("%s <0xsubnet> <0xdevaddr>\n", argv[0]);
		} 
		else  
		{
			printf("Usage: %s <0xsubnet> <0xdevaddr>\n", argv[0]);
		}
	}

DetectDevice430END:

	return return_code;
}  

*/
/*FUNCTION*-------------------------------------------------------------
*
*  Function Name :  
*  Returned Value:  none
*  Comments  :  
*  Usage:  
*
*END*-----------------------------------------------------------------*/

int_32  Shell_DetectDevUpgrade(int_32 argc, char_ptr argv[])
{
	boolean  print_usage, shorthelp = FALSE;
	int_32   return_code = SHELL_EXIT_SUCCESS;
	uint_32 retrytimes = 5;
	uint_8  uid[4];
	uint_8  i;
	UINT_8* sendbuf;
	char hex[3]; 
	uint_8 state = 0;
	_mqx_uint result;
	uint_8* p;
	unsigned char *didiflash;
	TIME_STRUCT time1, time2;
	uint_32 dt;
	uint_32 minute, second, mil;
	uint_8	sub, dev;
	uint_8 netaddr;

	hex[2] = '\0';

	print_usage = Shell_check_help_request(argc, argv, &shorthelp );

	if (!print_usage)  
	{
		if (argc == 2)  
		{	
			for (i=0; i<4; i++)
			{
				hex[0] = argv[1][i*2];
				hex[1] = argv[1][i*2+1];
				strupr(hex);
				uid[i] = ahextoi(hex);
			}
			

			printf("���Ӵ���=%d\n", retrytimes);
			
			
			dicor_waite_rf(RF_GET_DATA_END,100); 
			dicor_waite_rf(RF_GET_DATA_END,100); 
			dicor_waite_rf(RF_GET_DATA_END,100); 
				
			for (i=0; i<retrytimes; i++)
			{
				if (retrytimes != 0)
				{
					printf("retry! retrytimes=%d\n", i+1);
				}
			
				//�ȷ���������
			//	memset(PackageSendbuf.data, 0, VALIDDATALEN);
			//	memset(&PackageRecvbuf, 0, 32);
			//	PackageSendbuf.head1 = 0xFE;
			//	PackageSendbuf.head2 = PACKAGEHEAD2;
			//	PackageSendbuf.addr = 4;
			//	PackageSendbuf.data[0] = uid[0];
			//	PackageSendbuf.data[1] = uid[1];
			//	PackageSendbuf.data[2] = uid[2];
			//	PackageSendbuf.data[3] = uid[3];
				sendbuf = (uint_8*) &PackageSendbuf;
				printf("������������\n");
				RFSimpleTxPacket(sendbuf,0x00, 0xFE);
				printf("�ȴ���Ӧ...\n");
				result = _lwevent_wait_ticks(&RF_NET_event, RF_GET_DATA_END, TRUE, 1000);
				_lwevent_clear(&RF_NET_event, RF_GET_DATA_END);

				if (result == LWEVENT_WAIT_TIMEOUT)
				{
					printf("��ʱ��û���յ���Ӧ��\n");
					if (i < retrytimes-1)
					{
						//����
						printf("timeout! sleep 1s\n");
						_time_delay(1000);
						continue;
					}
					else
					{
						printf("timeout! stop send!\n");
						printf("��ʱ��û���յ���Ӧ��û�����ӵ��豸\n");
						state = 1;
						goto DetectDevUpgradeEND;
					}
				}
						
				RFSimpleRxPacket((uint_8*) &PackageRecvbuf);
				p = (uint_8*) &PackageRecvbuf;
				print_rfrecv_log(p);
			//	if (PackageRecvbuf.head1 != PACKAGEHEAD1)
				{
					printf("û�ҵ����ݰ�ͷ���������ݣ�����\n");
					if (i < retrytimes-1)
					{
						//����
						printf("sleep 1s\n");
						_time_delay(1000);
						continue;
					}
					else
					{
						printf("stop send!\n");
						printf("����ʧ��\n");
						state = 1;
						goto DetectDevUpgradeEND;
					}
				}
				
				/*
				if (PackageRecvbuf.head2 != PACKAGEHEAD2)
				{
					printf("���ӳ���!�������%d\n", PackageRecvbuf.head2);
					if (i < retrytimes - 1)
					{
						//����
						printf("sleep 1s\n");
						_time_delay(1000);
						continue;
					}
					else
					{
						printf("stop send!\n");
						printf("����ʧ��\n");
						state = 1;
						goto DetectDevUpgradeEND;
					}
				}
				else
				*/
				{	
					if (uid[0]==0xFE && uid[1]==0xFE && uid[2]==0xFE && uid[3]==0xFE)
					{
						printf("���ӳɹ���\n");
					}
					else
					{
						if (PackageRecvbuf.data[0]==uid[0] && PackageRecvbuf.data[1]==uid[1] && PackageRecvbuf.data[2]==uid[2] && PackageRecvbuf.data[3]==uid[3])
						{
							printf("���ӳɹ���\n");
						}
						else
						{
							printf("UIDУ��������Ӳ��ɹ�\n");
							state = 1;
							goto DetectDevUpgradeEND;
						}
					}
				}
				if (result == MQX_OK)
				{
					break;
				}
			}

			//
				sub = 0x00;
				dev = 0xFE;
				netaddr = 0xFE;
			   //send begin command
			  	//���������ⲿ�ڴ�
				didiflash = (unsigned char *) _mem_alloc_zero_from(_user_pool_id, BOOT_DIDIPROGRAMABLESIZE_FOR8245);
				if (didiflash == NULL)
				{
					printf("error when mem alloc didiflash buf\r\n");	
					return -1;
				}
				//����TFTP���س���SD����
				
				printf("���ڴ�SD��������DiDi�����ļ����ڴ沢���������Ե�...\n");
			//	if (!Boot_AnalysisSRecord(DiDiFileName, didiflash))
				{
					//
					printf("����DiDi�����ļ�[%s]���ڴ���ɣ��������ɹ���\n", DiDiFileName);
					_time_get(&time1);
	
					if (Boot_UpgradeSubDiDi(sub, dev, didiflash, netaddr))
					{
						printf("Failed in Upgrade DiDi 0x%02X 0x%02X\n",sub, dev);
						printf("����DiDi 0x%02X 0x%02X ʧ�ܣ������ԣ�\n", sub, dev);
					}
					else
					{
						printf("Succeed Upgrade DiDi 0x%02X 0x%02X\n", sub, dev);
						printf("����DiDi 0x%02X 0x%02X �ɹ���\n", sub, dev);
					}
					_time_get(&time2);
					dt = (time2.SECONDS*1000+time2.MILLISECONDS)-(time1.SECONDS*1000+time1.MILLISECONDS);
					minute = dt / 60000;
					mil = dt % 1000;
					second = (dt-minute*60000) / 1000;
					printf("total time [%dms]=[%dm %ds %dms]\n", dt, minute, second, mil);
				}
			//	else
				{
					printf("error when analysis didi program file\n");
					printf("��������DiDi�����ļ���������ls,dir������������ļ�[%s]�Ƿ���ڻ��ʽ�Ƿ���ȷ��\n", DiDiFileName);
				}
				_mem_free(didiflash);
	
				
			
			
			
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
			printf("%s <0xsubnet> <0xdevaddr>\n", argv[0]);
		} 
		else  
		{
			printf("Usage: %s <0xsubnet> <0xdevaddr>\n", argv[0]);
		}
	}

DetectDevUpgradeEND:

	return return_code;
} 

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name :  
*  Returned Value:  none
*  Comments  :  
*  Usage:  
*
*END*-----------------------------------------------------------------*/

int_32  Shell_DetectDevUpgrade8245(int_32 argc, char_ptr argv[],uint_8* data_buffer)
{
	boolean  print_usage, shorthelp = FALSE;
	int_32   return_code = SHELL_EXIT_SUCCESS;
	uint_32 retrytimes = 5;
	uint_8  uid[4];
	uint_8  i;
	uint_8 len;
	UINT_8* sendbuf;
	char hex[3]; 
	uint_8 state = 0;
	_mqx_uint result;
	uint_8* p;
	unsigned char *didiflash;
	TIME_STRUCT time1, time2;
	uint_32 dt;
	uint_32 minute, second, mil;
//	uint_8	sub, dev;
//	uint_8 netaddr;
	uint_16 addr_f,addr_r;

	hex[2] = '\0';

	print_usage = Shell_check_help_request(argc, argv, &shorthelp );

	if (!print_usage)  
	{
		if (argc == 2)  
		{	
			for (i=0; i<4; i++)
			{
				hex[0] = argv[1][i*2];
				hex[1] = argv[1][i*2+1];
				strupr(hex);
				uid[i] = ahextoi(hex);
			}
			

			printf("���Ӵ���=%d\n", retrytimes);
			
			
			dicor_waite_rf(RF_GET_DATA_END,100); 
			dicor_waite_rf(RF_GET_DATA_END,100); 
			dicor_waite_rf(RF_GET_DATA_END,100); 
				
			for (i=0; i<retrytimes; i++)
			{
				if (retrytimes != 0)
				{
					printf("retry! retrytimes=%d\n", i+1);
				}
			
				//�ȷ���������
			//	memset(PackageSendbuf.data, 0, VALIDDATALEN);
			//	memset(&PackageRecvbuf, 0, 32);
			//	PackageSendbuf.head1 = 0xFE;
			//	PackageSendbuf.head2 = PACKAGEHEAD2;
			//	PackageSendbuf.addr = 4;
			//	PackageSendbuf.data[0] = uid[0];
			//	PackageSendbuf.data[1] = uid[1];
			//	PackageSendbuf.data[2] = uid[2];
			//	PackageSendbuf.data[3] = uid[3];
				sendbuf = (uint_8*) &PackageSendbuf;
				printf("������������\n");
				RFSimpleTxPacket(sendbuf,0x00, 0xFE);
				printf("�ȴ���Ӧ...\n");
				result = _lwevent_wait_ticks(&RF_NET_event, RF_GET_DATA_END, TRUE, 1000);
				_lwevent_clear(&RF_NET_event, RF_GET_DATA_END);

				if (result == LWEVENT_WAIT_TIMEOUT)
				{
					printf("��ʱ��û���յ���Ӧ��\n");
					if (i < retrytimes-1)
					{
						//����
						printf("timeout! sleep 1s\n");
						_time_delay(1000);
						continue;
					}
					else
					{
						printf("timeout! stop send!\n");
						printf("��ʱ��û���յ���Ӧ��û�����ӵ��豸\n");
						state = 1;
						goto DetectDevUpgrade8245END;
					}
				}
						
				RFSimpleRxPacket((uint_8*) &PackageRecvbuf);
				p = (uint_8*) &PackageRecvbuf;
				print_rfrecv_log(p);
				/*
				if (PackageRecvbuf.head1 != PACKAGEHEAD1)
				{
					printf("û�ҵ����ݰ�ͷ���������ݣ�����\n");
					if (i < retrytimes-1)
					{
						//����
						printf("sleep 1s\n");
						_time_delay(1000);
						continue;
					}
					else
					{
						printf("stop send!\n");
						printf("����ʧ��\n");
						state = 1;
						goto DetectDevUpgrade8245END;
					}
				}
				if (PackageRecvbuf.head2 != PACKAGEHEAD2)
				{
					printf("���ӳ���!�������%d\n", PackageRecvbuf.head2);
					if (i < retrytimes - 1)
					{
						//����
						printf("sleep 1s\n");
						_time_delay(1000);
						continue;
					}
					else
					{
						printf("stop send!\n");
						printf("����ʧ��\n");
						state = 1;
						goto DetectDevUpgrade8245END;
					}
				}
				else
				*/
				{	
					if (uid[0]==0xFE && uid[1]==0xFE && uid[2]==0xFE && uid[3]==0xFE)
					{
						printf("���ӳɹ���\n");
					}
					else
					{
						if (PackageRecvbuf.data[0]==uid[0] && PackageRecvbuf.data[1]==uid[1] && PackageRecvbuf.data[2]==uid[2] && PackageRecvbuf.data[3]==uid[3])
						{
							printf("���ӳɹ���\n");
						}
						else
						{
							printf("UIDУ��������Ӳ��ɹ�\n");
							state = 1;
							goto DetectDevUpgrade8245END;
						}
					}
				}
				if (result == MQX_OK)
				{
					break;
				}
			}

			//
			//	sub = 0x00;
			//	dev = 0xFE;
			//	netaddr = 0xFE;
			   //send begin command
			  	//���������ⲿ�ڴ�
				didiflash = (unsigned char *) _mem_alloc_zero_from(_user_pool_id, BOOT_DIDIPROGRAMABLESIZE_FOR8245);
				if (didiflash == NULL)
				{
					printf("error when mem alloc didiflash buf\r\n");	
					return -1;
				}
				//����TFTP���س���SD����
				
				printf("���ڴ�SD��������DiDi�����ļ����ڴ沢���������Ե�...\n");
				if (!Boot_AnalysisSRecord_8245(DiDiFileName, didiflash,data_buffer))
				{
					//
					printf("����DiDi�����ļ�[%s]���ڴ���ɣ��������ɹ���\n", DiDiFileName);
					_time_get(&time1);
	
					if (Boot_UpgradeSubDiDi_8245(addr_f,didiflash,addr_r))
					{
						printf("Failed in Upgrade DiDi 0x%04X 0x%04X\n",addr_f, addr_r);
						printf("����DiDi 0x%04X 0x%04X ʧ�ܣ������ԣ�\n", addr_f, addr_r);
					}
					else
					{
						printf("Succeed Upgrade DiDi 0x%04X 0x%04X\n", addr_f, addr_r);
						printf("����DiDi 0x%04X 0x%04X �ɹ���\n", addr_f, addr_r);
					}
					_time_get(&time2);
					dt = (time2.SECONDS*1000+time2.MILLISECONDS)-(time1.SECONDS*1000+time1.MILLISECONDS);
					minute = dt / 60000;
					mil = dt % 1000;
					second = (dt-minute*60000) / 1000;
					printf("total time [%dms]=[%dm %ds %dms]\n", dt, minute, second, mil);
				}
				else
				{
					printf("error when analysis didi program file\n");
					printf("��������DiDi�����ļ���������ls,dir������������ļ�[%s]�Ƿ���ڻ��ʽ�Ƿ���ȷ��\n", DiDiFileName);
				}
				_mem_free(didiflash);
	
				
			
			
			
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
			printf("%s <0xsubnet> <0xdevaddr>\n", argv[0]);
		} 
		else  
		{
			printf("Usage: %s <0xsubnet> <0xdevaddr>\n", argv[0]);
		}
	}

DetectDevUpgrade8245END:

	return return_code;
} 



/*FUNCTION*-------------------------------------------------------------
*
*  Function Name :  
*  Returned Value:  none
*  Comments  :  
*  Usage:  
*
*END*-----------------------------------------------------------------*/

int_32  Shell_PrintDiDiEeprom(int_32 argc, char_ptr argv[])
{
	boolean  print_usage, shorthelp = FALSE;
	int_32   return_code = SHELL_EXIT_SUCCESS;
	uint_32 retrytimes = 3;
	uint_8  uid[4];
	uint_8  i;
	UINT_8* sendbuf;
	char hex[3]; 
	uint_8 state = 0;
	_mqx_uint result;
	uint_8* p;

	uint_16 addr;
	uint_16 len;


	hex[2] = '\0';

	print_usage = Shell_check_help_request(argc, argv, &shorthelp );

	if (!print_usage)  
	{
		if (argc == 4)  
		{	
			for (i=0; i<4; i++)
			{
				hex[0] = argv[1][i*2];
				hex[1] = argv[1][i*2+1];
				strupr(hex);
				uid[i] = ahextoi(hex);
			}
			
			
			hex[0] = argv[2][2];
			hex[1] = argv[2][3];
			strupr(hex);
				
			addr = ahextoi(hex);
			addr <<= 8;
			hex[0] = argv[2][4];
			hex[1] = argv[2][5];
			strupr(hex);
			addr += ahextoi(hex);
			
			len = atoi(argv[3]);

		//	printf("���Դ���=%d\n", retrytimes);
			
			
			dicor_waite_rf(RF_GET_DATA_END,100); 
			dicor_waite_rf(RF_GET_DATA_END,100); 
			dicor_waite_rf(RF_GET_DATA_END,100); 
				
			for (i=0; i<retrytimes; i++)
			{
				if (i != 0)
				{
					printf("retry! retrytimes=%d\n", i+1);
				}
			
		
			//	memset(PackageSendbuf.data, 0, VALIDDATALEN);
			//	memset(&PackageRecvbuf, 0, 32);
		//		PackageSendbuf.head1 = 0xFE;
		//		PackageSendbuf.head2 = PACKAGEHEAD2;
		//		PackageSendbuf.addr = 5;
			//	PackageSendbuf.data[0] = uid[0];
			//	PackageSendbuf.data[1] = uid[1];
			//	PackageSendbuf.data[2] = uid[2];
			//	PackageSendbuf.data[3] = uid[3];
				
			//	PackageSendbuf.data[4] = (uint_8)(addr >> 8);
			//	PackageSendbuf.data[5] = (uint_8)addr;
		//		PackageSendbuf.data[6] = (uint_8)(len >> 8);
		//		PackageSendbuf.data[7] = (uint_8)len;
		
				
				sendbuf = (uint_8*) &PackageSendbuf;
				printf("���Ͷ�ȡDiDi EEPROM����\n");
			//	RFSimpleTxPacket(sendbuf,0x00, 0xFE);
				printf("�ȴ���Ӧ...\n");
				result = _lwevent_wait_ticks(&RF_NET_event, RF_GET_DATA_END, TRUE, 1000);
				_lwevent_clear(&RF_NET_event, RF_GET_DATA_END);

				if (result == LWEVENT_WAIT_TIMEOUT)
				{
					printf("��ʱ��û���յ���Ӧ��\n");
					if (i < retrytimes-1)
					{
						//����
						printf("timeout! sleep 1s\n");
						_time_delay(1000);
						continue;
					}
					else
					{
						printf("timeout! stop send!\n");
						printf("��ʱ��û���յ���Ӧ����ȡDiDi EEPROM����ʧ��\n");
						state = 1;
						goto PrintDiDiEepromEND;
					}
				}
						
				RFSimpleRxPacket((uint_8*) &PackageRecvbuf);
				p = (uint_8*) &PackageRecvbuf;
				print_rfrecv_log(p);
				/*
				if (PackageRecvbuf.head1 != PACKAGEHEAD1)
				{
					printf("û�ҵ����ݰ�ͷ���������ݣ�����\n");
					if (i < retrytimes-1)
					{
						//����
						printf("sleep 1s\n");
						_time_delay(1000);
						continue;
					}
					else
					{
						printf("stop send!\n");
						printf("��ȡDiDi EEPROM����ʧ��\n");
						state = 1;
						goto PrintDiDiEepromEND;
					}
				}
				
				*/
			/*	
				if (PackageRecvbuf.head2 != PACKAGEHEAD2)
				{
				//	printf("��ȡDiDi EEPROM���ݳ���!�������%d\n", PackageRecvbuf.head2);
					if (i < retrytimes - 1)
					{
						//����
						printf("sleep 1s\n");
						_time_delay(1000);
						continue;
					}
					else
					{
						printf("stop send!\n");
						printf("��ȡDiDi EEPROM����ʧ��\n");
						state = 1;
						goto PrintDiDiEepromEND;
					}
				}
				else
				*/
				{	
					printf("��ȡ�ɹ���\n");
					printf("\n**********************************\n");
					for (i=0; i<len; i++)
					{
						printf("[%04XH]\t[%02X%02X]\n", addr+i, PackageRecvbuf.data[2*i], PackageRecvbuf.data[2*i+1]);
					}
					printf("\n**********************************\n");
				}
				if (result == MQX_OK)
				{
					break;
				}
			}
			
			
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
			printf("%s <0xsubnet> <0xdevaddr>\n", argv[0]);
		} 
		else  
		{
			printf("Usage: %s <0xsubnet> <0xdevaddr>\n", argv[0]);
		}
	}

PrintDiDiEepromEND:

	return return_code;
} 




/*FUNCTION*-------------------------------------------------------------
*
*  Function Name :  
*  Returned Value:  none
*  Comments  :  
*  Usage:  
*
*END*-----------------------------------------------------------------*/

int_32  Shell_SetDiDiEeprom(int_32 argc, char_ptr argv[])
{
	boolean  print_usage, shorthelp = FALSE;
	int_32   return_code = SHELL_EXIT_SUCCESS;
	uint_32 retrytimes = 3;
	uint_8  uid[4];
	uint_8  i;
	UINT_8* sendbuf;
	char hex[3]; 
	uint_8 state = 0;
	_mqx_uint result;
	uint_8* p;

	uint_16 addr;
	uint_16 val;


	hex[2] = '\0';

	print_usage = Shell_check_help_request(argc, argv, &shorthelp );




	if (!print_usage)  
	{
		if (argc == 4)  
		{	
			for (i=0; i<4; i++)
			{
				hex[0] = argv[1][i*2];
				hex[1] = argv[1][i*2+1];
				strupr(hex);
				uid[i] = ahextoi(hex);
			}
			
			
			hex[0] = argv[2][2];
			hex[1] = argv[2][3];
			strupr(hex);
				
			addr = ahextoi(hex);
			addr <<= 8;
			hex[0] = argv[2][4];
			hex[1] = argv[2][5];
			strupr(hex);
			addr += ahextoi(hex);
			
			
			
			
			hex[0] = argv[3][2];
			hex[1] = argv[3][3];
			strupr(hex);
				
			val = ahextoi(hex);
			val <<= 8;
			hex[0] = argv[3][4];
			hex[1] = argv[3][5];
			strupr(hex);
			val += ahextoi(hex);
			

		//	printf("���Դ���=%d\n", retrytimes);
			
			
			dicor_waite_rf(RF_GET_DATA_END,100); 
			dicor_waite_rf(RF_GET_DATA_END,100); 
			dicor_waite_rf(RF_GET_DATA_END,100); 
				
			for (i=0; i<retrytimes; i++)
			{
				if (i != 0)
				{
					printf("retry! retrytimes=%d\n", i+1);
				}
			
		
		//		memset(PackageSendbuf.data, 0, VALIDDATALEN);
		//		memset(&PackageRecvbuf, 0, 32);
			//	PackageSendbuf.head1 = 0xFE;
			//	PackageSendbuf.head2 = PACKAGEHEAD2;
			//	PackageSendbuf.addr = 6;
		//		PackageSendbuf.data[0] = uid[0];
		//		PackageSendbuf.data[1] = uid[1];
		////		PackageSendbuf.data[2] = uid[2];
		//		PackageSendbuf.data[3] = uid[3];
				
		//		PackageSendbuf.data[4] = (uint_8)(addr >> 8);
		///		PackageSendbuf.data[5] = (uint_8)addr;
		//		PackageSendbuf.data[6] = (uint_8)(val >> 8);
		//		PackageSendbuf.data[7] = (uint_8)val;
		
				
				sendbuf = (uint_8*) &PackageSendbuf;
				printf("��������DiDi EEPROM����\n");
		//		RFSimpleTxPacket(sendbuf,0x00, 0xFE);
				printf("�ȴ���Ӧ...\n");
				result = _lwevent_wait_ticks(&RF_NET_event, RF_GET_DATA_END, TRUE, 1000);
				_lwevent_clear(&RF_NET_event, RF_GET_DATA_END);

				if (result == LWEVENT_WAIT_TIMEOUT)
				{
					printf("��ʱ��û���յ���Ӧ��\n");
					if (i < retrytimes-1)
					{
						//����
						printf("timeout! sleep 1s\n");
						_time_delay(1000);
						continue;
					}
					else
					{
						printf("timeout! stop send!\n");
						printf("��ʱ��û���յ���Ӧ������DiDi EEPROM����ʧ��\n");
						state = 1;
						goto SetDiDiEepromEND;
					}
				}
						
				RFSimpleRxPacket((uint_8*) &PackageRecvbuf);
				p = (uint_8*) &PackageRecvbuf;
				print_rfrecv_log(p);
				/*
				if (PackageRecvbuf.head1 != PACKAGEHEAD1)
				{
					printf("û�ҵ����ݰ�ͷ���������ݣ�����\n");
					if (i < retrytimes-1)
					{
						//����
						printf("sleep 1s\n");
						_time_delay(1000);
						continue;
					}
					else
					{
						printf("stop send!\n");
						printf("����DiDi EEPROM����ʧ��\n");
						state = 1;
						goto SetDiDiEepromEND;
					}
				}
				if (PackageRecvbuf.head2 != PACKAGEHEAD2)
				{
					printf("����DiDi EEPROM���ݳ���!�������%d\n", PackageRecvbuf.head2);
					if (i < retrytimes - 1)
					{
						//����
						printf("sleep 1s\n");
						_time_delay(1000);
						continue;
					}
					else
					{
						printf("stop send!\n");
						printf("����DiDi EEPROM����ʧ��\n");
						state = 1;
						goto SetDiDiEepromEND;
					}
				}
				else
				*/
				{	
					printf("���óɹ���\n");
				}
				if (result == MQX_OK)
				{
					break;
				}
			}
			
			
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
			printf("%s <0xsubnet> <0xdevaddr>\n", argv[0]);
		} 
		else  
		{
			printf("Usage: %s <0xsubnet> <0xdevaddr>\n", argv[0]);
		}
	}

SetDiDiEepromEND:

	return return_code;
} 







/*
int_32  Shell_TestRF(int_32 argc, char_ptr argv[])
{
	boolean  print_usage, shorthelp = FALSE;
	int_32   return_code = SHELL_EXIT_SUCCESS;
	uint_32 retrytimes;
	uint_8  uid[4];
	uint_8  i;
	UINT_8* sendbuf;
	char hex[3]; 
	uint_8 state = 0;
	_mqx_uint result;
	uint_8* p;


	hex[2] = '\0';

	print_usage = Shell_check_help_request(argc, argv, &shorthelp );

	if (!print_usage)  
	{
		if (argc == 3 || argc == 2)  
		{	
		
			StopGetData();
	
			_lwsem_wait(&UpgradeStateSem);
			UpgradeState = 0;
			_lwsem_post(&UpgradeStateSem);
			
			for (i=0; i<4; i++)
			{
				hex[0] = argv[1][i*2];
				hex[1] = argv[1][i*2+1];
				strupr(hex);
				uid[i] = ahextoi(hex);
			}
			
			
			if (argc == 2)
			{
				retrytimes = 1;
			}
			else
			{
				retrytimes = atoi(argv[2]);
			}
			printf("��̽����=%d\n", retrytimes);
			
			//���������ַΪ0xFE,Ƶ��Ϊ430M
			RFWorkReserve(430000);
			
			dicor_waite_rf(RF_GET_DATA_END,100); 
			dicor_waite_rf(RF_GET_DATA_END,100); 
			dicor_waite_rf(RF_GET_DATA_END,100); 
				
			for (i=0; i<retrytimes; i++)
			{
				if (retrytimes != 0)
				{
					printf("retry! retrytimes=%d\n", i+1);
				}
			
				//�ȷ���̽������
				memset(PackageSendbuf.data, 0, VALIDDATALEN);
				memset(&PackageRecvbuf, 0, 32);
				PackageSendbuf.head1 = 0xFE;
				PackageSendbuf.head2 = PACKAGEHEAD2;
				PackageSendbuf.addr = 3;
				PackageSendbuf.data[0] = uid[0];
				PackageSendbuf.data[1] = uid[1];
				PackageSendbuf.data[2] = uid[2];
				PackageSendbuf.data[3] = uid[3];
				sendbuf = (uint_8*) &PackageSendbuf;
				printf("����̽���豸����\n");
				RFSimpleTxPacket(sendbuf,0x00, 0xFE);
				printf("�ȴ���Ӧ...\n");
				result = _lwevent_wait_ticks(&RF_NET_event, RF_GET_DATA_END, TRUE, 1000);
				_lwevent_clear(&RF_NET_event, RF_GET_DATA_END);

				if (result == LWEVENT_WAIT_TIMEOUT)
				{
					printf("��ʱ��û���յ���Ӧ��\n");
					if (i < retrytimes-1)
					{
						//����
						printf("timeout! sleep 1s\n");
						_time_delay(1000);
						continue;
					}
					else
					{
						printf("timeout! stop send!\n");
						printf("��ʱ��û���յ���Ӧ��û��̽�⵽�豸\n");
						state = 1;
						goto TestRFEND;
					}
				}
						
				RFSimpleRxPacket((uint_8*) &PackageRecvbuf);
				p = (uint_8*) &PackageRecvbuf;
				print_rfrecv_log(p);
				if (PackageRecvbuf.head1 != PACKAGEHEAD1)
				{
					printf("û�ҵ����ݰ�ͷ���������ݣ�����\n");
					if (i < retrytimes-1)
					{
						//����
						printf("sleep 1s\n");
						_time_delay(1000);
						continue;
					}
					else
					{
						printf("stop send!\n");
						printf("̽��ʧ��\n");
						state = 1;
						goto TestRFEND;
					}
				}
				if (PackageRecvbuf.head2 != PACKAGEHEAD2)
				{
					printf("̽�����!�������%d\n", PackageRecvbuf.head2);
					if (i < retrytimes - 1)
					{
						//����
						printf("sleep 1s\n");
						_time_delay(1000);
						continue;
					}
					else
					{
						printf("stop send!\n");
						printf("̽��ʧ��\n");
						state = 1;
						goto TestRFEND;
					}
				}
				else
				{
					printf("̽��ɹ���\n");
					printf("��Ϣ���£�\n");
					printf("================================================\n");
					printf("[���е�ַ] = 0x%02X\n", PackageRecvbuf.data[0]);
					if (PackageRecvbuf.data[1])
					{
						printf("[�豸����] = ���豸\n");
					}
					else
					{
						printf("[�豸����] = ��ͷ�豸\n");
					}
					printf("[�����ַ] = 0x%02X\n", PackageRecvbuf.data[2]);
					printf("[������ַ] = 0x%02X\n", PackageRecvbuf.data[3]);
					printf("[�豸��ַ] = 0x%02X\n", PackageRecvbuf.data[4]);
					printf("[���豸��] = %d\n", PackageRecvbuf.data[5]);
					printf("[UID] = %02X-%02X-%02X-%02X\n", PackageRecvbuf.data[7], PackageRecvbuf.data[8], PackageRecvbuf.data[9], PackageRecvbuf.data[10]);
					printf("[������־λ] = %d\n", PackageRecvbuf.data[11]);
					printf("================================================\n");
				}
		
			}

			
			
			
			
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
			printf("%s <0xsubnet> <0xdevaddr>\n", argv[0]);
		} 
		else  
		{
			printf("Usage: %s <0xsubnet> <0xdevaddr>\n", argv[0]);
		}
	}

TestRFEND:

	return return_code;
} 

*/

/* EOF*/
