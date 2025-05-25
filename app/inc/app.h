#ifndef _app_h_
#define _app_h_

#include "lcd_log.h"

//#define ENABLE_DHCP       /* enable DHCP, if disabled static address is used*/
//#define ENABLE_CLI
//#define ENABLE_UART
//#define ENABLE_DHCP_SERVER

/*Static IP ADDRESS*/
#define IP_ADDR0   192
#define IP_ADDR1   168
#define IP_ADDR2   0
#define IP_ADDR3   10

/*NETMASK*/
#define NETMASK_ADDR0   255
#define NETMASK_ADDR1   255
#define NETMASK_ADDR2   255
#define NETMASK_ADDR3   0

/*Gateway Address*/
#define GW_ADDR0   192
#define GW_ADDR1   168
#define GW_ADDR2   0
#define GW_ADDR3   1

#define LOG_Usr LCD_UsrLog

void APP_Setup(void);

#endif