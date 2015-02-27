#ifndef __eeprom_h__
#define __eeprom_h__
/**********************************************************************
* 
* Copyright (c) 2011 convertergy ;
* All Rights Reserved
* FileName: eeprom.h 
* Version : 
* Date    : 
*
* Comments:
*
*   This file contains definitions for the upload MG's data to data center
*
*************************************************************************/


#define         FLASH_NAME "flashx:"

#define	WRITEBUFSIZE			512

#define BASECONFIGSIZE			512
#define ROUTERTABLESIZE			512	//前256个用于存路由表，后256字节用于存深度表
#define REGTABLESIZE			6*1024
#define DIDIDIANTABLESIZE		6*1024	
#define DIDIDIANINDEXTABSIZE	512


struct params_tobootloader
{
	unsigned long server;           /**< @brief TFTP服务器
	                                   IP地址
                                     */
    unsigned long b_update;       /**< @brief 是否需要下载
                                     */               
    char file_name[40]; /**< @brief This is the default file name
                                     * to be loaded by TFTP loader if no 
                                     * file name is provided to the "tftp" shell command. @n
                                     * The string must be null-terminated.
                                     */
     //unsigned char reserve[4*1024-40-4-4-12];
};

typedef struct 
{
	char signature[12]; /**< @brief Signature string.@n
                                             * It's used for simple check if configuration 
                                             * structure is present in a persistent storage. 
                                             */
	struct params_tobootloader tobootloader;
   
}PARAMS_TOBOOTLOADER , _PTR_  PARAMS_TOBOOTLOADER_PTR;



//EEPROM的整体分配
typedef struct 
{
	unsigned char BaseConfig[BASECONFIGSIZE];
	unsigned char RouterTable[ROUTERTABLESIZE];
	unsigned char RegTable[REGTABLESIZE];	//先分配6K
	unsigned char DiDi_DianTable[DIDIDIANTABLESIZE];//前512字节存放索引表，其中前2字节代表DiAn的总数
	unsigned char reserve[3*1024];
	
	PARAMS_TOBOOTLOADER params_tobootloaderconfig;
	
} EEPROMDATA, _PTR_  EEPROMDATA_PTR;


//基本配置信息的结构分配
typedef struct
{
	unsigned char head[4];
	unsigned char hardversion[4];
	unsigned char softversion[4];
	unsigned char uid[4];			//12
	unsigned char dhcp;				//16
	unsigned char enabledns;		//17
	unsigned char enablesntp;		//18	
	unsigned char dump[1];			//19
	unsigned char ip[4];			//20
	unsigned char mask[4];			//24
	unsigned char gateway[4];		//28
	unsigned char dns[4];			//32
	char sntp1[40];					//36
	char sntp2[40];					//76
	char sntp3[40];					//116
	char sntp4[40];					//156
	char datacenterdns[40];			//196
	unsigned char datacenter[4];	//236
	unsigned char ipport[4];		//240
	unsigned char datacentermux[4];	//244
	unsigned char ipportmux[4];
	unsigned char net_addr;
	unsigned char subnet_num;
	unsigned char subdev_num;
	unsigned char work_mode;
	unsigned char rffreq[4];
	unsigned char rfpower;
	unsigned char reserve[BASECONFIGSIZE-1-1-1-1-4-4-40-40-40-40-40-4-4-4-4-1-1-1-1-4-4-4-4-5];
	
} BASECONFIGTABLE, _PTR_ BASECONFIGTABLE_PTR;

typedef struct
{
	unsigned char dianaddr[2];
	unsigned char offset[2];//偏移量，高字节在前offset[0]，低字节在后offset[1]，下同
	unsigned char count[2];//所带DiDi的个数
}DIANDIDIINDEXTAB, _PTR_ DIANDIDIINDEXTAB_PTR;


typedef struct
{
	unsigned char diancount[2];		//DiAn的个数，高字节在前diancount[0]，低字节在后diancount[1]
	DIANDIDIINDEXTAB indextab[(DIDIDIANINDEXTABSIZE-2)/6]; //索引表
	unsigned char didiaddr[DIDIDIANTABLESIZE-DIDIDIANINDEXTABSIZE];//DiDi的地址数据
}DIANDIDITABLE, _PTR_ DIANDIDITABLE_PTR;


extern BASECONFIGTABLE_PTR pBaseConfig;
extern PARAMS_TOBOOTLOADER_PTR pParams_toBootLoader;
extern uchar workmode;

void EepromInit(void);
void EepromWrite(uchar_ptr write_buffer, _mem_size seek_location, _mem_size read_write_size);
void EepromRead(uchar_ptr read_buffer, _mem_size seek_location, _mem_size read_write_size);
void eepromreadtest(void);
void ChkEepromFirstUse(void);
void EepromWriteCoodaddr(unsigned char *);

#endif
/* EOF */
