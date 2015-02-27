/*************************************************************
* 
* Copyright (c) convertergy ;
* All Rights Reserved
* FileName: My_Shell_Commands.c
* Version : 
* Date    : 2011/08/15
* ower: Alex
*
* Comments:*一些自定义的shell命令，方便控制操作
*			2011/09/08增加配置EEPROM中数据四条命令
*
***************************************************************/


#include <string.h>
#include <mqx.h>
#include <bsp.h>
#include <shell.h>
#include <rtcs.h>
#include "sh_rtcs.h"

#include "My_Shell_Commands.h"
#include "eeprom.h"
#include "utility.h"
#include <stdlib.h>


#include "network_nwk.h"
#include "dicor_upload.h"


extern BASECONFIGTABLE_PTR pBaseConfig;
extern UINT_8 SERVER_MODE;

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   
* Returned Value   :  int_32 error code
* Comments  :   
*
*END*---------------------------------------------------------------------*/

int_32 Shell_setuid(int_32 argc, char_ptr argv[])
{
	boolean print_usage, shorthelp = FALSE;
	int_32 return_code = SHELL_EXIT_SUCCESS;

	char hex[3];
	unsigned char writebuf[WRITEBUFSIZE];
	BASECONFIGTABLE_PTR baseconfig;

	baseconfig = (BASECONFIGTABLE_PTR) writebuf;


	hex[2] = '\0';

	print_usage = Shell_check_help_request(argc, argv, &shorthelp);

	if (!print_usage) {
		if (argc != 2) {
			printf("Error, invalid number of parameters\n");
			return_code = SHELL_EXIT_ERROR;
			print_usage = TRUE;
		} else {
			if (argc == 2) {
				hex[0] = argv[1][0];
				hex[1] = argv[1][1];
				strupr(hex);
				baseconfig->uid[0] = ahextoi(hex);
				
				hex[0] = argv[1][3];
				hex[1] = argv[1][4];
				strupr(hex);
				baseconfig->uid[1] = ahextoi(hex);
				
				hex[0] = argv[1][6];
				hex[1] = argv[1][7];
				strupr(hex);
				baseconfig->uid[2] = ahextoi(hex);
				
				hex[0] = argv[1][9];
				hex[1] = argv[1][10];
				strupr(hex);
				baseconfig->uid[3] = ahextoi(hex);
				
				//写入FLASH中
				EepromWrite(&baseconfig->uid[0], 12, 4);
				printf("请务必确保修改的UID唯一，不能有冲突！\n");
				printf("现场从小端开始使用，测试从最大端开始使用！\n");
				printf("UID的设置规则为：\n");
				printf("2X-XX-XX-XX\n");
				printf("4个字节，八个十六进制字符。其中上面的X代表'0'-'9'和'A'-'F'的任意十六进制字符\n");
				printf("需严格按照这种格式来书写，多一个字符或者少个字符都不行\n");
			}
		}
	}

	if (print_usage) {
		if (shorthelp) {
			printf("%s <uid>\n", argv[0]);
		} else {
			printf("Usage: %s <uid>\n", argv[0]);
			printf("   <uid> = XX-XX-XX-XX\n");
		}
	}
	return return_code;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   
* Returned Value   :  
* Comments  :  
*
*END*---------------------------------------------------------------------*/

int_32 Shell_getuid(int_32 argc, char_ptr argv[])
{
	boolean print_usage, shorthelp = FALSE;
	int_32 return_code = SHELL_EXIT_SUCCESS;


	print_usage = Shell_check_help_request(argc, argv, &shorthelp);

	if (!print_usage) {
		if (argc > 1) {
			printf("Error, invalid number of parameters\n");
			return_code = SHELL_EXIT_ERROR;
			print_usage = TRUE;
		} else {
			
				printf("[UID] = %02X-%02X-%02X-%02X\n", pBaseConfig->uid[0],pBaseConfig->uid[1],pBaseConfig->uid[2],pBaseConfig->uid[3]);
		}
	}

	if (print_usage) {
		if (shorthelp) {
			printf("%s \n", argv[0]);
		} else {
			printf("Usage: %s \n", argv[0]);
		}
	}
	return return_code;
}


/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   
* Returned Value   :  int_32 error code
* Comments  :   
*
*END*---------------------------------------------------------------------*/

int_32 Shell_sethardversion(int_32 argc, char_ptr argv[])
{
	boolean print_usage, shorthelp = FALSE;
	int_32 return_code = SHELL_EXIT_SUCCESS;


	unsigned char writebuf[WRITEBUFSIZE];
	BASECONFIGTABLE_PTR baseconfig;

	baseconfig = (BASECONFIGTABLE_PTR) writebuf;


	print_usage = Shell_check_help_request(argc, argv, &shorthelp);

	if (!print_usage) {
		if (argc != 2) {
			printf("Error, invalid number of parameters\n");
			return_code = SHELL_EXIT_ERROR;
			print_usage = TRUE;
		} else {
			if (argc == 2) {
				// if ((sscanf(argv[1],"%d/%d/%d",&year,&month,&day)>=1)
				//   && (sscanf(argv[2],"%d:%d:%d",&hour,&minute,&second)>=1))
				//{
				
					
					baseconfig->hardversion[0] = argv[1][0];
					baseconfig->hardversion[1] = argv[1][1]-'0';
					baseconfig->hardversion[2] = argv[1][3]-'0';
					baseconfig->hardversion[3] = argv[1][5]-'0';
	
					
					//写入FLASH中
					EepromWrite(&baseconfig->hardversion[0], 4, 4);
			
				}
				// } else {
				//   printf("Invalid rtc time specified\n");
				//} 
			}
	}

	if (print_usage) {
		if (shorthelp) {
			printf("%s <hardversion>\n", argv[0]);
		} else {
			printf("Usage: %s <hardversion>\n", argv[0]);
			printf("   <hardversion> = VX.X.X\n");
		}
	}
	return return_code;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   
* Returned Value   :  
* Comments  :  
*
*END*---------------------------------------------------------------------*/

int_32 Shell_gethardversion(int_32 argc, char_ptr argv[])
{
	boolean print_usage, shorthelp = FALSE;
	int_32 return_code = SHELL_EXIT_SUCCESS;


	print_usage = Shell_check_help_request(argc, argv, &shorthelp);

	if (!print_usage) {
		if (argc > 1) {
			printf("Error, invalid number of parameters\n");
			return_code = SHELL_EXIT_ERROR;
			print_usage = TRUE;
		} else {
			
				printf("[hardversion] = %c%d.%d.%d\n", pBaseConfig->hardversion[0],pBaseConfig->hardversion[1],pBaseConfig->hardversion[2],pBaseConfig->hardversion[3]);
		}
	}

	if (print_usage) {
		if (shorthelp) {
			printf("%s \n", argv[0]);
		} else {
			printf("Usage: %s \n", argv[0]);
		}
	}
	return return_code;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   
* Returned Value   :  
* Comments  :  
*
*END*---------------------------------------------------------------------*/

int_32 Shell_getsoftversion(int_32 argc, char_ptr argv[])
{
	boolean print_usage, shorthelp = FALSE;
	int_32 return_code = SHELL_EXIT_SUCCESS;


	print_usage = Shell_check_help_request(argc, argv, &shorthelp);

	if (!print_usage) {
		if (argc > 1) {
			printf("Error, invalid number of parameters\n");
			return_code = SHELL_EXIT_ERROR;
			print_usage = TRUE;
		} else {
			
				printf("[SoftVersion] = %c%d.%d.%d\nbuild date: %s\n", pBaseConfig->softversion[0],pBaseConfig->softversion[1],pBaseConfig->softversion[2],pBaseConfig->softversion[3], __DATE__);
		}
	}

	if (print_usage) {
		if (shorthelp) {
			printf("%s \n", argv[0]);
		} else {
			printf("Usage: %s \n", argv[0]);
		}
	}
	return return_code;
}


/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   Shell_setrtc
* Returned Value   :  int_32 error code
* Comments  :  设置RTC时间 .
*
*END*---------------------------------------------------------------------*/

int_32 Shell_setrtc(int_32 argc, char_ptr argv[])
{
	boolean print_usage, shorthelp = FALSE;
	int_32 return_code = SHELL_EXIT_SUCCESS;
	uint_16 year;

	/* 1 - 12 */
	uint_16 month;
	/* 1 - 31 (depending on month) */
	uint_16 day;
	/* 0 - 23 */
	uint_16 hour;
	/* 0 - 59 */
	uint_16 minute;
	/* 0 - 59 */
	uint_16 second;
	DATE_STRUCT date;
	TIME_STRUCT time;
	RTC_TIME_STRUCT time_rtc;


	print_usage = Shell_check_help_request(argc, argv, &shorthelp);

	if (!print_usage) {
		if (argc != 3) {
			printf("Error, invalid number of parameters\n");
			return_code = SHELL_EXIT_ERROR;
			print_usage = TRUE;
		} else {
			if (argc == 3) {
				// if ((sscanf(argv[1],"%d/%d/%d",&year,&month,&day)>=1)
				//   && (sscanf(argv[2],"%d:%d:%d",&hour,&minute,&second)>=1))
				//{
				year =
				    1000 * (argv[1][0] - '0') +
				    100 * (argv[1][1] - '0') +
				    10 * (argv[1][2] - '0') + argv[1][3] -
				    '0';
				month =
				    10 * (argv[1][5] - '0') + argv[1][6] -
				    '0';
				day =
				    10 * (argv[1][8] - '0') + argv[1][9] -
				    '0';
				hour =
				    10 * (argv[2][0] - '0') + argv[2][1] -
				    '0';
				minute =
				    10 * (argv[2][3] - '0') + argv[2][4] -
				    '0';
				second =
				    10 * (argv[2][6] - '0') + argv[2][7] -
				    '0';
				if (year < 1000 || month > 12 || month < 1
				    || day > 31 || day < 1 || hour > 23
				    || minute > 59 || second > 59) {
					printf
					    ("Invalid rtctime specified, format is YYYY/MM/DD HH:MM:SS\n");
				} else {
					date.YEAR = year;
					date.MONTH = month;
					date.DAY = day;
					date.HOUR = hour;
					date.MINUTE = minute;
					date.SECOND = second;
					date.MILLISEC = 0;

					_time_from_date(&date, &time);
					_rtc_time_from_mqx_time(&time,
								&time_rtc);

					printf
					    ("Set RTC Time: %04d %02d:%02d:%02d\r\n",
					     time_rtc.days, time_rtc.hours,
					     time_rtc.minutes,
					     time_rtc.seconds);

					_rtc_set_time(&time_rtc);
				}
				// } else {
				//   printf("Invalid rtc time specified\n");
				//} 
			}

		}
	}

	if (print_usage) {
		if (shorthelp) {
			printf("%s <rtctime>\n", argv[0]);
		} else {
			printf("Usage: %s <rtctime>\n", argv[0]);
			printf("   <rtctime> = YYYY/MM/DD HH:MM:SS\n");
		}
	}
	return return_code;
}



/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   Shell_getrtc
* Returned Value   :  int_32 error code
* Comments  :  读取RTC时间 .
*
*END*---------------------------------------------------------------------*/

int_32 Shell_getrtc(int_32 argc, char_ptr argv[])
{
	boolean print_usage, shorthelp = FALSE;
	int_32 return_code = SHELL_EXIT_SUCCESS;

	DATE_STRUCT date;
	TIME_STRUCT time;
	RTC_TIME_STRUCT time_rtc;

	print_usage = Shell_check_help_request(argc, argv, &shorthelp);

	if (!print_usage) {
		if (argc > 1) {
			printf("Error, invalid number of parameters\n");
			return_code = SHELL_EXIT_ERROR;
			print_usage = TRUE;
		} else {
			_rtc_get_time(&time_rtc);
			_rtc_time_to_mqx_time(&time_rtc, &time);
			_time_to_date(&time, &date);
			printf("RTC Time: %04d/%02d/%02d %02d:%02d:%02d\n",
			       date.YEAR, date.MONTH, date.DAY, date.HOUR,
			       date.MINUTE, date.SECOND);
		}
	}

	if (print_usage) {
		if (shorthelp) {
			printf("%s \n", argv[0]);
		} else {
			printf("Usage: %s \n", argv[0]);
		}
	}
	return return_code;
}


int_32 Shell_wall(int_32 argc, char_ptr argv[])
{
	boolean print_usage, shorthelp = FALSE;
	int_32 return_code = SHELL_EXIT_SUCCESS;
	char_ptr argv1[] = {"wbaseconfig","wregtable","wroutertable","wdidi_diantable"};

	print_usage = Shell_check_help_request(argc, argv, &shorthelp);

	if (!print_usage) {
		if (argc > 1) {
			printf("Error, invalid number of parameters\n");
			return_code = SHELL_EXIT_ERROR;
			print_usage = TRUE;
		} else {
			printf ("wall has be instead by wbaseconfig wregtable!\n");
			//Shell_wbaseconfig(1, argv1);
			//Shell_wregtable(1, argv1+1);
		    //Shell_wroutertable(1, argv1+2);
			//Shell_wdidi_diantable(1, argv1+3);
		}
	}

	if (print_usage) {
		if (shorthelp) {
			printf("%s \n", argv[0]);
		} else {
			printf("Usage: %s \n", argv[0]);
		}
	}
	return return_code;
}

int_32 Shell_rall(int_32 argc, char_ptr argv[])
{
	boolean print_usage, shorthelp = FALSE;
	int_32 return_code = SHELL_EXIT_SUCCESS;
	char_ptr argv1[] = {"rbaseconfig","rregtable","rroutertable","rdidi_diantable"};

	print_usage = Shell_check_help_request(argc, argv, &shorthelp);

	if (!print_usage) {
		if (argc > 1) {
			printf("Error, invalid number of parameters\n");
			return_code = SHELL_EXIT_ERROR;
			print_usage = TRUE;
		} else {
			Shell_rbaseconfig(1, argv1);
			Shell_rregtable(1, argv1+1);
			Shell_rroutertable(1, argv1+2);
			Shell_rdidi_diantable(1, argv1+3);
		}
	}

	if (print_usage) {
		if (shorthelp) {
			printf("%s \n", argv[0]);
		} else {
			printf("Usage: %s \n", argv[0]);
		}
	}
	return return_code;
}
int_32 Shell_pall(int_32 argc, char_ptr argv[])
{
	boolean print_usage, shorthelp = FALSE;
	int_32 return_code = SHELL_EXIT_SUCCESS;
	char_ptr argv1[] = {"pbaseconfig","pregtable","proutertable","pdidi_diantable"};

	print_usage = Shell_check_help_request(argc, argv, &shorthelp);

	if (!print_usage) {
		if (argc > 1) {
			printf("Error, invalid number of parameters\n");
			return_code = SHELL_EXIT_ERROR;
			print_usage = TRUE;
		} else {
			Shell_pbaseconfig(1, argv1);
			Shell_pregtable(1, argv1+1);
			Shell_proutertable(1, argv1+2);
			Shell_pdidi_diantable(1, argv1+3);
		}
	}

	if (print_usage) {
		if (shorthelp) {
			printf("%s \n", argv[0]);
		} else {
			printf("Usage: %s \n", argv[0]);
		}
	}
	return return_code;
}


int_32 Shell_wbaseconfig(int_32 argc, char_ptr argv[])
{

	boolean print_usage, shorthelp = FALSE;
	int_32 return_code = SHELL_EXIT_SUCCESS;
	FILE_PTR fd_ptr;
	char s[100];
	char hex[3];
	char t[40];

	uint_32 ipaddr;

	BASECONFIGTABLE_PTR baseconfig;
	unsigned char *writebuf;
	writebuf = (unsigned char *) _mem_alloc_zero_from(_user_pool_id, 512);
    if (writebuf == NULL)
    {
    	printf("error when mem alloc\r\n");	
    }
	_mem_copy (pBaseConfig, writebuf, WRITEBUFSIZE-16);
	baseconfig = (BASECONFIGTABLE_PTR) writebuf;
	hex[2] = '\0';
	
	print_usage = Shell_check_help_request(argc, argv, &shorthelp);
	if (!print_usage) {
		if (argc > 1) {
			printf("Error, invalid number of parameters\n");
			return_code = SHELL_EXIT_ERROR;
			print_usage = TRUE;
		} else {
			fd_ptr = fopen("d:\\dicorconfig\\baseconfig.txt", "r");
			if (fd_ptr == NULL) {
				printf ("The Config File baseconfig.txt is not exitting\n");
				return_code = SHELL_EXIT_ERROR;
				print_usage = TRUE;
				fclose(fd_ptr);
				return return_code;
			}
			
			printf("\n*******************************\n");
			printf("解析文件 baseconfig.txt\n");

			while (strcmp(s, "[BEGIN]") != 0) {
				fgetline(fd_ptr, s, 64);	//[BEGIN]
				if (feof(fd_ptr))
				{
					printf("\n");
					printf("没有找到文件头[BEGIN]，请用ls,dir,type等命令检查文件是否正确！\n");
					fclose(fd_ptr);
					return_code = SHELL_EXIT_ERROR;
					print_usage = TRUE;
					return return_code;
				}
				printf("%s\n", s);
			}

			fgetline(fd_ptr, s, 64);	//[DHCP]
			fgetline(fd_ptr, s, 64);
			printf("%s\n", s);
			if (cutstr(s, t, '='))
			{
				printf("文本数据书写格式错误\n");
				return_code = SHELL_EXIT_ERROR;
				print_usage = TRUE;
				return return_code;
			}
				
			baseconfig->dhcp = t[0] - '0';
			
			fgetline(fd_ptr, s, 64);	//[ENABLE_DNS]
			fgetline(fd_ptr, s, 64);
			printf("%s\n", s);
			if (cutstr(s, t, '='))
			{
				printf("文本数据书写格式错误\n");
				return_code = SHELL_EXIT_ERROR;
				print_usage = TRUE;
				return return_code;
			}
				
			baseconfig->enabledns = t[0] - '0';
			
			
			fgetline(fd_ptr, s, 64);	//[ENABLE_SNTP]
			fgetline(fd_ptr, s, 64);
			printf("%s\n", s);
			if (cutstr(s, t, '='))
			{
				printf("文本数据书写格式错误\n");
				return_code = SHELL_EXIT_ERROR;
				print_usage = TRUE;
				return return_code;
			}
				
			baseconfig->enablesntp = t[0] - '0';


			fgetline(fd_ptr, s, 64);	//[IP] 
			fgetline(fd_ptr, s, 64);
			printf("%s\n", s);
			if (cutstr(s, t, '='))
			{
				printf("文本数据书写格式错误\n");
				return_code = SHELL_EXIT_ERROR;
				print_usage = TRUE;
				return return_code;
			}
				
			get_dotted_address(t, &ipaddr);
			baseconfig->ip[0] = (uint_8) (ipaddr >> 24);
			baseconfig->ip[1] = (uint_8) (ipaddr >> 16);
			baseconfig->ip[2] = (uint_8) (ipaddr >> 8);
			baseconfig->ip[3] = (uint_8) (ipaddr);

			fgetline(fd_ptr, s, 64);	//[MASK]
			fgetline(fd_ptr, s, 64);
			printf("%s\n", s);
			if (cutstr(s, t, '='))
			{
				printf("文本数据书写格式错误\n");
				return_code = SHELL_EXIT_ERROR;
				print_usage = TRUE;
				return return_code;
			}
				
			get_dotted_address(t, &ipaddr);
			baseconfig->mask[0] = (uint_8) (ipaddr >> 24);
			baseconfig->mask[1] = (uint_8) (ipaddr >> 16);
			baseconfig->mask[2] = (uint_8) (ipaddr >> 8);
			baseconfig->mask[3] = (uint_8) (ipaddr);

			fgetline(fd_ptr, s, 94);	//[GATEWAY]
			fgetline(fd_ptr, s, 94);
			printf("%s\n", s);
			if (cutstr(s, t, '='))
			{
				printf("文本数据书写格式错误\n");
				return_code = SHELL_EXIT_ERROR;
				print_usage = TRUE;
				return return_code;
			}
				
			get_dotted_address(t, &ipaddr);
			baseconfig->gateway[0] = (uint_8) (ipaddr >> 24);
			baseconfig->gateway[1] = (uint_8) (ipaddr >> 16);
			baseconfig->gateway[2] = (uint_8) (ipaddr >> 8);
			baseconfig->gateway[3] = (uint_8) (ipaddr);

			fgetline(fd_ptr, s, 94);	//[DNS]
			fgetline(fd_ptr, s, 94);
			printf("%s\n", s);
			if (cutstr(s, t, '='))
			{
				printf("文本数据书写格式错误\n");
				return_code = SHELL_EXIT_ERROR;
				print_usage = TRUE;
				return return_code;
			}
				
			get_dotted_address(t, &ipaddr);
			baseconfig->dns[0] = (uint_8) (ipaddr >> 24);
			baseconfig->dns[1] = (uint_8) (ipaddr >> 16);
			baseconfig->dns[2] = (uint_8) (ipaddr >> 8);
			baseconfig->dns[3] = (uint_8) (ipaddr);

			fgetline(fd_ptr, s, 94);	//[SNTP1]
			fgetline(fd_ptr, s, 94);
			printf("%s\n", s);
			
			
			if (cutstr(s, baseconfig->sntp1, '='))
			{
				printf("文本数据书写格式错误\n");
				return_code = SHELL_EXIT_ERROR;
				print_usage = TRUE;
				return return_code;
			}
				
			
			fgetline(fd_ptr, s, 94);	//[SNTP2]
			fgetline(fd_ptr, s, 94);
			printf("%s\n", s);
			if (cutstr(s, baseconfig->sntp2, '='))
			{
				printf("文本数据书写格式错误\n");
				return_code = SHELL_EXIT_ERROR;
				print_usage = TRUE;
				return return_code;
			}
			
			fgetline(fd_ptr, s, 94);	//[SNTP3]
			fgetline(fd_ptr, s, 94);
			printf("%s\n", s);
			if (cutstr(s, baseconfig->sntp3, '='))
			{
				printf("文本数据书写格式错误\n");
				return_code = SHELL_EXIT_ERROR;
				print_usage = TRUE;
				return return_code;
			}
			
			fgetline(fd_ptr, s, 94);	//[SNTP4]
			fgetline(fd_ptr, s, 94);
			printf("%s\n", s);
			if (cutstr(s, baseconfig->sntp4, '='))
			{
				printf("文本数据书写格式错误\n");
				return_code = SHELL_EXIT_ERROR;
				print_usage = TRUE;
				return return_code;
			}

			fgetline(fd_ptr, s, 94);	//[DATACENTERDNS]
			fgetline(fd_ptr, s, 94);
			printf("%s\n", s);
			if (cutstr(s, baseconfig->datacenterdns, '='))
			{
				printf("文本数据书写格式错误\n");
				return_code = SHELL_EXIT_ERROR;
				print_usage = TRUE;
				return return_code;
			}
//////////////////////////////////////////////////////////////////			
			fgetline(fd_ptr, s, 94);	//[DATACENTER]
			fgetline(fd_ptr, s, 94);
			printf("%s\n", s);
			if (cutstr(s, t, '='))
			{
				printf("文本数据书写格式错误\n");
				return_code = SHELL_EXIT_ERROR;
				print_usage = TRUE;
				return return_code;
			}
			get_dotted_address(t, &ipaddr);
			baseconfig->datacentermux[0] = (uint_8) (ipaddr >> 24);
			baseconfig->datacentermux[1] = (uint_8) (ipaddr >> 16);
			baseconfig->datacentermux[2] = (uint_8) (ipaddr >> 8);
			baseconfig->datacentermux[3] = (uint_8) (ipaddr);
			
			fgetline(fd_ptr, s, 94);	//[IPPORT]
			fgetline(fd_ptr, s, 94);
			printf("%s\n", s);
			if (cutstr(s, t, '='))
			{
				printf("文本数据书写格式错误\n");
				fclose(fd_ptr);
				return_code = SHELL_EXIT_ERROR;
				print_usage = TRUE;
				return return_code;
			}
			ipaddr = atoi(t);
			baseconfig->ipportmux[0] = (uint_8) (ipaddr >> 24);
			baseconfig->ipportmux[1] = (uint_8) (ipaddr >> 16);
			baseconfig->ipportmux[2] = (uint_8) (ipaddr >> 8);
			baseconfig->ipportmux[3] = (uint_8) (ipaddr);
			if (!memcmp(baseconfig->ipport, baseconfig->ipportmux, 4)
				&& !memcmp(baseconfig->datacenter, baseconfig->datacentermux, 4))
				//|| !memcmp(baseconfig->datacentermux, 0, 2))
			{
				/****程序执行单IP功能****/
				SERVER_MODE = 1;
			}
			else 
				SERVER_MODE = 2;			
///////////////////////////////////////////////////////////////////			
			fgetline(fd_ptr, s, 94);	//[DATACENTER2]
			fgetline(fd_ptr, s, 94);
			printf("%s\n", s);			
			if (0 == strncmp(s, "[DATACENTER]", 12))
			{
				if (cutstr(s, t, '='))
				{
					printf("文本数据书写格式错误\n");
					return_code = SHELL_EXIT_ERROR;
					print_usage = TRUE;
					return return_code;
				}
				get_dotted_address(t, &ipaddr);
				baseconfig->datacentermux[0] = (uint_8) (ipaddr >> 24);
				baseconfig->datacentermux[1] = (uint_8) (ipaddr >> 16);
				baseconfig->datacentermux[2] = (uint_8) (ipaddr >> 8);
				baseconfig->datacentermux[3] = (uint_8) (ipaddr);
				
				fgetline(fd_ptr, s, 94);	//[IPPORT2]
				fgetline(fd_ptr, s, 94);
				printf("%s\n", s);
				if (cutstr(s, t, '='))
				{
					printf("文本数据书写格式错误\n");
					fclose(fd_ptr);
					return_code = SHELL_EXIT_ERROR;
					print_usage = TRUE;
					return return_code;
				}
				ipaddr = atoi(t);
				baseconfig->ipportmux[0] = (uint_8) (ipaddr >> 24);
				baseconfig->ipportmux[1] = (uint_8) (ipaddr >> 16);
				baseconfig->ipportmux[2] = (uint_8) (ipaddr >> 8);
				baseconfig->ipportmux[3] = (uint_8) (ipaddr);
				
				if (!memcmp(baseconfig->ipport, baseconfig->ipportmux, 4)
					&& !memcmp(baseconfig->datacenter, baseconfig->datacentermux, 4))
					//|| !memcmp(baseconfig->datacentermux, 0, 2))
				{
					/****程序执行单IP功能****/
					SERVER_MODE = 1;
				}
				else 
					SERVER_MODE = 2;
				
				fgetline(fd_ptr, s, 94);	//[DICOR_NET_ADDR]
				fgetline(fd_ptr, s, 94);
				printf("%s\n", s);
			}
				
			if (cutstr(s, t, '='))
			{
				printf("文本数据书写格式错误\n");
				return_code = SHELL_EXIT_ERROR;
				print_usage = TRUE;
				return return_code;
			}
				
			//
			hex[0] = t[2];
			hex[1] = t[3];
			strupr(hex);
			baseconfig->net_addr = ahextoi(hex);
		
			fgetline(fd_ptr, s, 94);	//[SUBNET_NUM]
			fgetline(fd_ptr, s, 94);
			printf("%s\n", s);
			if (cutstr(s, t, '='))
			{
				printf("文本数据书写格式错误\n");
				return_code = SHELL_EXIT_ERROR;
				print_usage = TRUE;
				return return_code;
			}
			baseconfig->subnet_num = atoi(t);
			
			fgetline(fd_ptr, s, 94);	//[SUBDEV_NUM]
			fgetline(fd_ptr, s, 94);
			printf("%s\n", s);
			if (cutstr(s, t, '='))
			{
				printf("文本数据书写格式错误\n");
				return_code = SHELL_EXIT_ERROR;
				print_usage = TRUE;
				return return_code;
			}
			baseconfig->subdev_num = atoi(t);

			fgetline(fd_ptr, s, 94);	//[WORKMODE]
			fgetline(fd_ptr, s, 94);
			printf("%s\n", s);
			
			if (strcmp(s, "[END]") != 0)
			{
			
				if (cutstr(s, t, '='))
				{
					printf("文本数据书写格式错误\n");
					return_code = SHELL_EXIT_ERROR;
					print_usage = TRUE;
					return return_code;
				}
				baseconfig->work_mode= atoi(t);
		

				fgetline(fd_ptr, s, 94);	//[END]
				fgetline(fd_ptr, s, 94);
				printf("%s\n", s);
				if (strcmp(s, "[END]") != 0)
				{
					if (cutstr(s, t, '='))
					{
						printf("文本数据书写格式错误\n");
						return_code = SHELL_EXIT_ERROR;
						print_usage = TRUE;
						return return_code;
					}
					ipaddr = atoi(t);
					baseconfig->rffreq[0] = (uint_8) (ipaddr >> 24);
					baseconfig->rffreq[1] = (uint_8) (ipaddr >> 16);
					baseconfig->rffreq[2] = (uint_8) (ipaddr >> 8);
					baseconfig->rffreq[3] = (uint_8) (ipaddr);
					
					fgetline(fd_ptr, s, 94);	
					fgetline(fd_ptr, s, 94);
					printf("%s\n", s);
					
					if (cutstr(s, t, '='))
					{
						printf("文本数据书写格式错误\n");
						return_code = SHELL_EXIT_ERROR;
						print_usage = TRUE;
						return return_code;
					}
					baseconfig->rfpower = atoi(t);
					
					fgetline(fd_ptr, s, 94);	//[END]
					fgetline(fd_ptr, s, 94);
					printf("%s\n", s);
				}
				else
				{
					ipaddr = 430000;
					baseconfig->rffreq[0] = (uint_8) (ipaddr >> 24);
					baseconfig->rffreq[1] = (uint_8) (ipaddr >> 16);
					baseconfig->rffreq[2] = (uint_8) (ipaddr >> 8);
					baseconfig->rffreq[3] = (uint_8) (ipaddr);
					baseconfig->rfpower = 4;
				}
			}
			else
			{
				baseconfig->work_mode = 0;
				ipaddr = 430000;
				baseconfig->rffreq[0] = (uint_8) (ipaddr >> 24);
				baseconfig->rffreq[1] = (uint_8) (ipaddr >> 16);
				baseconfig->rffreq[2] = (uint_8) (ipaddr >> 8);
				baseconfig->rffreq[3] = (uint_8) (ipaddr);
				baseconfig->rfpower = 4;
			}
			fclose(fd_ptr);
			EepromWrite(&baseconfig->dhcp, 16, WRITEBUFSIZE-16);
		}
	}
	if (print_usage) {
		if (shorthelp) {
			printf("%s \n", argv[0]);
		} else {
			printf("Usage: %s \n", argv[0]);
			printf("   help information\n");
		}
	}
	if (writebuf != NULL)
	{
		_mem_free(writebuf);
	}
	return return_code;
}

int_32 Shell_wregtable(int_32 argc, char_ptr argv[])
{


	boolean print_usage, shorthelp = FALSE;
	int_32 return_code = SHELL_EXIT_SUCCESS;

	char s[100];
	uchar i;
	uint_32 j, page;
	FILE_PTR fd_ptr;
	char hex[3];
	//unsigned char writebuf[WRITEBUFSIZE];
	unsigned char *writebuf;
	//测试申请外部内存
	writebuf = (unsigned char *) _mem_alloc_zero_from(_user_pool_id, 512);
    if (writebuf == NULL)
    {
    	printf("error when mem alloc\r\n");	
    }

	hex[2] = '\0';

	print_usage = Shell_check_help_request(argc, argv, &shorthelp);

	if (!print_usage) {
		if (argc > 1) {
			printf("Error, invalid number of parameters\n");
			return_code = SHELL_EXIT_ERROR;
			print_usage = TRUE;
		} else {

			//先打开配置文件
			fd_ptr = fopen("d:\\dicorconfig\\regtable.txt", "r");
			if (fd_ptr == NULL) {
				printf ("The Config File regtable.txt is not exitting\n");
				return_code = SHELL_EXIT_ERROR;
				print_usage = TRUE;
				fclose(fd_ptr);
				return return_code;
			}
			
			printf("\n*******************************\n");
			printf("解析文件 regtable.txt\n");

			while (strcmp(s, "[BEGIN]") != 0) {
				fgetline(fd_ptr, s, 64);	//[BEGIN]
				if (feof(fd_ptr))
				{
					printf("\n");
					printf("没有找到文件头[BEGIN]，请用ls,dir,type等命令检查文件是否正确！\n");
					fclose(fd_ptr);
					return_code = SHELL_EXIT_ERROR;
					print_usage = TRUE;
					return return_code;
				}
			}

			j = 0;
			page = 0;
			while (strcmp(s, "[END]") != 0) {
				fgetline(fd_ptr, s, 64);	//
				fgetline(fd_ptr, s, 64);
				printf("%s\n", s);
				i = 0;
				while (s[i * 5] != '\0') {
					hex[0] = s[i * 5 + 2];
					hex[1] = s[i * 5 + 3];
					i++;
					strupr(hex);
					writebuf[j++] = ahextoi(hex);
					if (j == WRITEBUFSIZE) {
						j = 0;
						//写入FLASH中
						EepromWrite(writebuf,
							    BASECONFIGSIZE
							    +
							    ROUTERTABLESIZE
							    +
							    page *
							    WRITEBUFSIZE,
							    WRITEBUFSIZE);
						page++;
					}
				}
			}

			if (j != 0) {
				//写不足一页的数据
				//写入FLASH中
				EepromWrite(writebuf,
					    BASECONFIGSIZE +
					    ROUTERTABLESIZE +
					    page * WRITEBUFSIZE,
					    j);
			}

			fclose(fd_ptr);

		}
	}

	if (print_usage) {
		if (shorthelp) {
			printf("%s \n", argv[0]);
		} else {
			printf("Usage: %s \n", argv[0]);
		}
	}
	if (writebuf != NULL)
	{
		_mem_free(writebuf);
	}
	return return_code;

}


extern EEPROMDATA_PTR pEeprom;
extern _mem_size base_addr;

int_32 Shell_pregtable(int_32 argc, char_ptr argv[])
{
	boolean print_usage, shorthelp = FALSE;
	int_32 return_code = SHELL_EXIT_SUCCESS;

	unsigned char test;
	unsigned int i;

	print_usage = Shell_check_help_request(argc, argv, &shorthelp);

	if (!print_usage) {
		if (argc > 1) {
			printf("Error, invalid number of parameters\n");
			return_code = SHELL_EXIT_ERROR;
			print_usage = TRUE;
		} else {

			pEeprom = (EEPROMDATA_PTR) base_addr;


			printf("\n*************RegTable************\n");
			for (i = 0; i < REGTABLESIZE; i++) {
				test = pEeprom->RegTable[i];

				printf("%02X ", test);
				if (((i - 7) % 6 == 0) && (i >= 7)) {
					printf("\n");
				}
			}
			printf("\n********************************\n");
		}
	}

	if (print_usage) {
		if (shorthelp) {
			printf("%s \n", argv[0]);
		} else {
			printf("Usage: %s \n", argv[0]);
		}
	}
	return return_code;
}

int_32 Shell_rregtable(int_32 argc, char_ptr argv[])
{
	boolean print_usage, shorthelp = FALSE;
	int_32 return_code = SHELL_EXIT_SUCCESS;
	
	FILE_PTR		  fd_ptr;
	char temp[100];

	unsigned char test;
	unsigned int i;

	print_usage = Shell_check_help_request(argc, argv, &shorthelp);

	if (!print_usage) {
		if (argc > 1) {
			printf("Error, invalid number of parameters\n");
			return_code = SHELL_EXIT_ERROR;
			print_usage = TRUE;
		} else {
		
		
			fd_ptr = fopen("d:\\dicorconfig\\bak_regtable.txt", "w+");
			if (fd_ptr)
			{	
				sprintf(temp, "#请按标准格式书写注册表\r\n");
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "#必须以0x开头，以逗号结尾，数据必须由2个字节组成， 不足两位以0补全\r\n");
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "#不能随意增减空格回车等空白字符，可以通过Ctrl+A全选看出多余的空白字符\r\n");
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "#必须保留开头和结尾标识\r\n\r\n");
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "[BEGIN]\r\n");
				fwrite(temp, strlen(temp), 1, fd_ptr);
				pEeprom = (EEPROMDATA_PTR) base_addr;
				for (i=0; i<7; i++)
				{
					test = pEeprom->RegTable[i];
					sprintf(temp, "0x%02X,", test);
					fwrite(temp, strlen(temp), 1, fd_ptr);
				}
				test = pEeprom->RegTable[i];
				sprintf(temp, "0x%02X,\r\n", test);
				fwrite(temp, strlen(temp), 1, fd_ptr);
				for (i=8; i<test*6+8; i++)
				{
					sprintf(temp, "0x%02X,", pEeprom->RegTable[i]);
					fwrite(temp, strlen(temp), 1, fd_ptr);
					if ((i - 7) % 6 == 0) 
					{
						sprintf(temp, "\r\n");
						fwrite(temp, strlen(temp), 1, fd_ptr);
					}
				}
					
				sprintf(temp, "[END]");
				fwrite(temp, strlen(temp), 1, fd_ptr);
				fclose(fd_ptr);
			}
			else
			{
				printf("open file err!\n");
			}
		}
	}

	if (print_usage) {
		if (shorthelp) {
			printf("%s \n", argv[0]);
		} else {
			printf("Usage: %s \n", argv[0]);
		}
	}
	return return_code;
}


int_32 Shell_pbaseconfig(int_32 argc, char_ptr argv[])
{
	boolean print_usage, shorthelp = FALSE;
	int_32 return_code = SHELL_EXIT_SUCCESS;
	print_usage = Shell_check_help_request(argc, argv, &shorthelp);

	if (!print_usage) {
		if (argc > 1) {
			printf("Error, invalid number of parameters\n");
			return_code = SHELL_EXIT_ERROR;
			print_usage = TRUE;
		} else {


			printf("\n*************BaseConfig************\n");
			printf("[BEGIN]\n");
		//	printf("[SCANADDR] = %02X-%02X-%02X-%02X\n", pBaseConfig->scanaddr[0],pBaseConfig->scanaddr[1],pBaseConfig->scanaddr[2],pBaseConfig->scanaddr[3]);
			printf("[DHCP] = %d\n", pBaseConfig->dhcp);
			printf("[ENABLE_DNS] = %d\n", pBaseConfig->enabledns);
			printf("[ENABLE_SNTP] = %d\n", pBaseConfig->enablesntp);
			printf("[IP] = %d.%d.%d.%d\n",pBaseConfig->ip[0],pBaseConfig->ip[1],pBaseConfig->ip[2],pBaseConfig->ip[3]);
			printf("[MASK] = %d.%d.%d.%d\n",pBaseConfig->mask[0],pBaseConfig->mask[1],pBaseConfig->mask[2],pBaseConfig->mask[3]);
			printf("[GATEWAY] = %d.%d.%d.%d\n",pBaseConfig->gateway[0],pBaseConfig->gateway[1],pBaseConfig->gateway[2],pBaseConfig->gateway[3]);
			printf("[DNS] = %d.%d.%d.%d\n",pBaseConfig->dns[0],pBaseConfig->dns[1],pBaseConfig->dns[2],pBaseConfig->dns[3]);
			printf("[SNTP1] = %s\n", pBaseConfig->sntp1);
			printf("[SNTP2] = %s\n", pBaseConfig->sntp2);
			printf("[SNTP3] = %s\n", pBaseConfig->sntp3);
			printf("[SNTP4] = %s\n", pBaseConfig->sntp4);
			printf("[DATACENTERDNS] = %s\n", pBaseConfig->datacenterdns);
			printf("[DATACENTER] = %d.%d.%d.%d\n",pBaseConfig->datacenter[0],pBaseConfig->datacenter[1],pBaseConfig->datacenter[2],pBaseConfig->datacenter[3]);
			printf("[IPPORT] = %d\n", IPADDR(pBaseConfig->ipport[0],pBaseConfig->ipport[1],pBaseConfig->ipport[2],pBaseConfig->ipport[3]));
			if (SERVER_MODE == 2)
			{
				printf("[DATACENTERMUX] = %d.%d.%d.%d\n",pBaseConfig->datacentermux[0],pBaseConfig->datacentermux[1],pBaseConfig->datacentermux[2],pBaseConfig->datacentermux[3]);
				printf("[IPPORTMUX] = %d\n", IPADDR(pBaseConfig->ipportmux[0],pBaseConfig->ipportmux[1],pBaseConfig->ipportmux[2],pBaseConfig->ipportmux[3]));
			}
			printf("[DICOR_NET_ADDR]  = 0x%02X\n", pBaseConfig->net_addr);
			printf("[SUBNET_NUM]  = %d\n", pBaseConfig->subnet_num);
			printf("[SUBDEV_NUM]  = %d\n", pBaseConfig->subdev_num);
			printf("[WORKMODE]  = %d\n", pBaseConfig->work_mode);
			printf("[END]\n");
			printf("************************************\n");
			
		}
	}

	if (print_usage) {
		if (shorthelp) {
			printf("%s \n", argv[0]);
		} else {
			printf("Usage: %s \n", argv[0]);
		}
	}
	return return_code;
}


int_32 Shell_rbaseconfig(int_32 argc, char_ptr argv[])
{
	boolean print_usage, shorthelp = FALSE;
	int_32 return_code = SHELL_EXIT_SUCCESS;
	FILE_PTR		  fd_ptr;
	//char temp[100];

	print_usage = Shell_check_help_request(argc, argv, &shorthelp);

	if (!print_usage) {
		if (argc > 1) {
			printf("Error, invalid number of parameters\n");
			return_code = SHELL_EXIT_ERROR;
			print_usage = TRUE;
		} else {
			fd_ptr = fopen("d:\\dicorconfig\\bak_baseconfig.txt", "w+");
			if (fd_ptr) {
/*				
				sprintf(temp, "#请务必按照规定的格式编写数据\r\n");
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "#请不要随意增减空格换行等空白字符，可以通过Ctrl+A全选看出多余的空白字符\r\n");
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "#必须保留开头和结尾标识\r\n\r\n");
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "[BEGIN]\r\n");
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "[UID] = %02X-%02X-%02X-%02X\r\n", pBaseConfig->uid[0],pBaseConfig->uid[1],pBaseConfig->uid[2],pBaseConfig->uid[3]);
				fwrite(temp, strlen(temp), 1, fd_ptr);
			//	sprintf(temp, "[SCANADDR] = %02X-%02X-%02X-%02X\r\n", pBaseConfig->scanaddr[0],pBaseConfig->scanaddr[1],pBaseConfig->scanaddr[2],pBaseConfig->scanaddr[3]);
			//	fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "[DHCP] = %d\r\n", pBaseConfig->dhcp);
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "[ENABLE_DNS] = %d\r\n", pBaseConfig->enabledns);
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "[ENABLE_SNTP] = %d\r\n", pBaseConfig->enablesntp);
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "[IP] = %d.%d.%d.%d\r\n",pBaseConfig->ip[0],pBaseConfig->ip[1],pBaseConfig->ip[2],pBaseConfig->ip[3]);
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "[MASK] = %d.%d.%d.%d\r\n",pBaseConfig->mask[0],pBaseConfig->mask[1],pBaseConfig->mask[2],pBaseConfig->mask[3]);
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "[GATEWAY] = %d.%d.%d.%d\r\n",pBaseConfig->gateway[0],pBaseConfig->gateway[1],pBaseConfig->gateway[2],pBaseConfig->gateway[3]);
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "[DNS] = %d.%d.%d.%d\r\n",pBaseConfig->dns[0],pBaseConfig->dns[1],pBaseConfig->dns[2],pBaseConfig->dns[3]);
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "[SNTP1] = %s\r\n", pBaseConfig->sntp1);
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "[SNTP2] = %s\r\n", pBaseConfig->sntp2);
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "[SNTP3] = %s\r\n", pBaseConfig->sntp3);
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "[SNTP4] = %s\r\n", pBaseConfig->sntp4);
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "[DATACENTERDNS] = %s\r\n", pBaseConfig->datacenterdns);
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "[DATACENTER] = %d.%d.%d.%d\r\n",pBaseConfig->datacenter[0],pBaseConfig->datacenter[1],pBaseConfig->datacenter[2],pBaseConfig->datacenter[3]);
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "[IPPORT] = %d\r\n", IPADDR(pBaseConfig->ipport[0],pBaseConfig->ipport[1],pBaseConfig->ipport[2],pBaseConfig->ipport[3]));
				fwrite(temp, strlen(temp), 1, fd_ptr);
				if (SERVER_MODE == 2)
				{
					sprintf(temp, "[DATACENTERMUX] = %d.%d.%d.%d\r\n",pBaseConfig->datacentermux[0],pBaseConfig->datacentermux[1],pBaseConfig->datacentermux[2],pBaseConfig->datacentermux[3]);
					fwrite(temp, strlen(temp), 1, fd_ptr);
					sprintf(temp, "[IPPORTMUX] = %d\r\n", IPADDR(pBaseConfig->ipportmux[0],pBaseConfig->ipportmux[1],pBaseConfig->ipportmux[2],pBaseConfig->ipportmux[3]));
					fwrite(temp, strlen(temp), 1, fd_ptr);
				}
				sprintf(temp, "[DICOR_NET_ADDR] = 0x%2X\r\n", pBaseConfig->net_addr);
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "[SUBNET_NUM] = %d\r\n", pBaseConfig->subnet_num);
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "[SUBDEV_NUM] = %d\r\n", pBaseConfig->subdev_num);
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "[WORKMODE] = %d\r\n", pBaseConfig->work_mode);
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "[END]");
				fwrite(temp, strlen(temp), 1, fd_ptr);
*/				
				fclose(fd_ptr);
			}
			else
			{
				printf("open file err!\n");
			}
			
		}
	}

	if (print_usage) {
		if (shorthelp) {
			printf("%s \n", argv[0]);
		} else {
			printf("Usage: %s \n", argv[0]);
		}
	}
	return return_code;
}

int_32 Shell_rthirddevicetable(int_32 argc, char_ptr argv[])
{
	boolean print_usage, shorthelp = FALSE;
	int_32 return_code = SHELL_EXIT_SUCCESS;
	FILE_PTR		  fd_ptr;
	char s[100];

	print_usage = Shell_check_help_request(argc, argv, &shorthelp);

	if (!print_usage) {
		if (argc > 1) {
			printf("Error, invalid number of parameters\n");
			return_code = SHELL_EXIT_ERROR;
			print_usage = TRUE;
		}else {
			fd_ptr = fopen("d:\\dicorconfig\\thirddevicetable.txt", "r");
			if (fd_ptr)
			{	
				fgetline (fd_ptr, s, 72);
				while(!feof(fd_ptr))
				{					
					printf("%s\n", s);
					fgetline (fd_ptr, s, 72);					
				}
			
			} else
			{
				printf("open file err!\n");
				return_code = SHELL_EXIT_ERROR;
				print_usage = TRUE;
			}
		}
	}
	if (print_usage) {
		if (shorthelp) {
			printf("%s \n", argv[0]);
		} else {
			printf("Usage: %s \n", argv[0]);
		}
	}
	return return_code;
}

int_32 Shell_setdatacenter(int_32 argc, char_ptr argv[])
{
	boolean print_usage, shorthelp = FALSE;
	int_32 return_code = SHELL_EXIT_SUCCESS;

	uint_32 ipaddr[4];
	unsigned char writebuf[WRITEBUFSIZE];
	BASECONFIGTABLE_PTR baseconfig;

	baseconfig = (BASECONFIGTABLE_PTR) writebuf;
	print_usage = Shell_check_help_request(argc, argv, &shorthelp);

	if (!print_usage) {
		if (argc != 2) {
			printf("Error, invalid number of parameters\n");
			return_code = SHELL_EXIT_ERROR;
			print_usage = TRUE;
		} else {  			
			sscanf ((&argv[1][0]), "%3d.%3d.%3d.%3d", ipaddr, ipaddr+1, ipaddr+2, ipaddr+3 );
			baseconfig->datacenter[0] = ipaddr[0];
			baseconfig->datacenter[1] = ipaddr[1];
			baseconfig->datacenter[2] = ipaddr[2];
			baseconfig->datacenter[3] = ipaddr[3];
			EepromWrite(&baseconfig->datacenter[0], 236, 4);
		}
	}
	if (print_usage) {
		if (shorthelp) {
			printf("%s \n", argv[0]);
		} else {
			printf("Usage: %s \n", argv[0]);
		}
	}
	return return_code;
}

int_32 Shell_wdidi_diantable(int_32 argc, char_ptr argv[])
{
	boolean print_usage, shorthelp = FALSE;
	int_32 return_code = SHELL_EXIT_SUCCESS;
	char s[100];
	uchar i,k;
	uint_32 j;
	FILE_PTR fd_ptr;
	char hex[3];
	//unsigned char writebuf[WRITEBUFSIZE];
	unsigned short offset;
	unsigned char count;
	unsigned char didicount;
	unsigned char page;
	unsigned char *writebuf;
	//测试申请外部内存
	writebuf = (unsigned char *) _mem_alloc_zero_from(_user_pool_id, 512);
    if (writebuf == NULL)
    {
    	printf("error when mem alloc\r\n");	
    }

	hex[2] = '\0';

	print_usage = Shell_check_help_request(argc, argv, &shorthelp);

	if (!print_usage) {
		if (argc > 1) {
			printf("Error, invalid number of parameters\n");
			return_code = SHELL_EXIT_ERROR;
			print_usage = TRUE;
		} else {
			//先打开配置文件
			fd_ptr = fopen("d:\\dicorconfig\\didi_diantable.txt", "r");
			if (fd_ptr == NULL) {
				printf("The Config File is not exitting\n");
				return_code = SHELL_EXIT_ERROR;
				print_usage = TRUE;
			}
			
			printf("\n*******************************\n");
			printf("解析文件 didi_diantable.txt\n");
			while (strcmp(s, "[BEGIN]") != 0) {
				fgetline(fd_ptr, s, 64);	//[BEGIN]
				if (feof(fd_ptr))
				{
					printf("\n");
					printf("没有找到文件头[BEGIN]，请用ls,dir,type等命令检查文件是否正确！\n");
					fclose(fd_ptr);
					return_code = SHELL_EXIT_ERROR;
					print_usage = TRUE;
					return return_code;
				}
			}
			
			fgetline(fd_ptr, s, 64);	//
			fgetline(fd_ptr, s, 64);
			printf("%s\n", s);
			hex[0] = s[2];
			hex[1] = s[3];
			strupr(hex);
			writebuf[0] = 0;
			writebuf[1] = ahextoi(hex);
			count = writebuf[1];
			offset = 0;
			for (i=0; i<count; i++)
			{
				fgetline(fd_ptr, s, 64);	//
				fgetline(fd_ptr, s, 64);
				printf("%s\n", s);
				hex[0] = s[8];
				hex[1] = s[9];
				strupr(hex);
				writebuf[i*6+2] = ahextoi(hex);//地址
				hex[0] = s[13];
				hex[1] = s[14];
				strupr(hex);
				writebuf[i*6+3] = ahextoi(hex);//地址
				writebuf[i*6+4] = (offset>>8)&0xFF;
				writebuf[i*6+5] = offset&0xFF;
				fgetline(fd_ptr, s, 64);	//读出DIDI个数
				fgetline(fd_ptr, s, 64);
				printf("%s\n", s);
				hex[0] = s[2];
				hex[1] = s[3];
				strupr(hex);
				writebuf[i*6+6] = 0;
				writebuf[i*6+7] = ahextoi(hex);
				didicount = writebuf[i*6+7];
				offset += didicount*2;
				//跳过DiDi
				for (k=0; k<didicount; k++)
				{
					fgetline(fd_ptr, s, 64);
					fgetline(fd_ptr, s, 64);
				}
			}
			EepromWrite(writebuf,BASECONFIGSIZE+ROUTERTABLESIZE+REGTABLESIZE, writebuf[1]*6+2);
			fclose(fd_ptr);
				
			fd_ptr = fopen("d:\\dicorconfig\\didi_diantable.txt", "r");
			if (fd_ptr == NULL) {
				printf("The Config File is not exitting\n");
				return_code = SHELL_EXIT_ERROR;
				print_usage = TRUE;
			}
				
			offset = 0;
			while (strcmp(s, "[BEGIN]") != 0) {
				fgetline(fd_ptr, s, 64);	//[BEGIN]
			}
			
			fgetline(fd_ptr, s, 64);	//
			fgetline(fd_ptr, s, 64);
			printf("%s\n", s);
			j = 0;
			page = 0;
			for (i=0; i<count; i++)
			{
				fgetline(fd_ptr, s, 64);	//
				fgetline(fd_ptr, s, 64);
				printf("%s\n", s);
			
				fgetline(fd_ptr, s, 64);	//读出DIDI个数
				fgetline(fd_ptr, s, 64);
				printf("%s\n", s);
				hex[0] = s[2];
				hex[1] = s[3];
				strupr(hex);
				didicount = ahextoi(hex);
				for (k=0; k<didicount; k++)
				{
					fgetline(fd_ptr, s, 64);
					fgetline(fd_ptr, s, 64);
					hex[0] = s[2];
					hex[1] = s[3];
					strupr(hex);
					writebuf[j++] = ahextoi(hex);
					if (j == WRITEBUFSIZE) {
						j = 0;
						//写入FLASH中
						EepromWrite(writebuf,
							    BASECONFIGSIZE+ROUTERTABLESIZE+REGTABLESIZE+DIDIDIANINDEXTABSIZE+page*WRITEBUFSIZE,
							    WRITEBUFSIZE);
						page++;
					}
					hex[0] = s[7];
					hex[1] = s[8];
					strupr(hex);
					writebuf[j++] = ahextoi(hex);
					if (j == WRITEBUFSIZE) {
						j = 0;
						//写入FLASH中
						EepromWrite(writebuf,
							    BASECONFIGSIZE+ROUTERTABLESIZE+REGTABLESIZE+DIDIDIANINDEXTABSIZE+page*WRITEBUFSIZE,
							    WRITEBUFSIZE);
						page++;
					}
				}
			}	
			
			if (j != 0) {
				//写不足一页的数据
				//写入FLASH中
				EepromWrite(writebuf,
					    BASECONFIGSIZE+ROUTERTABLESIZE+REGTABLESIZE+DIDIDIANINDEXTABSIZE +
					    page * WRITEBUFSIZE,
					    j);
			}

			fclose(fd_ptr);
		}
	}

	if (print_usage) {
		if (shorthelp) {
			printf("%s \n", argv[0]);
		} else {
			printf("Usage: %s \n", argv[0]);
		}
	}
	if (writebuf != NULL)
	{
		_mem_free(writebuf);
	}
	return return_code;
}


int_32 Shell_pdidi_diantable(int_32 argc, char_ptr argv[])
{
	boolean print_usage, shorthelp = FALSE;
	int_32 return_code = SHELL_EXIT_SUCCESS;
	unsigned int i;
	unsigned char test;

	print_usage = Shell_check_help_request(argc, argv, &shorthelp);

	if (!print_usage) {
		if (argc > 1) {
			printf("Error, invalid number of parameters\n");
			return_code = SHELL_EXIT_ERROR;
			print_usage = TRUE;
		} else {

			pEeprom = (EEPROMDATA_PTR) base_addr;
			printf("\n***********Didi_DiAnTable**********\n");
			

			for (i = 0; i < DIDIDIANTABLESIZE; i++) {
				test = pEeprom->DiDi_DianTable[i];

				printf("%02X ", test);
				if (i % 10 == 0) {
					printf("\n");
				}
			}

			printf("*************************************\n");
			
		}
	}

	if (print_usage) {
		if (shorthelp) {
			printf("%s \n", argv[0]);
		} else {
			printf("Usage: %s \n", argv[0]);
		}
	}
	return return_code;
}

extern DIANDIDITABLE_PTR pDiAnDiDiTable;

int_32 Shell_rdidi_diantable(int_32 argc, char_ptr argv[])
{
	boolean print_usage, shorthelp = FALSE;
	int_32 return_code = SHELL_EXIT_SUCCESS;
	
	FILE_PTR		  fd_ptr;
	char temp[100];

	unsigned char i, j;
	unsigned short offset;

	print_usage = Shell_check_help_request(argc, argv, &shorthelp);

	if (!print_usage) {
		if (argc > 1) {
			printf("Error, invalid number of parameters\n");
			return_code = SHELL_EXIT_ERROR;
			print_usage = TRUE;
		} else {
		
		
			fd_ptr = fopen("d:\\dicorconfig\\bak_didi_diantable.txt", "w+");
			if (fd_ptr)
			{	
				sprintf(temp, "#请按标准格式书写DiDi_DiAn关系路由表\r\n");
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "#必须以0x开头，以逗号结尾，数据必须由2个字节组成， 不足两位以0补全\r\n");
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "#不能随意增减空格回车等空白字符，可以通过Ctrl+A全选看出多余的空白字符\r\n");
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "#必须保留开头和结尾标识\r\n");
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "#DiAn必须带头和结束标识，DiAn后一行代表此DiAn带的DiDi数\r\n");
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "#最开头的一个字节为DiAn的个数，增加删减DiAn请注意改变它的值\r\n\r\n");
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "[BEGIN]\r\n");
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "0x%02X,\t#DiAn个数\r\n", pDiAnDiDiTable->diancount[1]);
				fwrite(temp, strlen(temp), 1, fd_ptr);

				for (i=0; i<pDiAnDiDiTable->diancount[1]; i++)
				{
					sprintf(temp, "[DIAN]0x%02X,0x%02X,[DIANEND]\r\n", pDiAnDiDiTable->indextab[i].dianaddr[0], pDiAnDiDiTable->indextab[i].dianaddr[1]);
					fwrite(temp, strlen(temp), 1, fd_ptr);
					sprintf(temp, "0x%02X,\t#DiDi个数\r\n", pDiAnDiDiTable->indextab[i].count[1]);
					fwrite(temp, strlen(temp), 1, fd_ptr);
					offset = pDiAnDiDiTable->indextab[i].offset[0];
					offset <<= 8;
					offset += pDiAnDiDiTable->indextab[i].offset[1];
					for (j=0; j<pDiAnDiDiTable->indextab[i].count[1]; j++)
					{
						sprintf(temp, "0x%02X,0x%02X,\r\n", pDiAnDiDiTable->didiaddr[offset+j*2], pDiAnDiDiTable->didiaddr[offset+j*2+1]);
						fwrite(temp, strlen(temp), 1, fd_ptr);
					}
				}
			
				sprintf(temp, "[END]");
				fwrite(temp, strlen(temp), 1, fd_ptr);
				fclose(fd_ptr);
			}
			else
			{
				printf("open file err!\n");
			}
		}
	}

	if (print_usage) {
		if (shorthelp) {
			printf("%s \n", argv[0]);
		} else {
			printf("Usage: %s \n", argv[0]);
		}
	}
	return return_code;
}



int_32 Shell_wroutertable(int_32 argc, char_ptr argv[])
{


	boolean print_usage, shorthelp = FALSE;
	int_32 return_code = SHELL_EXIT_SUCCESS;

	char s[100];
	uchar i;
	uint_32 j;
	FILE_PTR fd_ptr;
	char hex[3];
	//unsigned char writebuf[WRITEBUFSIZE];
		unsigned char *writebuf;
	//测试申请外部内存
	writebuf = (unsigned char *) _mem_alloc_zero_from(_user_pool_id, 512);
    if (writebuf == NULL)
    {
    	printf("error when mem alloc\r\n");	
    }

	hex[2] = '\0';

	print_usage = Shell_check_help_request(argc, argv, &shorthelp);

	if (!print_usage) {
		if (argc > 1) {
			printf("Error, invalid number of parameters\n");
			return_code = SHELL_EXIT_ERROR;
			print_usage = TRUE;
		} else {

			//先打开配置文件
			fd_ptr =
			    fopen("d:\\dicorconfig\\routertable.txt", "r");
			if (fd_ptr == NULL) {
				printf
				    ("The Config File is not exitting\n");
				return_code = SHELL_EXIT_ERROR;
				print_usage = TRUE;
			}
			printf("\n*******************************\n");
			printf("解析文件 routertable.txt\n");
			while (strcmp(s, "[BEGIN]") != 0) {
				fgetline(fd_ptr, s, 64);	//[BEGIN]
				if (feof(fd_ptr))
				{
					printf("\n");
					printf("没有找到文件头[BEGIN]，请用ls,dir,type等命令检查文件是否正确！\n");
					fclose(fd_ptr);
					return_code = SHELL_EXIT_ERROR;
					print_usage = TRUE;
					return return_code;
				}
			}
			
			while (strcmp(s, "[UPTAB]") != 0) {
				fgetline(fd_ptr, s, 64);	//[UPTAB]
				if (feof(fd_ptr))
				{
					printf("\n");
					printf("没有找到[UPTAB]，请用ls,dir,type等命令检查文件书写格式是否正确！\n");
					fclose(fd_ptr);
					return_code = SHELL_EXIT_ERROR;
					print_usage = TRUE;
					return return_code;
				}
			}
			
			fgetline(fd_ptr, s, 64);	//
			fgetline(fd_ptr, s, 64);
			printf("%s\n", s);
	
			hex[0] = s[2];
			hex[1] = s[3];
			strupr(hex);
			writebuf[0] = ahextoi(hex);
			hex[0] = s[7];
			hex[1] = s[8];
			strupr(hex);
			writebuf[1] = ahextoi(hex);	
			
			while (strcmp(s, "[DOWNTAB]") != 0) {
				fgetline(fd_ptr, s, 64);	//[DOWNTAB]
				if (feof(fd_ptr))
				{
					printf("\n");
					printf("没有找到[DOWNTAB]，请用ls,dir,type等命令检查文件书写格式是否正确！\n");
					fclose(fd_ptr);
					return_code = SHELL_EXIT_ERROR;
					print_usage = TRUE;
					return return_code;
				}
			}
			
			fgetline(fd_ptr, s, 64);	//
			fgetline(fd_ptr, s, 64);
			printf("%s\n", s);
			hex[0] = s[2];
			hex[1] = s[3];
			strupr(hex);
			writebuf[2] = ahextoi(hex);	

			j = 3;
			while (strcmp(s, "[ENDDOWNTAB]") != 0) {
				fgetline(fd_ptr, s, 64);	//
				fgetline(fd_ptr, s, 64);
				printf("%s\n", s);
				i = 0;
				while (s[i * 5] != '\0') {
					hex[0] = s[i * 5 + 2];
					hex[1] = s[i * 5 + 3];
					i++;
					strupr(hex);
					writebuf[j++] = ahextoi(hex);

				}
			}
			
			while (strcmp(s, "[DEPTHTAB]") != 0) {
				fgetline(fd_ptr, s, 64);	//[DEPTHTAB]
				if (feof(fd_ptr))
				{
					printf("\n");
					printf("没有找到[DEPTHTAB]，请用ls,dir,type等命令检查文件书写格式是否正确！\n");
					fclose(fd_ptr);
					return_code = SHELL_EXIT_ERROR;
					print_usage = TRUE;
					return return_code;
				}
			}
			
			j = 256;//后256个字节存放深度表
			while (strcmp(s, "[ENDDEPTHTAB]") != 0) {
				fgetline(fd_ptr, s, 64);	//
				fgetline(fd_ptr, s, 64);
				printf("%s\n", s);
				i = 0;
				while (s[i * 5] != '\0') {
					hex[0] = s[i * 5 + 2];
					hex[1] = s[i * 5 + 3];
					i++;
					strupr(hex);
					writebuf[j++] = ahextoi(hex);

				}
			}
		}

		//写入FLASH中
		EepromWrite(writebuf,BASECONFIGSIZE, WRITEBUFSIZE);
			
		fclose(fd_ptr);
	}

	if (print_usage) {
		if (shorthelp) {
			printf("%s \n", argv[0]);
		} else {
			printf("Usage: %s \n", argv[0]);
		}
	}
	if (writebuf != NULL)
	{
		_mem_free(writebuf);
	}
	return return_code;
}


int_32 Shell_proutertable(int_32 argc, char_ptr argv[])
{
	boolean print_usage, shorthelp = FALSE;
	int_32 return_code = SHELL_EXIT_SUCCESS;
	unsigned int i;
	unsigned char test;

	print_usage = Shell_check_help_request(argc, argv, &shorthelp);

	if (!print_usage) {
		if (argc > 1) {
			printf("Error, invalid number of parameters\n");
			return_code = SHELL_EXIT_ERROR;
			print_usage = TRUE;
		} else {

			pEeprom = (EEPROMDATA_PTR) base_addr;
			printf("\n*************RouterTable************\n");
			printf("[BEGIN]\n");
			printf("[UPTAB]\n");
			printf("0x%02X,0x%02X,\n", pEeprom->RouterTable[0],pEeprom->RouterTable[1]);
			printf("[ENDUPTAB]\n");
			
			printf("[DOWNTAB]\n");
			printf("0x%02X\n", pEeprom->RouterTable[2]);
			
			for (i = 3; i < ROUTERTABLESIZE/2; i++) {
				test = pEeprom->RouterTable[i];

				printf("%02X ", test);
				if ((i+1) % 3 == 0) {
					printf("\n");
				}
			}
			
			printf("[ENDDOWNTAB]\n");
			printf("[DEPTHTAB]\n");
			for (i = ROUTERTABLESIZE/2; i < ROUTERTABLESIZE; i++) {
				test = pEeprom->RouterTable[i];

				printf("%02X ", test);
				if ((i+1) % 3 == 0) {
					printf("\n");
				}
			}
			
			
			printf("[ENDDEPTHTAB]\n");
			
			printf("[END]\n");
			printf("*************************************\n");
			
		}
	}

	if (print_usage) {
		if (shorthelp) {
			printf("%s \n", argv[0]);
		} else {
			printf("Usage: %s \n", argv[0]);
		}
	}
	return return_code;
}


int_32 Shell_rroutertable(int_32 argc, char_ptr argv[])
{
	boolean print_usage, shorthelp = FALSE;
	int_32 return_code = SHELL_EXIT_SUCCESS;
	
	FILE_PTR		  fd_ptr;
	char temp[100];

	unsigned char test;
	unsigned int i;

	print_usage = Shell_check_help_request(argc, argv, &shorthelp);

	if (!print_usage) {
		if (argc > 1) {
			printf("Error, invalid number of parameters\n");
			return_code = SHELL_EXIT_ERROR;
			print_usage = TRUE;
		} else {
		
		
			fd_ptr = fopen("d:\\dicorconfig\\bak_routertable.txt", "w+");
			if (fd_ptr)
			{	
				sprintf(temp, "#请按标准格式书写路由表\r\n");
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "#必须以0x开头，以逗号结尾，数据必须由2个字节组成， 不足两位以0补全\r\n");
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "#不能随意增减空格回车等空白字符，可以通过Ctrl+A全选看出多余的空白字符\r\n");
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "#必须保留开头和结尾标识\r\n");
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "\r\n[BEGIN]\r\n\r\n");
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "#上行路由表，两个，前一个有用，后一个备用\r\n");
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "#请以逗号结尾\r\n");
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "[UPTAB]\r\n");
				fwrite(temp, strlen(temp), 1, fd_ptr);
				
				pEeprom = (EEPROMDATA_PTR) base_addr;
				sprintf(temp, "0x%02X,0x%02X,\r\n", pEeprom->RouterTable[0], pEeprom->RouterTable[1]);
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "[ENDUPTAB]\r\n\r\n");
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "#下行路由表，开始是下行路由个数，请以逗号结尾\r\n");
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "[DOWNTAB]\r\n");
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "0x%02X,\r\n", pEeprom->RouterTable[2]);
				fwrite(temp, strlen(temp), 1, fd_ptr);
				
			
				test = pEeprom->RouterTable[2];
	
				for (i=3; i<test*3+3; i++)
				{
					sprintf(temp, "0x%02X,", pEeprom->RouterTable[i]);
					fwrite(temp, strlen(temp), 1, fd_ptr);
					if ((i+1) % 3 == 0) 
					{
						sprintf(temp, "\r\n");
						fwrite(temp, strlen(temp), 1, fd_ptr);
					}
				}
				
				sprintf(temp, "[ENDDOWNTAB]\r\n\r\n");
				fwrite(temp, strlen(temp), 1, fd_ptr);	
				
				
				sprintf(temp, "#深度表\r\n");
				fwrite(temp, strlen(temp), 1, fd_ptr);
				sprintf(temp, "[DEPTHTAB]\r\n");
				fwrite(temp, strlen(temp), 1, fd_ptr);
				
				test = pBaseConfig->subnet_num;
				for (i=ROUTERTABLESIZE/2; i<test*3+ROUTERTABLESIZE/2; i++)
				{
					sprintf(temp, "0x%02X,", pEeprom->RouterTable[i]);
					fwrite(temp, strlen(temp), 1, fd_ptr);
					if ((i+1) % 3 == 0) 
					{
						sprintf(temp, "\r\n");
						fwrite(temp, strlen(temp), 1, fd_ptr);
					}
				}
				
				sprintf(temp, "[ENDDEPTHTAB]\r\n");
				fwrite(temp, strlen(temp), 1, fd_ptr);
				
				sprintf(temp, "[END]");
				fwrite(temp, strlen(temp), 1, fd_ptr);
				fclose(fd_ptr);
			}
			else
			{
				printf("open file err!\n");
			}
		}
	}

	if (print_usage) {
		if (shorthelp) {
			printf("%s \n", argv[0]);
		} else {
			printf("Usage: %s \n", argv[0]);
		}
	}
	return return_code;
}



int_32 Shell_reboot(int_32 argc, char_ptr argv[])
{
	boolean print_usage, shorthelp = FALSE;
	int_32 return_code = SHELL_EXIT_SUCCESS;
	MCF5225_CCM_STRUCT_PTR ccr_reg_addr;

	print_usage = Shell_check_help_request(argc, argv, &shorthelp);

	if (!print_usage) {
		if (argc > 1) {
			printf("Error, invalid number of parameters\n");
			return_code = SHELL_EXIT_ERROR;
			print_usage = TRUE;
		} else {
			extern void dicor_close_socket(void);
			dicor_close_socket();
			ccr_reg_addr = (MCF5225_CCM_STRUCT_PTR) (& ((VMCF5225_STRUCT_PTR) _PSP_GET_IPSBAR())->CCM);
			ccr_reg_addr->RCR |= 0x80;
		}
	}

	if (print_usage) {
		if (shorthelp) {
			printf("%s \n", argv[0]);
		} else {
			printf("Usage: %s \n", argv[0]);
		}
	}
	return return_code;
}

int_32 Shell_testwatchdog(int_32 argc, char_ptr argv[])
{
	boolean print_usage, shorthelp = FALSE;
	int_32 return_code = SHELL_EXIT_SUCCESS;

	print_usage = Shell_check_help_request(argc, argv, &shorthelp);

	if (!print_usage) {
		if (argc > 1) {
			printf("Error, invalid number of parameters\n");
			return_code = SHELL_EXIT_ERROR;
			print_usage = TRUE;
		} else {
			//死循环测试看门狗能否复位
			while(1);
		}
	}

	if (print_usage) {
		if (shorthelp) {
			printf("%s \n", argv[0]);
		} else {
			printf("Usage: %s \n", argv[0]);
		}
	}
	return return_code;
}



/*FUNCTION*-------------------------------------------------------------
*
*  Function Name :  Shell_Kill
*  Returned Value:  none
*  Comments  :  SHELL utility to TFTP to or from a host
*  Usage:  tftp host get source [destination] [mode] 
*
*END*-----------------------------------------------------------------*/

int_32  Shell_kill(int_32 argc, char_ptr argv[] )
{
   _task_id task_id;
   uint_32  result;
   boolean  print_usage, shorthelp = FALSE;
   int_32   return_code = SHELL_EXIT_SUCCESS;

   print_usage = Shell_check_help_request(argc, argv, &shorthelp );

   if (!print_usage)  {
      if (argc == 2)  {
         task_id = _task_get_id_from_name( argv[1] );
         if (task_id == MQX_NULL_TASK_ID)  {
            printf("No task named %s running.\n",argv[1]);
            return_code = SHELL_EXIT_ERROR;
         } else  {
            result = _task_destroy(task_id);
            if (result == MQX_OK)  {  
               printf("Task %s killed.\n",argv[1]);
            } else  {
               printf("Unable to kill task %s.\n",argv[1]);
               return_code = SHELL_EXIT_ERROR;
            }
         }
      } else  {
         printf("Error, %s invoked with incorrect number of arguments\n", argv[0]);
         print_usage = TRUE;
      }
   }

   if (print_usage)  {
      if (shorthelp)  {
         printf("%s <taskid>\n", argv[0]);
      } else  {
         printf("Usage: %s <taskname>\n", argv[0]);
         printf("   <taskname> = MQX Task name\n");
      }
   }
   return return_code;
} /* Endbody */

   

#if SHELLCFG_USES_RTCS

#include <rtcs.h>
#include "sh_rtcs.h"
        
         
/*FUNCTION*-------------------------------------------------------------
*
*  Function Name :  Shell_TFTP_client
*  Returned Value:  none
*  Comments  :  SHELL utility to TFTP to or from a host
*  Usage:  tftp host get source [destination] [mode] 
*
*END*-----------------------------------------------------------------*/

int_32  Shell_TFTP_client(int_32 argc, char_ptr argv[] )
{
   _ip_address          hostaddr;
   char                 hostname[MAX_HOSTNAMESIZE];
   char_ptr             file_ptr;
   uint_32              tftp_handle, buffer_size/*,byte_number*/;
   uchar_ptr            buffer_ptr;
   TFTP_DATA_STRUCT     tftp_data;
   MQX_FILE_PTR         fd;
   int_32               error;
   boolean              print_usage, shorthelp = FALSE;
   int_32               return_code = SHELL_EXIT_SUCCESS;
   char                 path[SHELL_MAX_FILELEN];  
   boolean              trans = FALSE;
printf("\nfile=%s,func=%s,line=%d\n",__FILE__,__FUNCTION__,__LINE__);	
   print_usage = Shell_check_help_request(argc, argv, &shorthelp );
printf("\nfile=%s,func=%s,line=%d\n",__FILE__,__FUNCTION__,__LINE__);	
   if (!print_usage)  {
   
      if ((argc >= 3) && (argc <= 5))  {
         RTCS_resolve_ip_address( argv[1], &hostaddr, hostname, MAX_HOSTNAMESIZE ); 

         if (!hostaddr)  {
            printf("Unable to resolve host.\n");
            return_code = SHELL_EXIT_ERROR;
         } else  {
            tftp_data.SERVER   = hostaddr;
            tftp_data.FILENAME = argv[2];
            tftp_data.FILEMODE = "netascii";
            if (argc > 3)  {
               file_ptr = argv[3];
               if (argc > 4) {
                  tftp_data.FILEMODE = argv[4];
               } else {
                  tftp_data.FILEMODE = "netascii";
               }
            } else {
               file_ptr = argv[2];
            }
#if SHELLCFG_USES_MFS  
printf("\nfile=%s,func=%s,line=%d\n",__FILE__,__FUNCTION__,__LINE__);	     
            Shell_create_prefixed_filename(path, argv[3], argv);
            fd = fopen(path,"a");
            if (fd)  {
               printf("\nDownloading file %s from TFTP server: %s [%ld.%ld.%ld.%ld]\n",
                  tftp_data.FILENAME,hostname, IPBYTES(hostaddr));
               tftp_handle = (*FT_TFTP->OPEN)( (pointer) &tftp_data );
               if ( tftp_handle != RTCS_OK )  {
                  printf("\nError opening file %s\n",tftp_data.FILENAME);
                  return_code = SHELL_EXIT_ERROR;
               } else  {
                while (! (*FT_TFTP->EOFT)())  {
                   do {
                     buffer_ptr = (*FT_TFTP->READ)( &buffer_size );
                     if (buffer_ptr != NULL)  {
                         fseek(fd, 0 , IO_SEEK_CUR);
                        _io_write(fd,buffer_ptr,buffer_size); 
                        trans = TRUE;
                     } else {
printf("\nfile=%s,func=%s,line=%d\n",__FILE__,__FUNCTION__,__LINE__);	
                         switch (buffer_size) {
                         case (RTCSERR_TFTP_ERROR + 1):
                            printf("\nFile %s not found\n", tftp_data.FILENAME);
                            break;
                         case (RTCSERR_TFTP_ERROR + 2):
                            printf("\nAccess violation\n");
                            break;
                         case (RTCSERR_TFTP_ERROR + 3):
                            printf("\nDisk full or allocation exceeded\n");
                            break;
                         case (RTCSERR_TFTP_ERROR + 4):
                            printf("\nIllegal TFTP operation\n");
                            break;
                         case (RTCSERR_TFTP_ERROR + 5):
                            printf("\nUnknown transfer ID\n");
                            break;
                         case (RTCSERR_TFTP_ERROR + 6):
                            printf("\nFile already exists\n");
                            break;
                         case (RTCSERR_TFTP_ERROR + 7):
                            printf("\nNo such user\n");
                            break;
                         default:
                            if(trans) 
                            {
                              trans =FALSE;
                              break;
                            }
                          else
                            printf("\nError reading file %s\n", tftp_data.FILENAME);
                         } /* Endswitch */
                        }
                     } while(buffer_ptr !=NULL);
printf("\nfile=%s,func=%s,line=%d\n",__FILE__,__FUNCTION__,__LINE__);	                    
                        fclose(fd);
                  }
                  
                  error = (*FT_TFTP->CLOSE)();
                  
               }
               
            } else  {
               printf("\nError opening local file %s\n",file_ptr);
               return_code = SHELL_EXIT_ERROR;
            }
#else
            tftp_handle = (*FT_TFTP->OPEN)( (pointer) &tftp_data );
            if ( tftp_handle != RTCS_OK )  {
               printf("\nError opening file %s\n",tftp_data.FILENAME);
               return_code = SHELL_EXIT_ERROR;
            } else  {
               printf("SHELLCFG_USES_MFS is not set to 1 in user_config.h - file wont be written to disk\n");
            }
            error = (*FT_TFTP->CLOSE)();
#endif            
         }
      } else  {
         printf("Error, %s invoked with incorrect number of arguments\n", argv[0]);
         print_usage = TRUE;
      }
   }
   
   if (print_usage)  {
      if (shorthelp)  {
         printf("%s <host> <source> [<dest>] [<mode>]\n", argv[0]);
      } else  {
         printf("Usage: %s <host> <source> [<dest>] [<mode>]\n", argv[0]);
         printf("   <host>   = host ip address or name\n");
         printf("   <source> = remote file name\n");
         printf("   <dest>   = local file name\n");
         printf("   <mode>   = file transfer mode (netascii, etc.)\n");
      }
   }
   return return_code;
} /* Endbody */

#endif /* SHELLCFG_USES_RTCS */




#define SHELL_TFTPD_PRIO        7
#define SHELL_TFTPD_STACK       2560

boolean TFTPSRV_access (char_ptr filename, uint_16 op) {return TRUE;}

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name :  Shell_TFTPd
*  Returned Value:  none
*  Comments  :  SHELL utility to TFTP to or from a host
*  Usage:  tftp host get source [destination] [mode] 
*
*END*-----------------------------------------------------------------*/

int_32  Shell_TFTPd(int_32 argc, char_ptr argv[] )
{
   uint_32  result;
   boolean  print_usage, shorthelp = FALSE;
   int_32   return_code = SHELL_EXIT_SUCCESS;

   print_usage = Shell_check_help_request(argc, argv, &shorthelp );

   if (!print_usage)  {
      if (argc == 2)  {
         if (strcmp(argv[1], "start") == 0)  {
         
            result = TFTPSRV_init("TFTP_server", SHELL_TFTPD_PRIO, SHELL_TFTPD_STACK );
            if (result ==  0)  {
               printf("TFTP Server Started.\n");
            } else  {
               printf("Unable to start TFTP Server, error = 0x%x\n",result);
               return_code = SHELL_EXIT_ERROR;
            }
         } else if (strcmp(argv[1], "stop") == 0)  {
            result = TFTPSRV_stop();
            if (result ==  0)  {
               printf("TFTP Server Stopped.\n");
            } else  {
               printf("Unable to stop TFTP Server, error = 0x%x\n",result);
               return_code = SHELL_EXIT_ERROR;
            }
         } else  {
         printf("Error, %s invoked with incorrect option\n", argv[0]);
            print_usage = TRUE;
         }
      } else  {
         printf("Error, %s invoked with incorrect number of arguments\n", argv[0]);
         print_usage = TRUE;
      }
   }
   
   if (print_usage)  {
      if (shorthelp)  {
         printf("%s [start|stop]\n", argv[0]);
      } else  {
         printf("Usage: %s [start|stop]\n",argv[0]);
      }
   }
   return return_code;
} /* Endbody */


extern void EMN_APL_SetParameter(UINT_8 subnet,UINT_8 dev,ParaType *para);
extern void EMN_APL_GetParameter(UINT_8 subnet,UINT_8 dev,GetParaType *para);
extern GetParaType GetParaData;
extern ParaType SetParameterData;


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name :  
*  Returned Value:  none
*  Comments  :  
*  Usage:  
*
*END*-----------------------------------------------------------------*/

int_32  Shell_UpdateDiDi(int_32 argc, char_ptr argv[] )
{
   boolean  print_usage, shorthelp = FALSE;
   int_32   return_code = SHELL_EXIT_SUCCESS;
   uint_8	sub, dev;
   char hex[3];


   UINT_16	i;


   hex[2] = '\0';

   print_usage = Shell_check_help_request(argc, argv, &shorthelp );

   if (!print_usage)  {
      if (argc == 3)  {
         //子网地址，
         hex[0] = argv[1][2];
		 hex[1] = argv[1][3];
		 strupr(hex);
		 sub = ahextoi(hex);
		 printf("*********************************\n");
		 printf("DIDI subaddr=%02X\t", sub);
		 printf("update didi program\n");
         //设备地址
         hex[0] = argv[2][2];
		 hex[1] = argv[2][3];
		 strupr(hex);
		 dev = ahextoi(hex);
		 
	
		 SetParameterData.brd=0x03;
		 SetParameterData.index=56;
 	     SetParameterData.len=14;
 		 SetParameterData.dev=dev;
 	
		 
		 EMN_APL_SetParameter(sub, dev, &SetParameterData);
		 i = 0;
		 while (i < 1600)
		 {
		 	_time_delay(10);
			if (SetParameterData.flag)
			{
				break;
			}
			i++;
		 }
		 if(i != 1600)
		 	{
		 	  printf("i = %d\n",i*10);
		 	}
		 else
		 	{
		 	  printf("timeout update didi program\n");
		 	}
	
		 printf("*********************************\n");
         
      } else  {
      
         printf("Error, %s invoked with incorrect number of arguments\n", argv[0]);
         print_usage = TRUE;
      }
   }

   if (print_usage)  {
      if (shorthelp)  {
         printf("%s <0xsubnet> <0xdevaddr>\n", argv[0]);
      } else  {
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

int_32  Shell_GetDiDiVersion(int_32 argc, char_ptr argv[] )
{
   boolean  print_usage, shorthelp = FALSE;
   int_32   return_code = SHELL_EXIT_SUCCESS;
   uint_8	sub, dev;
   char hex[3];
   UINT_16	i;
   hex[2] = '\0';

   print_usage = Shell_check_help_request(argc, argv, &shorthelp );

   if (!print_usage)  {
      if (argc == 3)  {
         //子网地址，
         hex[0] = argv[1][2];
		 hex[1] = argv[1][3];
		 strupr(hex);
		 sub = ahextoi(hex);
		 printf("*****************************\n");
		 printf("DiDi subaddr=%02X  ", sub);
		 
         //设备地址
         hex[0] = argv[2][2];
		 hex[1] = argv[2][3];
		 strupr(hex);
		 dev = ahextoi(hex);
		 printf("devaddr=%02X\n", dev);
		 //ParaData0.brd=0x00;
		 GetParaData.index=0;
 	     GetParaData.len=20;
 		 GetParaData.dev=dev;
	     GetParaData.data[0]=0x01;//表示要从内存中取版本号
        
		 EMN_APL_GetParameter(sub, dev, &GetParaData);
		 i = 0;
		 while (i < 400)
		 {
		 	_time_delay(10);
			if (GetParaData.flag)
			{
				break;
			}
			i++;
		 }
		 if(i != 400)
		 {
		 	  printf("\nGet Time = %d ms\n",i*10);
		 	  printf("[功率程序软件版本号] = %s\n", &GetParaData.data[0]);
		 	  printf("[通讯程序软件版本号] = %s\n", &GetParaData.data[13]);
		 }
		 else
		 	{
		 	  printf("timeout get parameter\n");
		 	}
         
      } else  {
      
         printf("Error, %s invoked with incorrect number of arguments\n", argv[0]);
         print_usage = TRUE;
      }
   }

   if (print_usage)  {
      if (shorthelp)  {
         printf("%s <0xsubnet> <0xdevaddr>\n", argv[0]);
      } else  {
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

int_32  Shell_GetDiDiPt(int_32 argc, char_ptr argv[] )
{
   boolean  print_usage, shorthelp = FALSE;
   int_32   return_code = SHELL_EXIT_SUCCESS;
   uint_8	sub, dev;
   char hex[3];
   uint_32  temp, temp2;
   UINT_16	i;
   hex[2] = '\0';

   print_usage = Shell_check_help_request(argc, argv, &shorthelp );

   if (!print_usage)  {
      if (argc == 3)  {
         //子网地址，
         hex[0] = argv[1][2];
		 hex[1] = argv[1][3];
		 strupr(hex);
		 sub = ahextoi(hex);
		 printf("*****************************\n");
		 printf("DiDi subaddr=%02X  ", sub);
		 
         //设备地址
         hex[0] = argv[2][2];
		 hex[1] = argv[2][3];
		 strupr(hex);
		 dev = ahextoi(hex);
		 printf("devaddr=%02X\n", dev);
		 //ParaData0.brd=0x00;
		 GetParaData.index=0;
 	     GetParaData.len=20;
 		 GetParaData.dev=dev;
	
	
        
		 EMN_APL_GetParameter(sub, dev, &GetParaData);
		 i = 0;
		 while (i < 400)
		 {
		 	_time_delay(10);
			if (GetParaData.flag)
			{
				break;
			}
			i++;
		 }
		 if(i != 400)
		 {
		 	  printf("\nGet Time = %d ms\n",i*10);
		 	  printf("[软件版本号] = %c%d.%d.%d\n", GetParaData.data[0], GetParaData.data[1], GetParaData.data[2], GetParaData.data[3]);
		 	  printf("[硬件版本号] = %c%d.%d.%d\n", GetParaData.data[4], GetParaData.data[5], GetParaData.data[6], GetParaData.data[7]);
		 	  printf("[上行地址] = 0x%02X\n", GetParaData.data[8]);
		 	  if (GetParaData.data[9])
		 	  {
		 	  	printf("[设备类型] = 子设备\n");
		 	  }
		 	  else
		 	  {
		 	  	printf("[设备类型] = 簇头设备\n");
		 	  }
		 	  printf("[网络地址] = 0x%02X\n", GetParaData.data[10]);
		 	  printf("[子网地址] = 0x%02X\n", GetParaData.data[11]);
		 	  printf("[设备地址] = 0x%02X\n", GetParaData.data[12]);
		 	  printf("[子设备数] = %d\n", GetParaData.data[13]);
		 	  printf("[UID] = %02X-%02X-%02X-%02X\n", GetParaData.data[15], GetParaData.data[16], GetParaData.data[17], GetParaData.data[18]);
		 	  
		 }
		 else
		 	{
		 	  printf("timeout get parameter\n");
		 	}
		 
		 
		 GetParaData.index=20;
 	     GetParaData.len=20;
 		 GetParaData.dev=dev;
		 EMN_APL_GetParameter(sub, dev, &GetParaData);
		 i = 0;
		 while (i < 400)
		 {
		 	_time_delay(10);
			if (GetParaData.flag)
			{
				break;
			}
			i++;
		 }
		 if(i != 400)
		 {
		 	  printf("\nGet Time = %d ms\n",i*10);
		 	 
		 	  printf("[下行地址1] = %02X:%02X:%02X\n", GetParaData.data[1], GetParaData.data[2], GetParaData.data[3]);
		 	  printf("[下行地址2] = %02X:%02X:%02X\n", GetParaData.data[5], GetParaData.data[6], GetParaData.data[7]);
		 	  printf("[下行地址3] = %02X:%02X:%02X\n", GetParaData.data[9], GetParaData.data[10], GetParaData.data[11]);
		 	  printf("[下行地址4] = %02X:%02X:%02X\n", GetParaData.data[13], GetParaData.data[14], GetParaData.data[15]);
		 	  printf("[下行地址5] = %02X:%02X:%02X\n", GetParaData.data[17], GetParaData.data[18], GetParaData.data[19]);
		 }
		 else
		 	{
		 	  printf("timeout get parameter\n");
		 	}
		 	
		 	GetParaData.index=40;
 	     GetParaData.len=12;
 		 GetParaData.dev=dev;
		 EMN_APL_GetParameter(sub, dev, &GetParaData);
		 i = 0;
		 while (i < 400)
		 {
		 	_time_delay(10);
			if (GetParaData.flag)
			{
				break;
			}
			i++;
		 }
		 if(i != 400)
		 {
		 	  printf("\nGet Time = %d ms\n",i*10);
		 	 
		 	  printf("[下行地址6] = %02X:%02X:%02X\n", GetParaData.data[1], GetParaData.data[2], GetParaData.data[3]);
		 	  printf("[下行地址7] = %02X:%02X:%02X\n", GetParaData.data[5], GetParaData.data[6], GetParaData.data[7]);
		 	  printf("[下行地址8] = %02X:%02X:%02X\n", GetParaData.data[9], GetParaData.data[10], GetParaData.data[11]);
		 }
		 else
		 	{
		 	  printf("timeout get parameter\n");
		 	}
		 	
		 	
		 	GetParaData.index=56;
 	     GetParaData.len=20;
 		 GetParaData.dev=dev;
		 EMN_APL_GetParameter(sub, dev, &GetParaData);
		 i = 0;
		 while (i < 400)
		 {
		 	_time_delay(10);
			if (GetParaData.flag)
			{
				break;
			}
			i++;
		 }
		 if(i != 400)
		 {
		 	  printf("\nGet Time = %d ms\n",i*10);
		 	 
		 	  temp = GetParaData.data[1];
		 	  temp <<= 8;
		 	  temp += GetParaData.data[0];
		 	  temp2 = (temp*62.6*10)/32768;
		 	  if ((temp2%10)>=5)
		 	  {
		 	  		temp2 = temp2/10+1;
		 	  }
		 	  else
		 	  {
		 	  		temp2 = temp2 / 10;
		 	  }
		 	  printf("[输出电压设置值] = %d ==> %d(V)\n",  temp, temp2);
		 	  temp = GetParaData.data[3];
		 	  temp <<= 8;
		 	  temp += GetParaData.data[2];
		 	  temp2 = (temp*23.6*10)/32768;
		 	  if ((temp2%10)>=5)
		 	  {
		 	  		temp2 = temp2/10+1;
		 	  }
		 	  else
		 	  {
		 	  		temp2 = temp2 / 10;
		 	  }
		 	  printf("[输出电流设置值] = %d ==> %d(A)\n",  temp, temp2);
		 	  temp = GetParaData.data[5];
		 	  temp <<= 8;
		 	  temp += GetParaData.data[4];
		 	  temp2 = (temp*23.6*10)/32768;
		 	  if ((temp2%10)>=5)
		 	  {
		 	  		temp2 = temp2/10+1;
		 	  }
		 	  else
		 	  {
		 	  		temp2 = temp2 / 10;
		 	  }
		 	  printf("[输入电流设置值] = %d ==> %d(A)\n",  temp, temp2);
		 	  temp = GetParaData.data[7];
		 	  temp <<= 8;
		 	  temp += GetParaData.data[6];
		 	  temp2 = (temp*23.6*62.6*10)/32768;
		 	  if ((temp2%10)>=5)
		 	  {
		 	  		temp2 = temp2/10+1;
		 	  }
		 	  else
		 	  {
		 	  		temp2 = temp2 / 10;
		 	  }
		 	  printf("[输出功率设置值] = %d ==> %d(W)\n",  temp, temp2);
		 	  temp = GetParaData.data[9];
		 	  temp <<= 8;
		 	  temp += GetParaData.data[8];
		 	  temp2 = (temp*62.6*10)/32768;
		 	  if ((temp2%10)>=5)
		 	  {
		 	  		temp2 = temp2/10+1;
		 	  }
		 	  else
		 	  {
		 	  		temp2 = temp2 / 10;
		 	  }
		 	  printf("[输出过压保护值] = %d ==> %d(V)\n",  temp, temp2);
		 	  temp = GetParaData.data[11];
		 	  temp <<= 8;
		 	  temp += GetParaData.data[10];
		 	  temp2 = (temp*62.6*10)/32768;
		 	  if ((temp2%10)>=5)
		 	  {
		 	  		temp2 = temp2/10+1;
		 	  }
		 	  else
		 	  {
		 	  		temp2 = temp2 / 10;
		 	  }
		 	  printf("[输入过压保护值] = %d ==> %d(V)\n",  temp, temp2);
		 	  temp = GetParaData.data[13];
		 	  temp <<= 8;
		 	  temp += GetParaData.data[12];
		 	  temp2 = (temp*23.6*10)/32768;
		 	  if ((temp2%10)>=5)
		 	  {
		 	  		temp2 = temp2/10+1;
		 	  }
		 	  else
		 	  {
		 	  		temp2 = temp2 / 10;
		 	  }
		 	  printf("[输入电流保护值] = %d ==> %d(A)\n",  temp, temp2);
		 	  temp = GetParaData.data[15];
		 	  temp <<= 8;
		 	  temp += GetParaData.data[14];
		 	  temp2 = (temp*23.6*10)/32768;
		 	  if ((temp2%10)>=5)
		 	  {
		 	  		temp2 = temp2/10+1;
		 	  }
		 	  else
		 	  {
		 	  		temp2 = temp2 / 10;
		 	  }
		 	  printf("[输出电流保护值] = %d ==> %d(A)\n",  temp, temp2);
		 	  temp = GetParaData.data[17];
		 	  temp <<= 8;
		 	  temp += GetParaData.data[16];
		 	  temp2 = (temp*23.6*62.6*10)/32768;
		 	  if ((temp2%10)>=5)
		 	  {
		 	  		temp2 = temp2/10+1;
		 	  }
		 	  else
		 	  {
		 	  		temp2 = temp2 / 10;
		 	  }
		 	  printf("[输出功率保护值] = %d ==> %d(W)\n",  temp, temp2);
		 	  temp = GetParaData.data[19];
		 	  temp <<= 8;
		 	  temp += GetParaData.data[18];
		 	  temp2 = (temp*62.6*10)/32768;
		 	  if ((temp2%10)>=5)
		 	  {
		 	  		temp2 = temp2/10+1;
		 	  }
		 	  else
		 	  {
		 	  		temp2 = temp2 / 10;
		 	  }
		 	  printf("[DiDi最低工作电压设置值] = %d ==> %d(V)\n",  temp, temp2);
		 }
		 else
		 	{
		 	  printf("timeout get parameter\n");
		 	}
		
		 printf("*****************************\n");
         
      } else  {
      
         printf("Error, %s invoked with incorrect number of arguments\n", argv[0]);
         print_usage = TRUE;
      }
   }

   if (print_usage)  {
      if (shorthelp)  {
         printf("%s <0xsubnet> <0xdevaddr>\n", argv[0]);
      } else  {
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

int_32  Shell_SetDiDiPt(int_32 argc, char_ptr argv[] )
{
   boolean  print_usage, shorthelp = FALSE;
   int_32   return_code = SHELL_EXIT_SUCCESS;
   uint_8	sub, dev;
   char hex[3];
   char filename[28];
   	FILE_PTR fd_ptr;
	char s[100];
	char t[20];
   UINT_16	i, k;
   UINT_32 temp;
   UINT_32 l;

   UINT_16 val[10];
   hex[2] = '\0';
   filename[27] = '\0';
   print_usage = Shell_check_help_request(argc, argv, &shorthelp );

   if (!print_usage)  {
      if (argc == 3)  {
         //子网地址，
         hex[0] = argv[1][2];
		 hex[1] = argv[1][3];
		 strupr(hex);
		 sub = ahextoi(hex);
		 printf("*********************************\n");
		 printf("DIDI subaddr=%02X\t", sub);
		 
         //设备地址
         hex[0] = argv[2][2];
		 hex[1] = argv[2][3];
		 strupr(hex);
		 dev = ahextoi(hex);
		 
		 //先取得文件名
	     sprintf(filename, "d:\\didiconfig\\didi_%02x_%02x.txt", sub, dev);
		 
		 //先打开配置文件
		 fd_ptr =fopen(filename, "r");
		if (fd_ptr == NULL) {
				printf("The Config File is not exitting\n");
				return_code = SHELL_EXIT_ERROR;
				print_usage = TRUE;
				return return_code;
			}


			while (strcmp(s, "[BEGIN]") != 0) {
				fgetline(fd_ptr, s, 64);	//[BEGIN]
				if (feof(fd_ptr))
				{
					printf("\n");
					printf("没有找到文件头[BEGIN]，请用ls,dir,type等命令检查文件是否正确！\n");
					fclose(fd_ptr);
					return_code = SHELL_EXIT_ERROR;
					print_usage = TRUE;
					return return_code;
				}
				//printf("%s\n", s);
			}

			for (k=0; k<10; k++)
			{
				fgetline(fd_ptr, s, 64);	//[Voltage_Out_MaxS_PS]
				fgetline(fd_ptr, s, 64);
				
				if (cutstr(s, t, '='))
				{
					printf("文本数据书写格式错误\n");
					return_code = SHELL_EXIT_ERROR;
					print_usage = TRUE;
					return return_code;
				}
				
	
				val[k] = atoi(t);
			}
			//setval = (temp/62.6)*32768;
			fclose(fd_ptr);
		 
		 temp = val[0];
		 l = (temp/62.6)*327680;
		 temp = l / 10;
		 if ((l%10) >= 5)
		 {
		 	temp++;
		 }
		 val[0] = temp;
		 
		 temp = val[1];
		 l = (temp/23.6)*327680;
		 temp = l / 10;
		 if ((l%10) >= 5)
		 {
		 	temp++;
		 }
		 val[1] = temp;
		 
		 temp = val[2];
		 l = (temp/23.6)*327680;
		 temp = l / 10;
		 if ((l%10) >= 5)
		 {
		 	temp++;
		 }
		 val[2] = temp;
		 
		 temp = val[3];
		 l = (temp/62.6/23.6)*327680;
		 temp = l / 10;
		 if ((l%10) >= 5)
		 {
		 	temp++;
		 }
		 val[3] = temp;
		 
		 temp = val[4];
		 l = (temp/62.6)*327680;
		 temp = l / 10;
		 if ((l%10) >= 5)
		 {
		 	temp++;
		 }
		 val[4] = temp;
		 
		 temp = val[5];
		 l = (temp/62.6)*327680;
		 temp = l / 10;
		 if ((l%10) >= 5)
		 {
		 	temp++;
		 }
		 val[5] = temp;
		 
		 temp = val[6];
		 l = (temp/23.6)*327680;
		 temp = l / 10;
		 if ((l%10) >= 5)
		 {
		 	temp++;
		 }
		 val[6] = temp;
		 
		 temp = val[7];
		 l = (temp/23.6)*327680;
		 temp = l / 10;
		 if ((l%10) >= 5)
		 {
		 	temp++;
		 }
		 val[7] = temp;
		 
		 temp = val[8];
		 l = (temp/62.6/23.6)*327680;
		 temp = l / 10;
		 if ((l%10) >= 5)
		 {
		 	temp++;
		 }
		 val[8] = temp;
		 
		 temp = val[9];
		 l = (temp/62.6)*327680;
		 temp = l / 10;
		 if ((l%10) >= 5)
		 {
		 	temp++;
		 }
		 val[9] = temp;
		 printf("devaddr=%02X\n", dev);
		 SetParameterData.brd=0x00;
		 SetParameterData.index=56;
 	     SetParameterData.len=14;
 		 SetParameterData.dev=dev;
 		 for (i=0; i<7; i++)
 		 {
 		 	SetParameterData.data[2*i+1]=(uint_8)(val[i]>>8);
 		 	SetParameterData.data[2*i]=(uint_8)val[i];
 		 }
		 
		 EMN_APL_SetParameter(sub, dev, &SetParameterData);
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
		 
		 SetParameterData.brd=0x00;
		 SetParameterData.index=70;
 	     SetParameterData.len=6;
 		 SetParameterData.dev=dev;
 		 for (i=0; i<3; i++)
 		 {
 		 	SetParameterData.data[2*i+1]=(uint_8)(val[i+7]>>8);
 		 	SetParameterData.data[2*i]=(uint_8)val[i+7];
 		 }
		 EMN_APL_SetParameter(sub, dev, &SetParameterData);
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
		 	  printf("参数设置成功\n");
		 	}
		 else
		 	{
		 	  printf("timeout set parameter\n");
		 	}
	
		 
		 printf("*********************************\n");
         
      } else  {
      
         printf("Error, %s invoked with incorrect number of arguments\n", argv[0]);
         print_usage = TRUE;
      }
   }

   if (print_usage)  {
      if (shorthelp)  {
         printf("%s <0xsubnet> <0xdevaddr>\n", argv[0]);
      } else  {
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

int_32  Shell_GetDiAnArg(int_32 argc, char_ptr argv[])
{
   boolean  print_usage, shorthelp = FALSE;
   int_32   return_code = SHELL_EXIT_SUCCESS;
   uint_8	sub, dev;
   char hex[3];
   UINT_16	i;
   hex[2] = '\0';

   print_usage = Shell_check_help_request(argc, argv, &shorthelp );

   if (!print_usage)  {
      if (argc == 3)  {
         //子网地址，
         hex[0] = argv[1][2];
		 hex[1] = argv[1][3];
		 strupr(hex);
		 sub = ahextoi(hex);
		 printf("*****************************\n");
		 printf("DiAn subaddr=%02X  ", sub);
		 
         //设备地址
         hex[0] = argv[2][2];
		 hex[1] = argv[2][3];
		 strupr(hex);
		 dev = ahextoi(hex);
		 printf("devaddr=%02X\n", dev);
		 //ParaData0.brd=0x00;
		 GetParaData.index=0;
 	     GetParaData.len=20;
 		 GetParaData.dev=dev;
	
	
        
		 EMN_APL_GetParameter(sub, dev, &GetParaData);
		 i = 0;
		 while (i < 400)
		 {
		 	_time_delay(10);
			if (GetParaData.flag)
			{
				break;
			}
			i++;
		 }
		 if(i != 400)
		 {
		 	  printf("\nGet Time = %d ms\n",i*10);
		 	  printf("[软件版本号] = %c%d.%d.%d\n", GetParaData.data[0], GetParaData.data[1], GetParaData.data[2], GetParaData.data[3]);
		 	  printf("[硬件版本号] = %c%d.%d.%d\n", GetParaData.data[4], GetParaData.data[5], GetParaData.data[6], GetParaData.data[7]);
		 	  printf("[上行地址] = 0x%02X\n", GetParaData.data[8]);
		 	  if (GetParaData.data[9])
		 	  {
		 	  	printf("[设备类型] = 子设备\n");
		 	  }
		 	  else
		 	  {
		 	  	printf("[设备类型] = 簇头设备\n");
		 	  }
		 	  printf("[网络地址] = 0x%02X\n", GetParaData.data[10]);
		 	  printf("[子网地址] = 0x%02X\n", GetParaData.data[11]);
		 	  printf("[设备地址] = 0x%02X\n", GetParaData.data[12]);
		 	  printf("[子设备数] = %d\n", GetParaData.data[13]);
		 	  printf("[UID] = %02X-%02X-%02X-%02X\n", GetParaData.data[15], GetParaData.data[16], GetParaData.data[17], GetParaData.data[18]);
		 	  
		 }
		 else
		 	{
		 	  printf("timeout get parameter\n");
		 	}
		 
		 
		 GetParaData.index=20;
 	     GetParaData.len=20;
 		 GetParaData.dev=dev;
		 EMN_APL_GetParameter(sub, dev, &GetParaData);
		 i = 0;
		 while (i < 400)
		 {
		 	_time_delay(10);
			if (GetParaData.flag)
			{
				break;
			}
			i++;
		 }
		 if(i != 400)
		 {
		 	  printf("\nGet Time = %d ms\n",i*10);
		 	 
		 	  printf("[下行地址1] = %02X:%02X:%02X\n", GetParaData.data[1], GetParaData.data[2], GetParaData.data[3]);
		 	  printf("[下行地址2] = %02X:%02X:%02X\n", GetParaData.data[5], GetParaData.data[6], GetParaData.data[7]);
		 	  printf("[下行地址3] = %02X:%02X:%02X\n", GetParaData.data[9], GetParaData.data[10], GetParaData.data[11]);
		 	  printf("[下行地址4] = %02X:%02X:%02X\n", GetParaData.data[13], GetParaData.data[14], GetParaData.data[15]);
		 	  printf("[下行地址5] = %02X:%02X:%02X\n", GetParaData.data[17], GetParaData.data[18], GetParaData.data[19]);
		 }
		 else
		 	{
		 	  printf("timeout get parameter\n");
		 	}
		 	
		 	GetParaData.index=40;
 	     GetParaData.len=12;
 		 GetParaData.dev=dev;
		 EMN_APL_GetParameter(sub, dev, &GetParaData);
		 i = 0;
		 while (i < 400)
		 {
		 	_time_delay(10);
			if (GetParaData.flag)
			{
				break;
			}
			i++;
		 }
		 if(i != 400)
		 {
		 	  printf("\nGet Time = %d ms\n",i*10);
		 	 
		 	  printf("[下行地址6] = %02X:%02X:%02X\n", GetParaData.data[1], GetParaData.data[2], GetParaData.data[3]);
		 	  printf("[下行地址7] = %02X:%02X:%02X\n", GetParaData.data[5], GetParaData.data[6], GetParaData.data[7]);
		 	  printf("[下行地址8] = %02X:%02X:%02X\n", GetParaData.data[9], GetParaData.data[10], GetParaData.data[11]);
		 }
		 else
		 	{
		 	  printf("timeout get parameter\n");
		 	}
		
		 printf("*****************************\n");
         
      } else  {
      
         printf("Error, %s invoked with incorrect number of arguments\n", argv[0]);
         print_usage = TRUE;
      }
   }

   if (print_usage)  {
      if (shorthelp)  {
         printf("%s <0xsubnet> <0xdevaddr>\n", argv[0]);
      } else  {
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

int_32  Shell_SetDiAnArg(int_32 argc, char_ptr argv[])
{
   boolean  print_usage, shorthelp = FALSE;
   int_32   return_code = SHELL_EXIT_SUCCESS;
   uint_8	sub, dev;
   char hex[3];
   char filename[28];
   	FILE_PTR fd_ptr;
	char s[100];
	char t[20];
   UINT_16	i;
	uint_8 downtab[24];
	uint_8 j;
   uint_32 ipaddr;

   hex[2] = '\0';
   filename[27] = '\0';
   print_usage = Shell_check_help_request(argc, argv, &shorthelp );

   if (!print_usage)  {
      if (argc == 3)  {
         //子网地址，
         hex[0] = argv[1][2];
		 hex[1] = argv[1][3];
		 strupr(hex);
		 sub = ahextoi(hex);
		 printf("*********************************\n");
		 printf("DiAn subaddr=%02X  ", sub);
		 
         //设备地址
         hex[0] = argv[2][2];
		 hex[1] = argv[2][3];
		 strupr(hex);
		 dev = ahextoi(hex);
		 printf("devaddr=%02X\n", dev);
		 //先取得文件名
	     sprintf(filename, "d:\\dianconfig\\dian_%02x_%02x.txt", sub, dev);
		 
		 //先打开配置文件
		 fd_ptr =fopen(filename, "r");
		if (fd_ptr == NULL) {
				printf("The Config File is not exitting\n");
				return_code = SHELL_EXIT_ERROR;
				print_usage = TRUE;
				return return_code;
			}


			while (strcmp(s, "[BEGIN]") != 0) {
				fgetline(fd_ptr, s, 64);	//[BEGIN]
				if (feof(fd_ptr))
				{
					printf("\n");
					printf("没有找到文件头[BEGIN]，请用ls,dir,type等命令检查文件是否正确！\n");
					fclose(fd_ptr);
					return_code = SHELL_EXIT_ERROR;
					print_usage = TRUE;
					return return_code;
				}
				//printf("%s\n", s);
			}
			
			
			fgetline(fd_ptr, s, 64);	//[HARDWAREVERSION] 
			fgetline(fd_ptr, s, 64);
			printf("%s\n", s);
			if (cutstr(s, t, '='))
			{
				printf("文本数据书写格式错误\n");
				return_code = SHELL_EXIT_ERROR;
				print_usage = TRUE;
				return return_code;
			}
			for (i=18; i>0; i--)
			{
				t[i+1] = t[i];
			}
			t[0] = '1';
			t[1] = '.';
			SetParameterData.data[0] = 'V';
			get_dotted_address(t, &ipaddr);

			SetParameterData.data[1] = (uint_8) (ipaddr >> 16);
			SetParameterData.data[2] = (uint_8) (ipaddr >> 8);
			SetParameterData.data[3] = (uint_8) (ipaddr);
			
			for (i=4; i<10; i++)
			{
				fgetline(fd_ptr, s, 64);	//[UPADDR]
				fgetline(fd_ptr, s, 64);
				printf("%s\n", s);
				if (cutstr(s, t, '='))
				{
					printf("文本数据书写格式错误\n");
					return_code = SHELL_EXIT_ERROR;
					print_usage = TRUE;
					return return_code;
				}
					
				//
				hex[0] = t[2];
				hex[1] = t[3];
				strupr(hex);
				SetParameterData.data[i] = ahextoi(hex);
			}
			
		
		 
		 
		 printf("devaddr=%02X\n", dev);
		 SetParameterData.brd=0x00;
		 SetParameterData.index=4;
 	     SetParameterData.len=10;
 		 SetParameterData.dev=dev;
 		 
		 
		 EMN_APL_SetParameter(sub, dev, &SetParameterData);
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
		 	    return_code = SHELL_EXIT_ERROR;
				print_usage = TRUE;
				return return_code;
		 	}
		 	
		 	
		    fgetline(fd_ptr, s, 64);	//[UID]
		    fgetline(fd_ptr, s, 64);
			printf("%s\n", s);
			if (cutstr(s, t, '='))
			{
				printf("文本数据书写格式错误\n");
				return_code = SHELL_EXIT_ERROR;
				print_usage = TRUE;
				return return_code;
			}
					
			//
			for (i=0; i<4; i++)
			{
				hex[0] = t[3*i];
				hex[1] = t[3*i+1];
				strupr(hex);
				SetParameterData.data[i] = ahextoi(hex);
			}
		 	SetParameterData.data[4] = 0x00;
		 	SetParameterData.data[5] = 0x00;
		 	
		 	
		 		while (strcmp(s, "[DOWNTAB]") != 0) {
				fgetline(fd_ptr, s, 64);	//[DOWNTAB]
			}
			
			j = 0;
			while (strcmp(s, "[ENDDOWNTAB]") != 0) {
				fgetline(fd_ptr, s, 64);	//
				fgetline(fd_ptr, s, 64);
				printf("%s\n", s);
				i = 0;
				while (s[i * 5] != '\0') {
					hex[0] = s[i * 5 + 2];
					hex[1] = s[i * 5 + 3];
					i++;
					strupr(hex);
					downtab[j++] = ahextoi(hex);
					if (j >= 24)
					{
						break;
					}

				}
				if (j >= 24)
				{
					break;
				}
			}
		 	fclose(fd_ptr);
		 	SetParameterData.data[6] = downtab[0];
		 	SetParameterData.data[7] = downtab[1];
		 	SetParameterData.data[8] = downtab[2];
		 	SetParameterData.data[9] = 0;
		 	SetParameterData.data[10] = downtab[3];
		 	SetParameterData.data[11] = downtab[4];
		 	SetParameterData.data[12] = downtab[5];
		 	
		 
		 SetParameterData.brd=0x00;
		 SetParameterData.index=15;
 	     SetParameterData.len=13;
 		 SetParameterData.dev=dev;
 
		 EMN_APL_SetParameter(sub, dev, &SetParameterData);
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
		 	  return_code = SHELL_EXIT_ERROR;
				print_usage = TRUE;
				return return_code;
		 	}
	
	//第三包
		SetParameterData.data[0] = 0;
		SetParameterData.data[1] = downtab[6];
		SetParameterData.data[2] = downtab[7];
		SetParameterData.data[3] = downtab[8];
		SetParameterData.data[4] = 0;
		SetParameterData.data[5] = downtab[9];
		SetParameterData.data[6] = downtab[10];
		SetParameterData.data[7] = downtab[11];
		SetParameterData.data[8] = 0;
		SetParameterData.data[9] = downtab[12];
		SetParameterData.data[10] = downtab[13];
		SetParameterData.data[11] = downtab[14];
		 	
		SetParameterData.brd=0x00;
		 SetParameterData.index=28;
 	     SetParameterData.len=12;
 		 SetParameterData.dev=dev;
 
		 EMN_APL_SetParameter(sub, dev, &SetParameterData);
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
		 	  return_code = SHELL_EXIT_ERROR;
				print_usage = TRUE;
				return return_code;
		 	}
		 
		 	//第4包
		SetParameterData.data[0] = 0;
		SetParameterData.data[1] = downtab[15];
		SetParameterData.data[2] = downtab[16];
		SetParameterData.data[3] = downtab[17];
		SetParameterData.data[4] = 0;
		SetParameterData.data[5] = downtab[18];
		SetParameterData.data[6] = downtab[19];
		SetParameterData.data[7] = downtab[20];
		SetParameterData.data[8] = 0;
		SetParameterData.data[9] = downtab[21];
		SetParameterData.data[10] = downtab[22];
		SetParameterData.data[11] = downtab[23];
		 	
		SetParameterData.brd=0x00;
		 SetParameterData.index=40;
 	     SetParameterData.len=12;
 		 SetParameterData.dev=dev;
 
		 EMN_APL_SetParameter(sub, dev, &SetParameterData);
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
		 	  return_code = SHELL_EXIT_ERROR;
				print_usage = TRUE;
				return return_code;
		 	}
		 	
		 	
		 	//写入FLASH并重启
		 		SetParameterData.brd=0x00;
		 SetParameterData.index=255;
 	     SetParameterData.len=12;
 		 SetParameterData.dev=dev;
 
		 EMN_APL_SetParameter(sub, dev, &SetParameterData);
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
		 	  printf("参数设置成功\n");
		 	}
		 else
		 	{
		 	  printf("timeout set parameter\n");
		 	}
		 printf("*********************************\n");
		 
         
      } else  {
      
         printf("Error, %s invoked with incorrect number of arguments\n", argv[0]);
         print_usage = TRUE;
      }
   }

   if (print_usage)  {
      if (shorthelp)  {
         printf("%s <0xsubnet> <0xdevaddr>\n", argv[0]);
      } else  {
         printf("Usage: %s <0xsubnet> <0xdevaddr>\n", argv[0]);
  
      }
   }
   return return_code;
} 


//extern MQX_FILE_PTR uart2_dev;
extern MQX_FILE_PTR uart1_dev;
extern void upload_data_exit(void);
extern void rf_nwk_exit(void);
/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   
* Returned Value   :  int_32 error code
* Comments  :   
*
*END*---------------------------------------------------------------------*/

int_32 Shell_poweroff(int_32 argc, char_ptr argv[])
{
	boolean print_usage, shorthelp = FALSE;
	int_32 return_code = SHELL_EXIT_SUCCESS;
	char chr[8];
	char *hex;
	hex = chr;


	print_usage = Shell_check_help_request(argc, argv, &shorthelp);

	if (!print_usage) {
		if (argc != 2) {
			printf("Error, invalid number of parameters\n");
			return_code = SHELL_EXIT_ERROR;
			print_usage = TRUE;
		} else {
			if (argc == 2) {
					upload_data_exit();
					rf_nwk_exit();
					printf("program will poweroff!\n");
					*hex++ = '#';	
					*hex++ = 's';	
					*hex++ = 't';	
					*hex++ = argv[1][0];
					*hex++ = argv[1][1];
					*hex++ = argv[1][3];
					*hex++ = argv[1][4];
					*hex++ = '*';
//					return_code = fwrite(chr, sizeof(chr), 1, uart2_dev);
//2014年10月14日   by vitor
            		if (IO_ERROR == return_code) 
           			{
           				printf("error to send data to uart2\r\n");
           				return_code = SHELL_EXIT_ERROR;
           				return return_code;
            		}
					
				}
				// } else {
				//   printf("Invalid rtc time specified\n");
				//} 
			}
	}

	if (print_usage) {
		if (shorthelp) {
			printf("%s <HH:MM>\n", argv[0]);
		} else {
			printf("Usage: %s <Time>\n", argv[0]);
			printf("   <uid> = HH:MM\n");
		}
	}
	return return_code;
}


extern void PHY_WriteFreq(UINT_8 data);
extern void StopGetData(void);
extern void StartGetData(void);
extern LWSEM_STRUCT NetworkRfSem;

int_32 Shell_SetRFFreq(int_32 argc, char_ptr argv[])
{
	boolean print_usage, shorthelp = FALSE;
	int_32 return_code = SHELL_EXIT_SUCCESS;
	char chr[8];
	char *hex;
	uint_8 data;
	hex = chr;


	print_usage = Shell_check_help_request(argc, argv, &shorthelp);

	if (!print_usage) {
		if (argc != 2) {
			printf("Error, invalid number of parameters\n");
			return_code = SHELL_EXIT_ERROR;
			print_usage = TRUE;
		} else {
			if (argc == 2) {
					StopGetData();
					_time_delay(1000);
					hex[0] = argv[1][2];
		 			hex[1] = argv[1][3];
		 			strupr(hex);
					 data = ahextoi(hex);
		 			PHY_WriteFreq(data);
					StartGetData();
					_lwsem_wait(&NetworkRfSem);
					EMN_APL_GetSampleData();
					_lwsem_post(&NetworkRfSem);
					
				}
				// } else {
				//   printf("Invalid rtc time specified\n");
				//} 
			}
	}

	if (print_usage) {
		if (shorthelp) {
			printf("%s <0xXX>\n", argv[0]);
		} else {
			printf("Usage: %s 0xXX\n", argv[0]);
		}
	}
	return return_code;
}


/* EOF*/
