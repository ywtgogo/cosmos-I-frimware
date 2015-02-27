/**********************************************************************
* 
* Copyright (c) 2010 convertergy;
* All Rights Reserved
* FileName: dicor.c
* Version : 
* Date    : 
*
* Comments:
*
*   this file implements the feature about network part. it handle the share part that used by web server and 
*    telnet server, or shell command.
*
*************************************************************************/
#include <mqx.h>
#include <bsp.h>
#include <ipcfg.h>
#include <lwtimer.h>
#include <Watchdog.h>
#include <string.h>
#include "dicor.h"
#include "eeprom.h"
#include "led.h"
#include "dicor_upload.h"
#include "logging_public.h"
#include "dicor_upload.h"

extern void dicor_get_timer(void);
extern _mem_pool_id _Enet_pool_id;


//#define ENET_IPDNS          IPADDR(192,168,8,1) 
/**************************************************
 return value
         0: OK.  it is our desired.
         1: no cable is plugged in. 
         2: No ip adress is allocated.      
         3: initalization is failed.( install the ethernet driver)

***************************************************/
uint_32 dicor_InitializeNetworking(uint_32 pcbs, uint_32 msgs,   uint_32 sockets, boolean dhcp) 
{
    int_32                	error;
    IPCFG_IP_ADDRESS_DATA	ip_data;
	uint_32  dicor_id_no = 0;
	DATE_STRUCT date;
    //使用外部扩展的SRAM
 	_RTCS_mem_pool = _mem_create_pool((pointer)DICORCFG_RTCS_POOL_ADDR, DICORCFG_RTCS_POOL_SIZE); 
    /* runtime RTCS configuration */  
#if 0
    _RTCSPCB_init = pcbs;
    _RTCS_msgpool_init = msgs;
    _RTCS_socket_part_init = sockets;
#else 
    _RTCSPCB_init = 8;
    _RTCSPCB_grow = 2;
    _RTCSPCB_max = 32;
    _RTCS_msgpool_init = 8;
    _RTCS_msgpool_grow = 2;
    _RTCS_msgpool_max  = 16;
    _RTCS_socket_part_init = 8;
    _RTCS_socket_part_grow = 2;
    _RTCS_socket_part_max  = 16;
#endif
	DEBUG_DIS(printf("RTCS create ...\n")); 	
    error = RTCS_create();

    if (error == RTCS_OK) {
    	DEBUG_DIS(printf("rtcs create successful!\n"));
        IPCFG_default_enet_device = BSP_DEFAULT_ENET_DEVICE;
        IPCFG_default_ip_address = IPADDR(pBaseConfig->ip[0],pBaseConfig->ip[1],pBaseConfig->ip[2],pBaseConfig->ip[3]);
        IPCFG_default_ip_mask = IPADDR(pBaseConfig->mask[0],pBaseConfig->mask[1],pBaseConfig->mask[2],pBaseConfig->mask[3]);
        IPCFG_default_ip_gateway = IPADDR(pBaseConfig->gateway[0],pBaseConfig->gateway[1],pBaseConfig->gateway[2],pBaseConfig->gateway[3]);
#if RTCSCFG_ENABLE_LWDNS
        LWDNS_server_ipaddr = IPADDR(pBaseConfig->dns[0],pBaseConfig->dns[1],pBaseConfig->dns[2],pBaseConfig->dns[3]);
#endif      
        ip_data.ip = IPCFG_default_ip_address;
        ip_data.mask = IPCFG_default_ip_mask;
        ip_data.gateway = IPCFG_default_ip_gateway;
        
    
		dicor_id_no = (dicor_id_no+pBaseConfig->uid[0])<<8;
      	dicor_id_no = (dicor_id_no+pBaseConfig->uid[1])<<8;
		dicor_id_no = (dicor_id_no+pBaseConfig->uid[2])<<8;
		dicor_id_no += pBaseConfig->uid[3];

        ENET_get_mac_address(IPCFG_default_enet_device, dicor_id_no, IPCFG_default_enet_address);
		ipcfg_init_device(IPCFG_default_enet_device, IPCFG_default_enet_address);//add by dick
        //ENET_initialize_ex(&ENET_param[0], IPCFG_default_enet_address, &ehandle);	
		
        /*if (RTCS_if_add(ehandle, RTCS_IF_ENET, &ihandle) == 0) {
		    if (ipcfg_init_interface(-1, ihandle) != 0) {
		        RTCS_if_remove(ihandle);
 	 	        ENET_shutdown(ehandle);
 	 	        return(3);
 	 	    }
	    }
	    else {
	        ENET_shutdown(ehandle);
	    }*/

		ipcfg_add_dns_ip(IPCFG_default_enet_device,LWDNS_server_ipaddr); //add by dick

        error = 0;
        while(!ipcfg_get_link_active(IPCFG_default_enet_device)) 
		{
            if(error == 0)
				printf("\n please plug ethernet cable in ... ");
			EthernetLedOff();
			DimoLedOff();
			SdLedOff();
			error++;				
			_time_delay(500);	
			if(error > 10)
			{
				dicor_get_logtime(&date);
				sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t 没有插网线\r\n", date.HOUR,date.MINUTE,date.SECOND);
				PrintLog(pDiCorLog->logbuf);
				return(1);
			}	
		}
        printf("Cable connected\n");   
		//此处应该做成闪灯eth
		DimoLedOff();
		SdLedOff();
        /* If DHCP Enabled, get IP address from DHCP server */ 
        if (dhcp) {        
#if 1
	        uint_8  ii;  
            DEBUG_DIS(printf("\nDHCP bind ... "));
			for(ii =0 ; ii < 10 ;ii++)
			{
				ip_data.ip = IPCFG_default_ip_address;
				ip_data.mask = IPCFG_default_ip_mask;
				ip_data.gateway = IPCFG_default_ip_gateway;
	            error = ipcfg_bind_dhcp_wait(IPCFG_default_enet_device, 0, &ip_data);
	            
	            if (error != IPCFG_ERROR_OK) {
	                DEBUG_DIS(printf("Error %08x!\n", error));
	            	dicor_get_logtime(&date);
					EthernetLedOff();
					DimoLedOff();
					SdLedOff();
					sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t 动态分配IP(DHCP)出错，出错代码:%08X\r\n", date.HOUR,date.MINUTE,date.SECOND,error);
					PrintLog(pDiCorLog->logbuf);
			
					if(9 == ii)
					    return(2);
	            } else {
	                DEBUG_DIS(printf("Successful!\n"));
	                //EthernetLedOn();//server
					//DimoLedOn();
					SdLedOn();//eth
					break;
	            }
			}
#else
			printf("\nDHCP bind ... ");
			error = ipcfg_bind_dhcp_wait(IPCFG_default_enet_device, 1, &ip_data);
			if (error != IPCFG_ERROR_OK) {
				printf("Error %08x!\n", error);
				EthernetLedOff();
				DimoLedOff();
				SdLedOff();
			} else {
                printf("Successful!\n");
                //EthernetLedOn();
				//DimoLedOn();
				SdLedOn();
            }
#endif        
        } else {
            /* Else bind with static IP */
            DEBUG_DIS(printf ("\nStatic IP bind ... "));
            error = ipcfg_bind_staticip(IPCFG_default_enet_device, &ip_data);
   
            if (error != IPCFG_ERROR_OK) {
                DEBUG_DIS(printf("Error %08x!\n",error));
	            dicor_get_logtime(&date);
				sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t 静态绑定IP出错，出错代码:%08X\r\n", date.HOUR,date.MINUTE,date.SECOND,error);
				PrintLog(pDiCorLog->logbuf);
				return(2);
            } 
            else {
                DEBUG_DIS(printf("Successful!\n"));
                //EthernetLedOn();
				//DimoLedOn();
				SdLedOn();
            }
        }
   
        if (error == IPCFG_ERROR_OK) {
            ipcfg_get_ip(IPCFG_default_enet_device, &dicor_ip_data);
            DEBUG_DIS(printf("\nIP Address      : %d.%d.%d.%d\n",IPBYTES(dicor_ip_data.ip)));
			/**************** end ************************/
            DEBUG_DIS(printf("Subnet Address  : %d.%d.%d.%d\n",IPBYTES(dicor_ip_data.mask)));
            DEBUG_DIS(printf("Gateway Address : %d.%d.%d.%d\n",IPBYTES(dicor_ip_data.gateway)));
            DEBUG_DIS(printf("DNS Address     : %d.%d.%d.%d\n",IPBYTES(ipcfg_get_dns_ip(IPCFG_default_enet_device,0))));
            //EthernetLedOn();
			//从网络获取时间
			dicor_get_timer();
        }
    } else {
        DEBUG_DIS(printf("rtcs init error\n"));
        dicor_get_logtime(&date);

		sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t RTCS创建失败\r\n", date.HOUR,date.MINUTE,date.SECOND);
		PrintLog(pDiCorLog->logbuf);

		return (3);
        //_task_block();
    }
	return(0);

}


//char_ptr timeserver[] = {"time.nist.gov","ntp.fudan.edu.cn","210.72.145.44","time.windows.com"};//添加这个列表



uint_32 dicor_GetTime(void) 
{
//#if DICORCFG_ENABLE_SNTP
   _ip_address  ipaddr;
   int_8 ii;
   DATE_STRUCT date;
  	UINT_32;
   
   if (!(ipcfg_get_link_active(0))) {
			upload_buffer.eth_st = ETH_INIT;
			shutdown(upload_buffer.sock,
				 FLAG_ABORT_CONNECTION);
				 EthernetLedOff();
				  DimoLedOff();
		SdLedOff();
			DEBUG_DIS(printf("\n please plug cable"));
			return 3;
			}
			
   if (pBaseConfig->enablesntp)
   {
   	
   		DEBUG_DIS(printf("Getting time from time server [%s] ... \n", pBaseConfig->sntp1));

	   for(ii = 0 ; ii<4 ;ii++)
	   {
		   //if(RTCS_resolve_ip_address(DICOR_SNTP_SERVER,&ipaddr,NULL,0)) 
		   if(RTCS_resolve_ip_address(pBaseConfig->sntp1,&ipaddr,NULL,0))
		   //if(RTCS_resolve_ip_address(*(timeserver+n),&ipaddr,NULL,0))   
		   {
		        break;
		   }
		   else 
		   {
			   if( ii == 4)
			   {
			   		dicor_get_logtime(&date);
					sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t 时间服务器域名解析出错\r\n", date.HOUR,date.MINUTE,date.SECOND);
					PrintLog(pDiCorLog->logbuf);
				
				   DEBUG_DIS(printf("Failed in getting in LWDNS\n"));
				  return(2);  //  error in  SNTP server 
			   }
		   }
	   	}
	     /* Contact SNTP server and update time */
		  for(ii = 0; ii<3 ; ii++)
		  {
		      if(SNTP_oneshot(ipaddr,2000)==RTCS_OK) 
		      {
		         DEBUG_DIS(printf("Succeeded\n"));
				 return (0);
		      } 
		      else 
		      {
		
				 if( ii == 2)
				 {
					 DEBUG_DIS(printf("Failed in getting in SNTP\n"));
					 dicor_get_logtime(&date);
				
					sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t 从时间服务器1[%s]获取时间失败\r\n", date.HOUR,date.MINUTE,date.SECOND,pBaseConfig->sntp1);
					PrintLog(pDiCorLog->logbuf);
				
					 //return(1);  // error in get time 
				
				 }  
		      }
		  }
		  
		  if (!(ipcfg_get_link_active(0))) {
			upload_buffer.eth_st = ETH_INIT;
			shutdown(upload_buffer.sock,
				 FLAG_ABORT_CONNECTION);
				 EthernetLedOff();
				  DimoLedOff();
		SdLedOff();
			DEBUG_DIS(printf("\n please plug cable"));
			return 3;
			}
		  
		  DEBUG_DIS(printf("Getting time from time server [%s] ... \n", pBaseConfig->sntp2));

	   for(ii = 0 ; ii<4 ;ii++)
	   {
		   //if(RTCS_resolve_ip_address(DICOR_SNTP_SERVER,&ipaddr,NULL,0)) 
		   if(RTCS_resolve_ip_address(pBaseConfig->sntp2,&ipaddr,NULL,0))
		   //if(RTCS_resolve_ip_address(*(timeserver+n),&ipaddr,NULL,0))   
		   {
		        break;
		   }
		   else 
		   {
			   if( ii == 4)
			   {
				   DEBUG_DIS(printf("Failed in getting in LWDNS\n"));
				   dicor_get_logtime(&date);
				   
					sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t 时间服务器域名解析出错\r\n", date.HOUR,date.MINUTE,date.SECOND);
					PrintLog(pDiCorLog->logbuf);
				
				  return(2);  //  error in  SNTP server 
			   }  
		   }
	   	}
	     /* Contact SNTP server and update time */
		  for(ii = 0; ii<3 ; ii++)
		  {
		      if(SNTP_oneshot(ipaddr,2000)==RTCS_OK) 
		      {
		         DEBUG_DIS(printf("Succeeded\n"));
				 return (0);
		      } 
		      else 
		      {
		
				 if( ii == 2)
				 {
					 DEBUG_DIS(printf("Failed in getting in SNTP\n"));
					 dicor_get_logtime(&date);
					sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t 从时间服务器2[%s]获取时间失败\r\n", date.HOUR,date.MINUTE,date.SECOND,pBaseConfig->sntp2);
					PrintLog(pDiCorLog->logbuf);
				
					 //return(1);  // error in get time 
				
				 }  
		      }
		  }
		  
		  if (!(ipcfg_get_link_active(0))) {
			upload_buffer.eth_st = ETH_INIT;
			shutdown(upload_buffer.sock,
				 FLAG_ABORT_CONNECTION);
				 EthernetLedOff();
				 DimoLedOff();
		SdLedOff();
			DEBUG_DIS(printf("\n please plug cable"));
			return 3;
			}
		  
		  	DEBUG_DIS(printf("\nGetting time from time server [%s] ... \n", pBaseConfig->sntp3));

	   for(ii = 0 ; ii<4 ;ii++)
	   {
		   //if(RTCS_resolve_ip_address(DICOR_SNTP_SERVER,&ipaddr,NULL,0)) 
		   if(RTCS_resolve_ip_address(pBaseConfig->sntp3,&ipaddr,NULL,0))
		   //if(RTCS_resolve_ip_address(*(timeserver+n),&ipaddr,NULL,0))   
		   {
		        break;
		   }
		   else 
		   {
			   if( ii == 4)
			   {
				   DEBUG_DIS(printf("Failed in getting in LWDNS\n"));
				   dicor_get_logtime(&date);
					sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t 时间服务器域名解析出错\r\n", date.HOUR,date.MINUTE,date.SECOND);
					PrintLog(pDiCorLog->logbuf);
				  return(2);  //  error in  SNTP server 
			   }  
		   }
	   	}
	     /* Contact SNTP server and update time */
		  for(ii = 0; ii<3 ; ii++)
		  {
		      if(SNTP_oneshot(ipaddr,2000)==RTCS_OK) 
		      {
		         DEBUG_DIS(printf("Succeeded\n"));
				 return (0);
		      } 
		      else 
		      {
		
				 if( ii == 2)
				 {
					 DEBUG_DIS(printf("Failed in getting in SNTP\n"));
					 dicor_get_logtime(&date);
					sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t 从时间服务器3[%s]获取时间失败\r\n", date.HOUR,date.MINUTE,date.SECOND,pBaseConfig->sntp3);
					PrintLog(pDiCorLog->logbuf);
					 //return(1);  // error in get time 
				
				 }  
		      }
		  }
		  
		  if (!(ipcfg_get_link_active(0))) {
			upload_buffer.eth_st = ETH_INIT;
			shutdown(upload_buffer.sock,
				 FLAG_ABORT_CONNECTION);
				 EthernetLedOff();
				 DimoLedOff();
		SdLedOff();
			DEBUG_DIS(printf("\n please plug cable"));
			return 3;
			}
		  
		  DEBUG_DIS(printf("\nGetting time from time server [%s] ... \n", pBaseConfig->sntp4));

	   for(ii = 0 ; ii<4 ;ii++)
	   {
		   //if(RTCS_resolve_ip_address(DICOR_SNTP_SERVER,&ipaddr,NULL,0)) 
		   if(RTCS_resolve_ip_address(pBaseConfig->sntp4,&ipaddr,NULL,0))
		   //if(RTCS_resolve_ip_address(*(timeserver+n),&ipaddr,NULL,0))   
		   {
		        break;
		   }
		   else 
		   {
			   if( ii == 4)
			   {
				   DEBUG_DIS(printf("Failed in getting in LWDNS\n"));
				   dicor_get_logtime(&date);
				  
					sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t 时间服务器域名解析出错\r\n", date.HOUR,date.MINUTE,date.SECOND);
					PrintLog(pDiCorLog->logbuf);
				
				  return(2);  //  error in  SNTP server 
			   }  
		   }
	   	}
	     /* Contact SNTP server and update time */
		  for(ii = 0; ii<3 ; ii++)
		  {
		      if(SNTP_oneshot(ipaddr,2000)==RTCS_OK) 
		      {
		         DEBUG_DIS(printf("Succeeded\n"));
				 return (0);
		      } 
		      else 
		      {
		
				 if( ii == 2)
				 {
					 DEBUG_DIS(printf("Failed in getting in SNTP\n"));
					 dicor_get_logtime(&date);
			
					sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t 从时间服务器4[%s]获取时间失败\r\n", date.HOUR,date.MINUTE,date.SECOND,pBaseConfig->sntp4);
					PrintLog(pDiCorLog->logbuf);
				
					 return(1);  // error in get time 
				
				 }  
		      }
		  }
   
   }
   else
   {
   		dicor_get_logtime(&date);
	
		sprintf(pDiCorLog->logbuf, "%02d:%02d:%02d\t 时间服务器域名解析出错，域名服务没开被禁用\r\n", date.HOUR,date.MINUTE,date.SECOND);
		PrintLog(pDiCorLog->logbuf);
	
		printf("sntp is disable\n");
   		return(3);
   }
//#endif	  
}



/* EOF */
