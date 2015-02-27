/*************************************************************
* 
* Copyright (c) convertergy ;
* All Rights Reserved
* FileName: eeprom.c
* Version : 
* Date    : 2011/09/08
* ower    : Alex
*
* Comments:*
*
*
***************************************************************/

#include <mqx.h>
#include <bsp.h>
#include <rtcs.h>
#include <string.h>
#include <lwtimer.h>
#include <mfs.h>
#include <fio.h>
#include "dicor_upload.h"
#include "eeprom.h"

extern UINT_8 SERVER_MODE;
extern void Dicor_Reboot(void);

_mem_size base_addr;

volatile EEPROMDATA_PTR  pEeprom;
static MQX_FILE_PTR   flash_hdl;

uchar CoodAddr;
uchar workmode;
BASECONFIGTABLE_PTR pBaseConfig;

PARAMS_TOBOOTLOADER_PTR pParams_toBootLoader;

const uchar softversion[3] = SOFTVERSION;
 
static MQX_FILE_PTR flash_open(char_ptr name);
static void flash_close(MQX_FILE_PTR flash);
static void size_compare( MQX_FILE_PTR flash_hdl, _mqx_int i, _mqx_int read_write_size );
static void GetEepromBaseAddr(void);

void EepromReadCoodaddr(void);
void EepromWriteUpdata(uint_8 *);
void EepromEras(_mem_size seek_location, unsigned char *secter_erase);

/*FUNCTION*-----------------------------------------------------
* 
* Task Name    : flash_open
* Comments     :
*    Open the flash device
*
*END*-----------------------------------------------------*/
static MQX_FILE_PTR flash_open(char_ptr name) 
{
    MQX_FILE_PTR flash_hdl = NULL;
    
    /* Open the flash device */
    flash_hdl = fopen(name, NULL);
    if (flash_hdl == NULL) {
        printf("\nUnable to open flash device %s", name);
        //_task_block();
        Dicor_Reboot();
    } /* Endif */
//    printf("\nFlash device %s opened", name);       
    return flash_hdl;
}

/*FUNCTION*-----------------------------------------------------
* 
* Task Name    : flash_close
* Comments     :
*    Close the flash device
*
*END*-----------------------------------------------------*/
static void flash_close(MQX_FILE_PTR flash)
{
    fclose(flash);
//    printf("\n\rFlash closed.");    
}

/*FUNCTION*-----------------------------------------------------
* 
* Task Name    : size_compare
* Comments     :
*    function compare i and read_write_size
*
*END*-----------------------------------------------------*/
static void size_compare( MQX_FILE_PTR flash_hdl, _mqx_int i, _mqx_int read_write_size )
{
    if (i != read_write_size ) {
        printf("\nFailed to write flash, size returned:%d expected %d", i,
        read_write_size);
        printf("\nTEC:0x%X FERROR:0x%X", _task_get_error(), ferror(flash_hdl));
        //_task_block();
        Dicor_Reboot();
    } /* Endif */   
}


static void GetEepromBaseAddr(void)
{
	_mqx_uint      error_code;
	
		  /* Open the flash device */
   flash_hdl = flash_open(FLASH_NAME);
   
	error_code = ioctl(flash_hdl, FLASH_IOCTL_GET_BASE_ADDRESS, &base_addr);
   if (error_code != MQX_OK) {
      printf("\nFLASH_IOCTL_GET_BASE_ADDRESS failed.");
      //_task_block();
      Dicor_Reboot();
   } else {
      printf("\nThe BASE_ADDRESS: 0x%x", base_addr);
   } /* Endif */
   
   	flash_close(flash_hdl);
   	
}

 

void EepromWrite(uchar_ptr write_buffer, _mem_size seek_location, _mem_size read_write_size)
{
	_mqx_int	result;  
	flash_hdl = flash_open(FLASH_NAME);
	printf ("---write flashx 1---\n");
	_io_ioctl(flash_hdl, FLASH_IOCTL_ENABLE_SECTOR_CACHE, NULL); 
	printf ("---write flashx 2---\n");
	fseek(flash_hdl, seek_location, IO_SEEK_SET);
	printf ("---write flashx 3---\n");
	result = write(flash_hdl, write_buffer, read_write_size );
	printf ("---write flashx 4--\n");
	flash_close(flash_hdl);
}

void EepromRead(uchar_ptr read_buffer, _mem_size seek_location, _mem_size read_write_size)
{
   	_mqx_int	result;
   	flash_hdl = flash_open(FLASH_NAME);
   	printf ("---read flashx 1---\n");
   	_io_ioctl(flash_hdl, FLASH_IOCTL_ENABLE_SECTOR_CACHE, NULL); 
   	printf ("---read flashx 2---\n");
	fseek(flash_hdl, seek_location, IO_SEEK_SET);
	printf ("---read flashx 3---\n");
	result = read(flash_hdl, read_buffer, read_write_size );
	printf ("---read flashx 4---\n");
	flash_close(flash_hdl);
}


void ChkEepromFirstUse(void)
{
	unsigned char writebuf[WRITEBUFSIZE];
	char filename[40];

	BASECONFIGTABLE_PTR baseconfig ;
//	{
//		->datacenter[4] = "23",//192,168,8,240
//		//.ipport[] = {0,0,39,16}
//	};
	PARAMS_TOBOOTLOADER_PTR pdata;

	if (!(pBaseConfig->head[0] == 0xA1 && pBaseConfig->head[1] == 0xA2 && pBaseConfig->head[2] == 0xA3 && pBaseConfig->head[3] == 0xA4))
	{
		printf("\nFirst Use!\n");
		baseconfig = (BASECONFIGTABLE_PTR) writebuf;
		
		baseconfig->head[0] = 0xA1;
		baseconfig->head[1] = 0xA2;
		baseconfig->head[2] = 0xA3;
		baseconfig->head[3] = 0xA4;

		baseconfig->softversion[0] = 'V';
		baseconfig->softversion[1] = softversion[0];
		baseconfig->softversion[2] = softversion[1];
		baseconfig->softversion[3] = softversion[2];
		baseconfig->dhcp = 1;
		baseconfig->enabledns = 0;
		baseconfig->enablesntp = 0;
		baseconfig->datacenter[0] = 192;
		baseconfig->datacentermux[0] = 192;
		baseconfig->datacenter[1] = 168;
		baseconfig->datacentermux[1] = 168;
		baseconfig->datacenter[2] = 8;
		baseconfig->datacentermux[2] = 8;
		baseconfig->datacenter[3] = 240;
		baseconfig->datacentermux[3] = 240;
		baseconfig->ipport[0] = 0;
		baseconfig->ipportmux[0] = 0;
		baseconfig->ipport[1] = 0;
		baseconfig->ipportmux[1] = 0;
		baseconfig->ipport[2] = 39;
		baseconfig->ipportmux[2] = 39;
		baseconfig->ipport[3] = 16;
		baseconfig->ipportmux[3] = 16;
		EepromWrite(writebuf, 0, WRITEBUFSIZE);
	}
	if (pBaseConfig->softversion[1]!=softversion[0] || pBaseConfig->softversion[2]!=softversion[1] || pBaseConfig->softversion[3]!=softversion[2])
	{
		printf("\nSave Soft Version\n");

		writebuf[0] = 'V';
		writebuf[1] = softversion[0];
		writebuf[2] = softversion[1];
		writebuf[3] = softversion[2];
		EepromWrite(writebuf, 8, 4);
	}
	if (strcmp(pParams_toBootLoader->signature, "Cosmos") != 0)
	{
		printf("\nSave CosmosV%d.%d.%d.S19 Version\n", softversion[0],softversion[1],softversion[2]);
		
		pdata = (PARAMS_TOBOOTLOADER_PTR)writebuf;
		strncpy(pdata->signature, "Cosmos", 12);
		pdata->tobootloader.server = IPADDR(115,29,192,154);
		pdata->tobootloader.b_update = 0;
		sprintf(filename, "CosmosV%d.%d.%d.S19", softversion[0], softversion[1], softversion[2]);
		strncpy(pdata->tobootloader.file_name, filename, 40);
		EepromWrite(writebuf, 16*1024, WRITEBUFSIZE);
	}
}

void EepromWriteUpdata(uint_8 *version)
{
	unsigned char writebuf[WRITEBUFSIZE];
	char filename[40];

	//BASECONFIGTABLE_PTR baseconfig;
	PARAMS_TOBOOTLOADER_PTR pdata;

	
	unsigned char i = 32;
	unsigned char *secter_erase = &i;
	_mem_size seek_location = 16384;
	flash_hdl = flash_open(FLASH_NAME);	
	fseek(flash_hdl, seek_location, IO_SEEK_SET);
 	_io_ioctl(flash_hdl, FLASH_IOCTL_ERASE_SECTOR, secter_erase); 
	flash_close(flash_hdl); 	
	
	
	if (strcmp(pParams_toBootLoader->signature, "DICOR") != 0)
	{
		pdata = (PARAMS_TOBOOTLOADER_PTR)writebuf;
		strncpy(pdata->signature, "Cosmos", 12);//12\14
		pdata->tobootloader.server = IPADDR(110,110,110,110);
		pdata->tobootloader.b_update = 1;
		sprintf(filename, "CosmosV%d.%d.%d.S19", version[0],version[1],version[2]);
		strncpy(pdata->tobootloader.file_name, filename, 40);
		EepromWrite(writebuf, 32*512, WRITEBUFSIZE);
	}	
	return ;
}
void EepromWriteCoodaddr(unsigned char *coodaddr)
{

	unsigned char i = 26;
	unsigned char *secter_erase = &i;
	_mem_size seek_location = 13313;

	EepromWrite(coodaddr, 13312, 1);
	flash_hdl = flash_open(FLASH_NAME);
//	EepromWrite(coodaddr, 13313, 1);
	fseek(flash_hdl, seek_location, IO_SEEK_SET);
 	_io_ioctl(flash_hdl, FLASH_IOCTL_ERASE_SECTOR, secter_erase); 
	flash_close(flash_hdl);
	EepromWrite(coodaddr, 13313, 1);	
	EepromReadCoodaddr();
}

void EepromReadCoodaddr(void)
{

	unsigned char p[2];
	EepromRead(p, 13313, 1);//520,3400,13312	
	
	if (p[0] != 0xff) {
	
		CoodAddr = p[0];	
		printf("Read CoodAddrChanged !0xff=0x%02X\r\n", CoodAddr);
	} else if (p[0] == 0xff) {
	
		CoodAddr = COODADDRDEFAULT;
	}
}

void EepromInit(void)
{
	EEPROMDATA_PTR p;
	
	GetEepromBaseAddr();

	p = (EEPROMDATA_PTR) base_addr;
	RegTable = &p->RegTable[0];
	EMN_DownTab = (EMN_ROUTER_TAB_ITEM*)&p->RouterTable[3];
	SubnetDepth = (UINT_8*)&p->RouterTable[256]; 
	pBaseConfig = (BASECONFIGTABLE_PTR) &p->BaseConfig[0];
	pDiAnDiDiTable = (DIANDIDITABLE_PTR)&p->DiDi_DianTable[0];
	pParams_toBootLoader = (PARAMS_TOBOOTLOADER_PTR)&p->params_tobootloaderconfig;
	
	ChkEepromFirstUse();//暂时关闭初始化检查
	if (!memcmp(pBaseConfig->ipport, pBaseConfig->ipportmux, 4)
		&& !memcmp(pBaseConfig->datacenter, pBaseConfig->datacentermux, 4))
		//|| !memcmp(baseconfig->datacentermux, 0, 2))
	{
		SERVER_MODE = 1;
	}
	else 
		SERVER_MODE = 2;	

	if (pBaseConfig->work_mode >= WORKMODE_MAX)
	{
		workmode = 0;
	}
	else
	{
		workmode = pBaseConfig->work_mode;
	}
}

void eepromreadtest(void)
{
	unsigned char test;
	unsigned int i;

	pEeprom = (EEPROMDATA_PTR)base_addr;
	printf("\n**********BaseConfig*********\n");
	for (i=0; i<BASECONFIGSIZE; i++)
	{
		test = pEeprom->BaseConfig[i];
		
		if (i % 10 == 0)
		{
			printf("\n");
		}
		printf("%02X ", test);
	}
	printf("\n**************base over**************\n");
	for (i=0; i<ROUTERTABLESIZE; i++)
	{
		test = pEeprom->RouterTable[i];
	}
		
	printf("\n**********RegTable*********\n");		
	for (i=0; i<REGTABLESIZE; i++)
	{
		test = pEeprom->RegTable[i];

		printf("%02X ", test);
		if (i==7)
		{
			printf("\n");
		}
		else if ((i-7)%6==0)
		{
			printf("\n");
		}
	}
	printf("\n**************************\n");
	for (i=0; i<DIDIDIANTABLESIZE; i++)
	{
		test = pEeprom->DiDi_DianTable[i];
	}
}


void EepromEras(_mem_size seek_location, unsigned char *secter_erase)
{
	MQX_FILE_PTR   flash_hd;
	flash_hd = flash_open(FLASH_NAME);	
	fseek(flash_hd, seek_location, IO_SEEK_SET);
 	_io_ioctl(flash_hd, FLASH_IOCTL_ERASE_SECTOR, secter_erase); 
	flash_close(flash_hd); 		
}



/* EOF */
