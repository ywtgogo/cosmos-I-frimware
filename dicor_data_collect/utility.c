/*************************************************************
* 
* Copyright (c) convertergy ;
* All Rights Reserved
* FileName: utility.c
* Version : 
* Date    : 2011/09/08
* ower    : Alex
*
* Comments:几个功能函数
*
*
***************************************************************/


#include "utility.h"
#include <string.h>
#include <math.h>
#include <mqx.h>
#include <bsp.h>

/*   Function Code       */
void print_sys_time
(
    void
)
{ /* Body*/
    TIME_STRUCT  	ts;
    DATE_STRUCT  	tm;
	
	uint32_t		sys_time;
	TIME_STRUCT		mqx_time;
	
	_time_get(&mqx_time);
	sys_time = mqx_time.SECONDS;
	
    ts.SECONDS = sys_time;
    ts.MILLISECONDS = 0;
    /* Convert rtc_time to date format */
    if (_time_to_date(&ts, &tm) == FALSE ) {
        printf("\n Cannot convert mqx_time to date format");
        //_task_block();
    }
    printf("\n (SYS) Current time is: %4d-%.2d-%.2d %.2d:%.2d:%.2d ",
            //_days_abbrev[tm.WDAY],
            //_months_abbrev[tm.MONTH - 1],
		   	tm.YEAR,
			tm.MONTH,
            tm.DAY,
            tm.HOUR,
            tm.MINUTE,
            tm.SECOND
            );
} /* Endboy*/

void print_rtc_time
(
    void
)
{ /* Body*/
    TIME_STRUCT  	ts;
    DATE_STRUCT  	tm;
	
	RTC_TIME_STRUCT time_rtc;
	_rtc_get_time(&time_rtc);
	_rtc_time_to_mqx_time(&time_rtc, &ts);
    _time_to_date(&ts, &tm);
    printf("\n (RTC) Current time is: %4d-%.2d-%.2d %.2d:%.2d:%.2d ",
		   	tm.YEAR,
			tm.MONTH,
            tm.DAY,
            tm.HOUR,
            tm.MINUTE,
            tm.SECOND
            );
} /* Endboy*/

//小写转大写
void strupr(char *s)
{
	unsigned char i;
	for(i=0; '\0' != s[i]; i++)
	{
		if(s[i] >= 'a' && s[i] <= 'z')
		{
				s[i] -= 32;  	
		}
	}
}



//将十六进制字符串拼成数值
unsigned char ahextoi(const char* hexstr)
{
	unsigned char val;
	unsigned char vh, vl;
	char temp;
	temp = *hexstr;
	if (temp <= '9' && temp >= '0')
	{
		vh = temp - '0';
	}
	else
	{
		vh = temp - 'A' + 10;
	}
	temp = *(hexstr+1);
	if (temp <= '9' && temp >= '0')
	{
		vl = temp - '0';
	}
	else
	{
		vl = temp - 'A' + 10;
	}
	val = vh * 16 + vl;
	return val;
}



boolean get_dotted_address (char_ptr name, uint_32_ptr address)
{
   uint_32                     i;
   uint_32                     digit_count = 0;
   uint_32                     dot_count = 0;
   int_32                      byte_num;
   boolean                     dotted_decimal = TRUE;
   
   if ((name == NULL) || (address == NULL)) return FALSE;
   
   *address = 0;
   
   for (i=0; name[i] != '\0'; ++i ) {
      if ( name[i] == '.' ) {
         dot_count++;
         if ( dot_count > 3 ) {
            /* invalid IP address */
            dotted_decimal = FALSE;
            break;
         }/* Endif */

         if ( digit_count == 0 ) {
            /* there are no digits before the '.' */
            dotted_decimal = FALSE;
            break;
         }/* Endif */
         digit_count = 0;
         byte_num = byte_num / 10; /* shift back */

         if ( (byte_num < 0 ) || (byte_num > 255) ) {
         /* if the number does fall within this range it's invalid */
            dotted_decimal = FALSE;
            break;
         } else  {
            *address = (*address) + byte_num;
            *address = (*address) * 0x100;
         }/* Endif */
      } else { /* a digit */

         if ( digit_count == 0 ) {
            byte_num = 0;
         }/* Endif */
         ++digit_count;

         if ( digit_count > 3 ) {
            /* too many digits between the '.' */
            dotted_decimal = FALSE;
            break;
         }/* Endif */

         if ( (name[i] >= '0') && (name[i] <= '9') ) {
            /* number is in range */
            byte_num = byte_num + name[i] - '0';
            byte_num = byte_num * 10;
         } else {
            /* if the characters are not decimal digits it's invalid */
            dotted_decimal = FALSE;
            break;
         }/* Endif */
      }/* Endif */
   } /* Endfor */

   if ( dotted_decimal ) { /* check last number */
      if ( digit_count == 0 ) {
         /* there are no digits before the '.' */
         dotted_decimal = FALSE;
      }/* Endif */

      byte_num = byte_num / 10;
      if ( (byte_num < 0 ) || (byte_num > 255) ) {
         /* if the number does fall within this range it's invalid */
         dotted_decimal = FALSE;
      } else {
         *address = (*address) + byte_num;
      }/* Endif */

      if ( dot_count != 3 ) {
         /* the wrong number of dots were found */
         dotted_decimal = FALSE;
      }/* Endif */

   }/* Endif */


   if ( i == 0 ) {
      /* no name string of first char was not a number */
      dotted_decimal = FALSE;
   }/* Endif */
    
   return dotted_decimal;
}

//截取字符串
uchar cutstr(const char* sour, char* dest, char c)
{
	while (*sour != c)
	{
		if (*sour == '\0')
		{
			return 1;
		}
		sour++;	
	}
	sour++;//跳过c
	//跳过空格
	while (*sour == ' ')
	{
		if (*sour == '\0')
		{
			return 1;
		}
		sour++;
	}
			
	while ((*sour!=' ') && (*sour!='#') && (*sour!='\0'))
	{
		*dest++ = *sour++;
	}
	*dest = '\0';
	return 0;
}

unsigned short UT_calcrc16(unsigned short crclast, unsigned char *buf, unsigned short len)
{
    unsigned char ii = 0;
    unsigned short crc = 0;
 	unsigned short tlen = len;
    crc = crclast;
    while (tlen--)
    {
        crc = crc ^ (unsigned short)((*buf++ << 8) & 0xFFFF);
        ii = 8;
        do
        {
            if (crc & 0x8000)
                crc = crc << 1 ^ 0x1021;
            else
                crc = crc << 1;
        }
  while(--ii);
    }
    return crc;
}

/* EOF */
