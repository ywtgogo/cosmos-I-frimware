/*************************************************************
* 
*   Copyright (c) 2010 convertergy;
*   All Rights Reserved
*
*   FileName: dicor_sd.c
*   Version : 
*   Date    :
*
*   Comments:
*
* 
*
*****************************************************************/
	
#include <mqx.h>
#include <bsp.h>
#include <fio.h>
#include <mfs.h>
#include <sdcard.h>
#include <sdcard_spi.h>
#include <Watchdog.h>
#include <spi.h>
#include <part_mgr.h>
#include "led.h"
#include "dicor_upload.h"
#include "dicor_sd.h"




char filesystem_name[] = "d:";
MQX_FILE_PTR filesystem_handle;
LWSEM_STRUCT	spi_sem;
static MCF5225_GPIO_STRUCT_PTR sdpwr_gpio_ptr;

extern EMN_DICOR_DATA_ST DiCorRunStatus;
extern void Dicor_Reboot(void);


/*
** parameters of sdcard0 initialization
*/

const SDCARD_INIT_STRUCT _bsp_sdcard0_init = { 
	_io_sdcard_spi_init,
	_io_sdcard_spi_read_block,
	_io_sdcard_spi_write_block,
	BSP_SDCARD_SPI_CS
};


static void Sdcard_Test(void);



static void SdPwrInit(void)
{
	sdpwr_gpio_ptr = (MCF5225_GPIO_STRUCT_PTR) _bsp_get_gpio_base_address();

	sdpwr_gpio_ptr->PANPAR &= 0xFB;	//AN2
	sdpwr_gpio_ptr->DDRAN |= 0x04;
}

static void SdReset(void)
{

	SD_PWR_OFF();
	_time_delay(1000);
	SD_PWR_ON(); 

}

	
/*----------------------------------------------------------
 *
 * Function Name  : sdcard_task
 * Returned Value : void
 * Comments		 :
 *
 *-----------------------------------------------------------*/
void SDcard_Install(void)
{ 
// boolean inserted = TRUE, readonly = FALSE, last = FALSE;
 _mqx_int error_code;
 //_mqx_uint param;
 MQX_FILE_PTR com_handle, sdcard_handle;
// MQX_FILE_PTR partman_handle;
 //char filesystem_name[] = "dicor:";

 //char partman_name[] = "pm:";
	char temp[15];
 char_ptr                   volume_ptr;
 _mqx_uint   result;
 
 SdPwrInit();
 SdReset();
	
		/* Create the lightweight semaphores */
   result = _lwsem_create(&spi_sem, 1);
   if (result != MQX_OK) {
      printf("\nCreating spi_sem failed: 0x%X", result);
      //_mqx_exit(0);
    //  Dicor_Reboot();
   }
   SetSpiSem(&spi_sem);
	
	volume_ptr = temp;
   /* Open low level communication device */
 com_handle = fopen (BSP_SDCARD_SPI_CHANNEL, NULL);
 if (NULL == com_handle) 
 {
    printf("Error installing communication handle.\n");
	//_task_block();
	Dicor_Reboot();
 }
	
/* Install SD card device */
  error_code = _io_sdcard_install("sdcard:", (pointer)&_bsp_sdcard0_init, 
                                  com_handle);
  if ( error_code != MQX_OK ) 
  {
	 printf("Error installing SD card device (0x%x)\n", error_code);
	 //_task_block();
	 Dicor_Reboot();
  }
	
  _time_delay (200);
	
  /* Open the device which MFS will be installed on */
  sdcard_handle = fopen("sdcard:", 0);//The first time fopen() is called on the device driver, it opens the device driver.
  if ( sdcard_handle == NULL ) 
  {
	 printf("Unable to open SD card device.\n");
	 SdLedOff();
	 DiCorRunStatus.sd_status = 1;
	 //_task_block();
	 
	 SdReset();
	 //可以加判断重启N次后不再重启
	 Dicor_Reboot();
	 
	 return;
  }
#if 0
	/* Set read only flag as needed */
	param = 0;
	if (readonly) 
    {
		param = IO_O_RDONLY;
	}
	if (IO_OK != ioctl(sdcard_handle, IO_IOCTL_SET_FLAGS, (char_ptr) &param))
	{
		printf("Setting device read only failed.\n");
		_task_block();
	}
#endif
//#if SD_PART_MGR
#if 0

					/* Install partition manager over SD card driver */
					error_code = _io_part_mgr_install(sdcard_handle, partman_name, 0);
					if (error_code != MFS_NO_ERROR) 
					{
						printf("Error installing partition manager: %s\n", MFS_Error_text((uint_32)error_code));
						_task_block();
					} 
	
					/* Open partition manager */
					partman_handle = fopen(partman_name, NULL);
					if (partman_handle == NULL) 
					{
						error_code = ferror(partman_handle);
						printf("Error opening partition manager: %s\n", MFS_Error_text((uint_32)error_code));
						_task_block();
					} 
	
					/* Validate partition 1 */
					param = 1;
					error_code = _io_ioctl(partman_handle, IO_IOCTL_VAL_PART, &param);
					if (error_code == MQX_OK) 
					{
					
						/* Install MFS over partition 1 */
						error_code = _io_mfs_install(partman_handle, filesystem_name, param);
						if (error_code != MFS_NO_ERROR) 
						{
							printf("Error initializing MFS over partition: %s\n", MFS_Error_text((uint_32)error_code));
							_task_block();
						} 
						
					} else {
					
						/* Install MFS over SD card driver */
						error_code = _io_mfs_install(sdcard_handle, filesystem_name, (_file_size)0);
						if (error_code != MFS_NO_ERROR) 
						{
							printf("Error initializing MFS: %s\n", MFS_Error_text((uint_32)error_code));
							_task_block();
						}
					   
					}
#else
	//使用外部扩展的SRAM
	_MFS_pool_id = _mem_create_pool((pointer)DICORCFG_MFS_POOL_ADDR, DICORCFG_MFS_POOL_SIZE);		
				
	/* Install MFS over SD card driver */
	error_code = _io_mfs_install(sdcard_handle, filesystem_name, (_file_size)0);
	if (error_code != MFS_NO_ERROR) 
	{
		printf("Error initializing MFS: %s\n", MFS_Error_text((uint_32)error_code));
		//_task_block();
		Dicor_Reboot();
	}
#endif						
	/* Open file system and detect format */
	filesystem_handle = fopen(filesystem_name, NULL);
	error_code = ferror (filesystem_handle);
	if ((error_code != MFS_NO_ERROR) && (error_code != MFS_NOT_A_DOS_DISK))
	{
		printf("Error opening filesystem: %s\n", MFS_Error_text((uint_32)error_code));
		//_task_block();
		Dicor_Reboot();
	}
	if ( error_code == MFS_NOT_A_DOS_DISK ) 
	{
		printf("NOT A DOS DISK! You must format to continue.\n");

		_watchdog_stop();
		printf("\nFormating...\n");
		DiCorRunStatus.sd_status = 2;	//SD卡正在格式
		#if 0
		error_code = ioctl(filesystem_handle, IO_IOCTL_DEFAULT_FORMAT,  NULL);
		if ( !error_code && volume_ptr) {
		   error_code = ioctl(filesystem_handle, IO_IOCTL_SET_VOLUME,	(pointer) volume_ptr);
		}		
		if (error_code) {
		   printf("Error while formatting: 0x%x\n", error_code);
		} else	{
		/* print disk information */
		   error_code = ioctl(filesystem_handle, IO_IOCTL_GET_VOLUME, (uint_32_ptr) volume_ptr);
		   printf("Done. Volume name is %s\nfree disk space: %lu bytes\n",
				volume_ptr,
				ioctl(filesystem_handle, IO_IOCTL_FREE_SPACE, NULL) );
		} 
		#endif

		
		error_code = ioctl(filesystem_handle, IO_IOCTL_DEFAULT_FORMAT,  NULL);
		if (!error_code) {
	
		   printf("Done. free disk space: %lu bytes\n",
				ioctl(filesystem_handle, IO_IOCTL_FREE_SPACE, NULL) );
		} 
		_watchdog_start(60*2000);
	}
	
	error_code = ioctl(filesystem_handle, IO_IOCTL_CREATE_SUBDIR, (uint_32_ptr) "\\TempData");
	error_code = ioctl(filesystem_handle, IO_IOCTL_CREATE_SUBDIR, (uint_32_ptr) "\\dicorconfig");
	error_code = ioctl(filesystem_handle, IO_IOCTL_CREATE_SUBDIR, (uint_32_ptr) "\\didiconfig");
	error_code = ioctl(filesystem_handle, IO_IOCTL_CREATE_SUBDIR, (uint_32_ptr) "\\dianconfig");
	error_code = ioctl(filesystem_handle, IO_IOCTL_CREATE_SUBDIR, (uint_32_ptr) "\\dicorlogs");
	error_code = ioctl(filesystem_handle, IO_IOCTL_CREATE_SUBDIR, (uint_32_ptr) "\\didiprogram");


	
	printf ("\nSD card installed to %s\n", filesystem_name);   
    //Sdcard_Test();
	SdLedOn();
	DiCorRunStatus.sd_status = 0;
}

static void Sdcard_Test(void)
{
	//create one file 
	FILE_PTR		  fd_ptr;	
	char buffer[20] = "123456789abcd\r\n";
 //   char buffer2[20];
//	 _mqx_int error;
	int_32 location;
	 
	fd_ptr = fopen("d:123.txt", "a+");
	if( fd_ptr == NULL )
	{
	   asm(nop);   
	}
	
	location = ftell(fd_ptr);
	printf("\r\nlocation=%d\r\n", location);
	fwrite(buffer, 8, 1, fd_ptr);

	location = ftell(fd_ptr);
	printf("\r\nlocation=%d\r\n", location);
	
	fwrite(buffer, 8, 1, fd_ptr);

	location = ftell(fd_ptr);
	printf("\r\nlocation=%d\r\n", location);
	
	fwrite(buffer, 8, 1, fd_ptr);
	location = ftell(fd_ptr);
	printf("\r\nlocation=%d\r\n", location);

	fseek(fd_ptr, 0, IO_SEEK_SET);
	location = ftell(fd_ptr);
	printf("\r\nlocation=%d\r\n", location);
	
	fread(buffer, 8, 1, fd_ptr);
	location = ftell(fd_ptr);
	printf("\r\nlocation=%d\r\n", location);
	fread(buffer, 8, 1, fd_ptr);
	location = ftell(fd_ptr);
	printf("\r\nlocation=%d\r\n", location);
	fread(buffer, 8, 1, fd_ptr);
	location = ftell(fd_ptr);
	printf("\r\nlocation=%d\r\n", location);

	
	
	
	/*write(fd_ptr, buffer, 15);
	fclose(fd_ptr);

	
    fd_ptr = fopen("dicor:123.txt", "r");             
    read(fd_ptr, buffer2, 15);
    fclose(fd_ptr); 


	fd_ptr = fopen("dicor:\\TempData\\test.txt", "a+");

	if( fd_ptr == NULL )
	{
	   asm(nop);   
	}
	write(fd_ptr, buffer, 15);
	fclose(fd_ptr);*/


	printf("\r\n*********SD Card OK~!************\r\n");

/*	error = ioctl(fd_ptr, IO_IOCTL_DELETE_FILE,"dicor:123.txt");
    if (error)  
	{
       printf("Error deleting file\n");
    }*/

}




