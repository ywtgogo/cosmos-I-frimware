#ifndef __upgradedidi_h__
#define __upgradedidi_h__

//DiDi自动升级第一版//
#define		DIDIPROGRAMABLESIZE      10*2048	//10k words
#define		PROGRAMENABEEND			 0x7c00
#define		PROGRAMENABEBEGIN		 (0x7c00-DIDIPROGRAMABLESIZE/2)

#define     SENDOENBUFSIZE			112 	//236	//一次发送114

extern int_32  Shell_Downloaddidiprogram(int_32 argc, char_ptr argv[]);
extern int_32  Shell_Setdidiprogramfilename(int_32 argc, char_ptr argv[]);
extern int_32  Shell_Getdidiprogramfilename(int_32 argc, char_ptr argv[]);
extern int_32  Shell_UpgradeDiDi(int_32 argc, char_ptr argv[]);
extern int_32  Shell_StopGetData(int_32 argc, char_ptr argv[]);
extern void StopGetData(void);
extern void StartGetData(void);
extern _mem_size base_addr;
//extern UINT_8 GetSubDateFlag;
extern LWSEM_STRUCT GetSubDateSem;
extern LWSEM_STRUCT NetworkRfSem;
int_32  Shell_UpgradeDiDi8245(int_32 argc, char_ptr argv[],uint_8* data_buffer);
extern int_32 Boot_AnalysisSRecord_8245(char *filename, uint_8* rambuf,uint_8* data_buffer);
extern int_32 Boot_AnalysisHEX(char *filename, uint_8* rambuf, uint_8* data_buffer);
extern uint_8 Boot_UpgradeSubDiDi_8245(uint_16 addr_f, uint_8* rambuf, uint_16 addr_r);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//DiDi自动升级第二版 Bootloader模式//
//56F8255

#define		BOOT_PROGRAMENABEEND	   	0x7C00
#define     BOOT_CC2530_END             0x8000
#define     BOOT_CC2530_BEGIN           0x1000
#define		BOOT_PROGRAMENABEBEGIN		0x0C00

#define		BOOT_DIDIPROGRAMABLESIZE    ((BOOT_PROGRAMENABEEND-BOOT_PROGRAMENABEBEGIN)*2) // (0x7c00-BOOT_DIDIPROGRAMABLESIZE/2)20*2048	//20k words

#define     BOOT_SENDOENBUFSIZE			VALIDDATALEN 	

#define PACKAGEHEAD1	0x5A
#define PACKAGEHEAD2	0xA5


// Flash Parameters and Functions
#define FLASH_PAGE_SIZE     1024    // Flash Page Size (in words)
#define FLASH_LO            BOOT_PROGRAMENABEBEGIN    // Flash Boundary (Low)
//#define FLASH_HI        	0x7FFF  // Flash Boundary (High) for applications 
									// since the bootloader occupies the range from 0x7000 to 0x7FFF
#define FLASH_LAST_PAGE		BOOT_PROGRAMENABEEND	// Last page address
#define VALIDDATALEN		124   //28  by  younger
#define  VALIDDATALEN2		80




typedef enum 
{
	ERR_OKAY = 0,
	//没有收到信息头
 	ERR_HEADNOTFIND,
 	//地址超出范围或不允许
 	ERR_ADDRNOTALLOW,
 	//擦除FLASH的时候出错，被写保护
 	ERR_FLASHERASE,
 	//写FLASH的时候出错
 	ERR_FLASHWRITE
}ERROR_STATE;
//int_8 len;
typedef struct
{
	int_8    function;
	uint_16  addr_f;
    uint_16  addr_r;
    uint_16  start_addr;
  	uint_8   data[VALIDDATALEN];
}Package;


typedef struct
{   uint_16  cos_addr;
    uint_8   function;	
	uint_16  addr_f;
	
    uint_16  addr_r;
    uint_16  start_addr;
    uint_8   len;
  	uint_8   data[VALIDDATALEN2];
}Package2;


typedef struct
{
	uint_16  addr_f;
	uint_8   funcode;	//数据正确做头标志，错误做错误返回码
    uint_16  addr_r;
    uint_16  start_addr;
  	uint_8 data[VALIDDATALEN];
}Package1;
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
//magnus-IAP
#define    BOOT_DIDIPROGRAMABLESIZE_MAGNUS      ((BOOT_CC2530_END-BOOT_CC2530_BEGIN)*2)



//为56F8245添加的几个宏
#define		BOOT_PROGRAMENABEBEGIN_FOR8245	 0x2000
#define		BOOT_DIDIPROGRAMABLESIZE_FOR8245    ((BOOT_PROGRAMENABEEND-BOOT_PROGRAMENABEBEGIN_FOR8245)*2) 
#define    FLASH_LO_FOR8245            BOOT_PROGRAMENABEBEGIN_FOR8245    // Flash Boundary (Low)


int_32  Shell_DetectDevice430(int_32 argc, char_ptr argv[]);
int_32  Shell_DetectDevice(int_32 argc, char_ptr argv[]);
int_32  Shell_DetectDevUpgrade(int_32 argc, char_ptr argv[]);
int_32  Shell_DetectDevUpgrade8245(int_32 argc, char_ptr argv[],uint_8* data_buffer);
int_32  Shell_PrintDiDiEeprom(int_32 argc, char_ptr argv[]);
int_32  Shell_SetDiDiEeprom(int_32 argc, char_ptr argv[]);
int_32  Shell_TestRF(int_32 argc, char_ptr argv[]);

#endif