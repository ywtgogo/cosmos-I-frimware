/*************************************************************
* 
* Copyright (c) convertergy ;
* All Rights Reserved
* FileName: breakpoint.c
* Version : 
* Date    : 
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
#include "breakpoint.h"
#include "eeprom.h"



SAVE_DATA_BUFFER  Save_Data_Buff;
BREAK_POINT_INFO BreakPointInfo;

static UINT_8 SaveFileCounter(void);
static void Number2FileName(char* str, UINT_16 num, UINT_8 strlen);
static UINT_8 ChkFileExist(void);

extern void dicor_get_time(DATE_STRUCT*  date);


/*****************************************************************************
 *  FunctionName 	: SaveDate2SdCard                                                                            *
 *  Function      	: UINT_8 SaveData2SdCard(DICOR_PACKET1* data0)                                                                                                    *
 *  Parameters    	: DICOR_PACKET1* data0                                                                                                    *
 *  Return        	: 成功返回0，失败返回1                                                                                                    *
 *  Global Vars   	: BREAK_POINT_INFO BreakPointInfo                                                                                                    *
 *  Author/Date   	: Dick.Yang/2011-07-05					                                    *
 *  ModifiedRecord	:                                                                                                     *
 *****************************************************************************/
UINT_8 SaveData2SdCard(DICOR_PACKET1* data0)
{
	int_32 location, bak_location;
	DATE_STRUCT date;
	uint_64 temp = 0xAAAAAAAAAAAAAAAA;
	char s[40];
	UINT_8 bak = 0;
	//dicor_get_rtctime(&date);
	dicor_get_time(&date);
	//_lwsem_wait(&spi_sem);
	
	//如果读写为同一个文件，备份读指针，待会恢复
	if (BreakPointInfo.fd_read_ptr == BreakPointInfo.fd_write_ptr)
	{
		bak = 1;
		bak_location = ftell(BreakPointInfo.fd_read_ptr);
		fseek(BreakPointInfo.fd_read_ptr, 0, IO_SEEK_END);	
	}

	location = ftell(BreakPointInfo.fd_write_ptr);
	//printf("\r\nlocation=%d\r\n", location);
	if (location > FILEMAXSIZE)
	{
		if (BreakPointInfo.fd_read_ptr != BreakPointInfo.fd_write_ptr)
		{
			fclose(BreakPointInfo.fd_write_ptr);
		}
		BreakPointInfo.writeindex++;
		//重开一个文件
		Number2FileName(s, BreakPointInfo.writeindex, 40);
		BreakPointInfo.fd_write_ptr = fopen(s, "a+");
		if (BreakPointInfo.fd_write_ptr == NULL )
		{
			printf("Error open file\r\n");
			return 1;
		}
		printf("\r\ncreate a new file!\r\n");
		//保存下当前文件名计数
		SaveFileCounter();
	}
	
	fwrite(&temp, 8, 1, BreakPointInfo.fd_write_ptr);	//8个AA开头
	fwrite(&date, 12, 1, BreakPointInfo.fd_write_ptr);
	fwrite((int_8_ptr)(&(data0->num)), (uint_32)(data0->num+2), 1, BreakPointInfo.fd_write_ptr);

	//如果不考虑掉电情况，可将这段代码删除提高操作速度
#if 1
	//关了再开防止掉电不保存
	fclose(BreakPointInfo.fd_write_ptr);
	Number2FileName(s, BreakPointInfo.writeindex, 40);
	BreakPointInfo.fd_write_ptr = fopen(s, "a+");
	if (BreakPointInfo.fd_write_ptr == NULL )
	{
		printf("Error open file\r\n");
		return 1;
	}
	if (BreakPointInfo.readindex == BreakPointInfo.writeindex)
	{
		BreakPointInfo.fd_read_ptr = BreakPointInfo.fd_write_ptr;
	}
#endif

	
	//文件正在读的情况写完再恢复保证读是正确的
	if (bak)
	{
		fseek(BreakPointInfo.fd_read_ptr, bak_location, IO_SEEK_SET);
	}
	//_lwsem_post(&spi_sem);
	return 0;
}

/*****************************************************************************
 *  FunctionName 	: BreakpointContinuingly                                                                    *
 *  Function      	: INT_8 BreakpointContinuingly(SAVE_DATA_BUFFER* save_data_buff, uint_32* offset)                                                                                                    *
 *  Parameters    	:                                                                                                     *
 *  Return        	:                                                                                                     *
 *  Global Vars   	:                                                                                                     *
 *  Author/Date   	: Dick.Yang/2011-07-06					                                    *
 *  ModifiedRecord	:                                                                                                     *
 *****************************************************************************/
INT_8 BreakpointContinuingly(SAVE_DATA_BUFFER* save_data_buff, uint_32* offset)
{
	uint_32 temp, j;
	_mqx_int error;
	uint_64 val = 0;
	char s[40];
	
	save_data_buff->type = 0x14;
	save_data_buff->dicor_id[0] = pBaseConfig->uid[0];
    save_data_buff->dicor_id[1] = pBaseConfig->uid[1];
	save_data_buff->dicor_id[2] = pBaseConfig->uid[2];
	save_data_buff->dicor_id[3] = pBaseConfig->uid[3];
	save_data_buff->dumy[0] = 0;
	temp = *offset;
	//_lwsem_wait(&spi_sem);

	fread((int_8_ptr)(&val), 8, 1, BreakPointInfo.fd_read_ptr);

	if (val != 0xAAAAAAAAAAAAAAAA)
	{
		//找到数据头部
		j = temp+1;
		while (!feof(BreakPointInfo.fd_read_ptr))
		{
			val = 0;
			fseek(BreakPointInfo.fd_read_ptr, j, IO_SEEK_SET);
			fread((int_8_ptr)(&val), 8, 1, BreakPointInfo.fd_read_ptr);
			if (val == 0xAAAAAAAAAAAAAAAA)
			{
				//printf("\r\nfind head!\r\n");
				temp = j + 8;
				break;
			}
			j++;
		}
	}
	
	fread((int_8_ptr)(&(save_data_buff->save_time)), 12+2, 1, BreakPointInfo.fd_read_ptr);
	temp += 14;
	fread((int_8_ptr)(&(save_data_buff->didi_data)), (uint_32)(save_data_buff->num), 1, BreakPointInfo.fd_read_ptr);
	temp += (uint_32)(save_data_buff->num);
	
	*offset = temp;

#if 0
	{
		int i;
		printf("READ DATA\r\n%04d/%02d/%02d %02d:%02d:%02d\r\n", save_data_buff->save_time.YEAR, save_data_buff->save_time.MONTH, save_data_buff->save_time.DAY, \
	  		save_data_buff->save_time.HOUR, save_data_buff->save_time.MINUTE, save_data_buff->save_time.SECOND);
		printf("%04X\r\n", save_data_buff->num); 
		for (i=0; i<save_data_buff->num; i++)
		{
			if (i % 10 == 0)
			{
				printf("\r\n");
			}
			printf("%02X ", save_data_buff->didi_data[i]); 
			
		}
		printf("*****************************\r\n");
	}
#endif		
	
	if (feof(BreakPointInfo.fd_read_ptr))
	{
		fclose(BreakPointInfo.fd_read_ptr);
		//数据上传结束后将文件删除
		Number2FileName(s, BreakPointInfo.readindex, 40);
		error = ioctl(BreakPointInfo.fd_read_ptr, IO_IOCTL_DELETE_FILE, s);
	    if (error)  
		{
	       printf("\r\nError deleting file!\r\n");
		   //_lwsem_post(&spi_sem);
		   return -1;
	    }
		if (BreakPointInfo.readindex == BreakPointInfo.writeindex)
		{
			BreakPointInfo.fd_read_ptr = NULL;
			BreakPointInfo.fd_write_ptr = NULL;
			BreakPointInfo.readindex= 0;
			BreakPointInfo.writeindex = 0;
			SaveFileCounter();
			//_lwsem_post(&spi_sem);
			return 1;
		}
		else
		{
			BreakPointInfo.readindex++;
			SaveFileCounter();
			if (BreakPointInfo.readindex != BreakPointInfo.writeindex)
			{
				//打开下一个待传文件
				Number2FileName(s, BreakPointInfo.readindex, 40);
				BreakPointInfo.fd_read_ptr = fopen(s, "a+");
				if (BreakPointInfo.fd_read_ptr == NULL )
				{
	   				printf("Error open file\r\n"); 
				}
				
			}
			else
			{
				BreakPointInfo.fd_read_ptr = BreakPointInfo.fd_write_ptr;
			}
			fseek(BreakPointInfo.fd_read_ptr, 0, IO_SEEK_SET);
			*offset = 0;
		}
	}
	//_lwsem_post(&spi_sem);
	return 0;
}





/*****************************************************************************
 *  FunctionName 	: ChkFileExist                                                                                   *
 *  Function      	:                                                                                                     *
 *  Parameters    	:                                                                                                     *
 *  Return        	:                                                                                                     *
 *  Global Vars   	:                                                                                                     *
 *  Author/Date   	: Dick.Yang/2011-07-07					                                    *
 *  ModifiedRecord	:                                                                                                     *
 *****************************************************************************/
static UINT_8 ChkFileExist(void)
{
	FILE_PTR fd_ptr;
	if (BreakPointInfo.readindex != 0)
	{
		BreakPointInfo.Fileexist = 1;
		return 1;
	}
	else
	{
	//	_lwsem_wait(&spi_sem);
		fd_ptr = fopen("d:\\TempData\\TempData0000", "r");
		if (fd_ptr != NULL)
		{
			fclose(fd_ptr);
	//		_lwsem_post(&spi_sem);
			BreakPointInfo.Fileexist = 1;
			return 1;
		}
		else
		{
//			_lwsem_post(&spi_sem);
			BreakPointInfo.Fileexist = 0;
			return 0;
		}
	}
}

/*****************************************************************************
 *  FunctionName 	: Number2FileName                                                                                   *
 *  Function      	:                                                                                                     *
 *  Parameters    	:                                                                                                     *
 *  Return        	:                                                                                                     *
 *  Global Vars   	:                                                                                                     *
 *  Author/Date   	: Dick.Yang/2011-07-14					                                    *
 *  ModifiedRecord	:                                                                                                     *
 *****************************************************************************/
static void Number2FileName(char* str, UINT_16 num, UINT_8 strlen)
{
	UINT_8 i;
	for (i=0; i<strlen; i++)
	{
		str[i] = '\0';
	}
	sprintf(str, "d:\\TempData\\TempData%04d", num);	
}
	
/*****************************************************************************
 *  FunctionName 	: BreakpointInit                                                                                   *
 *  Function      	:                                                                                                     *
 *  Parameters    	:                                                                                                     *
 *  Return        	:                                                                                                     *
 *  Global Vars   	:                                                                                                     *
 *  Author/Date   	: Dick.Yang/2011-07-14					                                    *
 *  ModifiedRecord	:                                                                                                     *
 *****************************************************************************/
uint_8 BreakpointInit(void)
{
	FILE_PTR fd_ptr;
	char s[40];

	fd_ptr = fopen("d:\\TempData\\FileCounter", "r");
	if (fd_ptr == NULL)
	{
		//第一次使用创建文件
		fd_ptr = fopen("d:\\TempData\\FileCounter", "a+");
		if (fd_ptr == NULL )
		{
			printf("Error open file\r\n");
			return 1;
		}
		BreakPointInfo.readindex = 0;
		BreakPointInfo.writeindex = 0;
		fwrite(&BreakPointInfo.readindex, 2, 1, fd_ptr);
		fwrite(&BreakPointInfo.writeindex, 2, 1, fd_ptr);
		fclose(fd_ptr);		
	}
	else
	{
		//将读写文件名序号从SD卡中读出来
		fseek(fd_ptr, 0, IO_SEEK_SET);
		fread(&BreakPointInfo.readindex, 2, 1, fd_ptr);
		fread(&BreakPointInfo.writeindex, 2, 1, fd_ptr);
		fclose(fd_ptr);
	}

	ChkFileExist();

	Number2FileName(s, BreakPointInfo.readindex, 40);
	//读写为同一个文件的情况
	if (BreakPointInfo.readindex == BreakPointInfo.writeindex)
	{
		if (BreakPointInfo.Fileexist)
		{
			BreakPointInfo.fd_read_ptr = fopen(s, "a+");
			if (BreakPointInfo.fd_read_ptr == NULL )
			{
				printf("Error open file\r\n");
				return 1;
			}
			BreakPointInfo.fd_write_ptr = BreakPointInfo.fd_read_ptr;
			//移到文件头部，为读做准备，没考虑断电情况
			fseek(BreakPointInfo.fd_read_ptr, 0, IO_SEEK_SET);
		}
		else
		{
			BreakPointInfo.fd_read_ptr = NULL;
			BreakPointInfo.fd_write_ptr = NULL;
		}
	}
	else
	{
		BreakPointInfo.fd_read_ptr = fopen(s, "a+");
		if (BreakPointInfo.fd_read_ptr == NULL )
		{
			printf("Error open file\r\n");
			return 1;
		}
		//移到文件头部，为读做准备，没考虑断电情况
		fseek(BreakPointInfo.fd_read_ptr, 0, IO_SEEK_SET);
		Number2FileName(s, BreakPointInfo.writeindex, 40);
		BreakPointInfo.fd_write_ptr = fopen(s, "a+");
		if (BreakPointInfo.fd_write_ptr == NULL )
		{
			printf("Error open file\r\n");
			return 1;
		}
	}
	
	return 0;
}

/*****************************************************************************
 *  FunctionName 	: SaveFileCounter                                                                                   *
 *  Function      	:                                                                                                     *
 *  Parameters    	:                                                                                                     *
 *  Return        	:                                                                                                     *
 *  Global Vars   	:                                                                                                     *
 *  Author/Date   	: Dick.Yang/2011-07-14					                                    *
 *  ModifiedRecord	:                                                                                                     *
 *****************************************************************************/
static UINT_8 SaveFileCounter(void)
{
	FILE_PTR fd_ptr;
	fd_ptr = fopen("d:\\TempData\\FileCounter", "w+");
	if (fd_ptr == NULL )
	{
		printf("Error open file\r\n");
		return 1;
	}
	fwrite(&BreakPointInfo.readindex, 2, 1, fd_ptr);
	fwrite(&BreakPointInfo.writeindex, 2, 1, fd_ptr);
	fclose(fd_ptr);	
	printf("\r\nSave FileCounter Succeed!\r\n");
}





void WriteTest(void)
{
	int i;
	int offset = 0;
	DICOR_PACKET1 data0;
	//SAVE_DATA_BUFFER save_data_buff;
	DATE_STRUCT  date;
	FILE_PTR fd_ptr;
	char s[40];
	uint_32 error;
	UINT_8 didi[] = {0x10,0x16,0x00,0x01,0x29,0x08,0x29,0x08,0x00,0x80,0x00,0x16,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		             0x10,0x16,0x00,0x02,0x29,0x08,0x29,0x08,0x00,0x80,0x00,0x16,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x10,0x16,0x00,0x03,0x29,0x08,0x29,0x08,0x00,0x80,0x00,0x16,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x10,0x16,0x00,0x04,0x29,0x08,0x29,0x08,0x00,0x80,0x00,0x16,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x10,0x16,0x00,0x05,0x29,0x08,0x29,0x08,0x00,0x80,0x00,0x16,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x10,0x16,0x00,0x06,0x29,0x08,0x29,0x08,0x00,0x80,0x00,0x16,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x10,0x16,0x00,0x07,0x29,0x08,0x29,0x08,0x00,0x80,0x00,0x16,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x10,0x16,0x00,0x08,0x29,0x08,0x29,0x08,0x00,0x80,0x00,0x16,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x10,0x16,0x00,0x09,0x29,0x08,0x29,0x08,0x00,0x80,0x00,0x16,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x10,0x16,0x00,0x0a,0x29,0x08,0x29,0x08,0x00,0x80,0x00,0x16,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x10,0x16,0x00,0x0b,0x29,0x08,0x29,0x08,0x00,0x80,0x00,0x16,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x10,0x16,0x00,0x0c,0x29,0x08,0x29,0x08,0x00,0x80,0x00,0x16,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x10,0x16,0x00,0x0d,0x29,0x08,0x29,0x08,0x00,0x80,0x00,0x16,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x10,0x16,0x00,0x0e,0x29,0x08,0x29,0x08,0x00,0x80,0x00,0x16,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x10,0x16,0x00,0x0f,0x29,0x08,0x29,0x08,0x00,0x80,0x00,0x16,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x10,0x16,0x00,0x10,0x29,0x08,0x29,0x08,0x00,0x80,0x00,0x16,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		}; 
    data0.type = 0x14;//0xaa register map table, 0x11:sample data frame
    data0.dicor_id[0] = 0x2E;
	data0.dicor_id[1] = 0x00;
	data0.dicor_id[2] = 0x29;
	data0.dicor_id[3] = 0x7C;
	data0.dumy[0] = 0x00;
	data0.num = 0x0160; 	  
    for (i=0; i< data0.num; i++)
	{
		data0.didi_data[i] = didi[i];
		//didi[i]= RS485_Data_Buffer[512];
    }

  	error = send(upload_buffer.sock, (int_8_ptr)(&Save_Data_Buff.type), (uint_32)(Save_Data_Buff.num + DICOR_ID_LEN + 4 + 12), 0);
	BreakPointInfo.writeindex = 0;
	BreakPointInfo.readindex = 0;
	BreakPointInfo.fd_read_ptr = NULL;


	
		fd_ptr = fopen("d:\\TempData\\FileCounter", "a+");
		if (fd_ptr == NULL )
		{
			printf("Error open file\r\n");
			return;
		}
		BreakPointInfo.readindex = 0;
		BreakPointInfo.writeindex = 0;
		fwrite(&BreakPointInfo.readindex, 2, 1, fd_ptr);
		fwrite(&BreakPointInfo.writeindex, 2, 1, fd_ptr);
		fclose(fd_ptr);		
	
	

		Number2FileName(s, BreakPointInfo.writeindex, 40);
		BreakPointInfo.fd_write_ptr = fopen(s, "a+");
		if (BreakPointInfo.fd_write_ptr == NULL )
		{
			printf("Error open file\r\n");
			return;
		}
		
	#if 0
	for (i=0; i<10000; i++)
	{
		SaveData2SdCard(&data0);
		if (i % 100 == 0)
			{
			printf("\r\ni=%d\r\n", i);
			//dicor_get_rtctime(&date);
			dicor_get_time(&date);
	printf("\r\n\store time: %04d/%02d/%02d %02d:%02d:%02d\r\n",
      date.YEAR,date.MONTH,date.DAY,date.HOUR,
      date.MINUTE,date.SECOND);
			}
	}
	SaveFileCounter();
	//BreakpointContinuingly(&save_data_buff, &offset);
    #endif
}




/* EOF */
