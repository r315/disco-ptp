#include <stdio.h>
#include <string.h>
#include "app.h"
#include "main.h"
#include "cmsis_os.h"
#include "app_ethernet.h"
#include "httpserver-socket.h"
#include "ethernetif.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "lwip/ip_addr.h"
#include "dhcp_server.h"
#include "ping.h"
#include "ptpd.h"

static struct netif gnetif; /* network interface structure */

#ifdef ENABLE_CLI

#include "cli_simple.h"

static int serial_available(void){ return UART_Available(); }
static int serial_read(char *buf, int len){ return UART_Read((uint8_t*)buf, len); }
static int serial_write(const char *buf, int len){ return UART_Write((const uint8_t*)buf, len); }

static const stdinout_t serial = {
    .available = serial_available,
    .read = serial_read,
    .write = serial_write
};

static int cmdReset(int argc, char **argv)
{
    NVIC_SystemReset();
    return CLI_OK;
}

static int cmdPhy(int argc, char **argv)
{
	extern ETH_HandleTypeDef EthHandle;
	uint32_t reg_value;

    const char *reg_name[] = {
        "BCR", "BSR",
        "ID1", "ID2", "ANAR", "AENR", "ANER",
        "MCSR","SM","SECR","CSIR","ISR","IMR","PSCR"

    };

    const uint8_t reg_index[] = {
        PHY_BCR, PHY_BSR,
        2,3,4,5,6,
        17,18,26,27,29,30,31
    };

    for (uint8_t i = 0; i < sizeof(reg_index); i++){
	    HAL_ETH_ReadPHYRegister(&EthHandle, reg_index[i], &reg_value);
	    printf("[%d] %s 0x%08lX\n", reg_index[i], reg_name[i], reg_value);
    }

	return CLI_OK;
}

static int cmdDhcps(int argc, char **argv)
{
	if(!strcmp(argv[1], "start")){
		dhcp_server_start();
	}

	if(!strcmp(argv[1], "stop")){
		dhcp_server_stop();
	}
    return CLI_OK;
}

static int cmdPing(int argc, char **argv)
{
    ip_addr_t target_addr;

    if (argc < 2) {
        printf("Usage: ping <IP address>\n");
        return CLI_OK;
    }

    if (!ipaddr_aton(argv[1], &target_addr)) {
        printf("Invalid IP address: %s\n", argv[1]);
        return CLI_BAD_PARAM;
    }

    printf("Pinging %s...\n", argv[1]);

    do_ping(&target_addr, 4);

    return CLI_OK;
}

static osThreadId ptpTaskHandle;

static int cmdPtpd(int argc, char **argv)
{
    if(CLI_IS_PARM(1, "start")){
        ptpTaskHandle = ptpd_init();
    }

    if(CLI_IS_PARM(1, "stop")){
        osThreadTerminate(ptpTaskHandle);
    }

    if(CLI_IS_PARM(1, "status")){
        //UBaseType_t restStack ;
        //restStack = uxTaskGetStackHighWaterMark(ptpTaskHandle);
        //printf("PTP task rest stack:%lu byte\n",restStack*4);
    }

    return CLI_OK;
}

static const cli_command_t cli_cmds [] = {
    {"help", ((int (*)(int, char**))CLI_Commands)},
    {"reset", cmdReset},
    {"phy", cmdPhy},
	{"dhcps", cmdDhcps},
	{"ping", cmdPing},
    {"ptpd", cmdPtpd},
};

static void CLI_thread(void const *argument)
{
    CLI_Init("ptp>", &serial);
    CLI_RegisterCommand(cli_cmds, sizeof(cli_cmds)/sizeof(cli_command_t));

	printf("CPU clock: %luMHz\n", SystemCoreClock/1000000UL);

	CLI_Run((void(*)(void))osThreadYield);
}
#endif /* ENABLE_CLI */

/* Redirect the printf to the LCD */
#ifdef __GNUC__
/* With GCC, small printf (option LD Linker->Libraries->Small printf
set to 'Yes') calls __io_putchar() */
int __io_getchar(void) { uint8_t ch; UART_Read(&ch, 1); return ch; }
//int __io_putchar(int ch) { return UART_Write((uint8_t*)&ch, 1); }
int __io_putchar(int ch)
{
#ifdef ENABLE_UART
    UART_Write((uint8_t*)&ch, 1);
#endif
    return LCD_LOG_Putchar(ch);
}
#else
int fputc(int ch, FILE *f) { return LCD_LOG_Putchar(ch); }
#endif /* __GNUC__ */

/**
 * @brief  Initializes the lwIP stack
 * @param  None
 * @retval None
 */
static void Netif_Config(void)
{
    ip_addr_t ipaddr;
    ip_addr_t netmask;
    ip_addr_t gw;

#ifdef ENABLE_DHCP
    ip_addr_set_zero_ip4(&ipaddr);
    ip_addr_set_zero_ip4(&netmask);
    ip_addr_set_zero_ip4(&gw);
#else
    IP_ADDR4(&ipaddr, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
    IP_ADDR4(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
    IP_ADDR4(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
#endif /* ENABLE_DHCP */

    netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input);

    /*  Registers the default network interface. */
    netif_set_default(&gnetif);

    if (netif_is_link_up(&gnetif))
    {
        /* When the netif is fully configured this function must be called.*/
        netif_set_up(&gnetif);
    }
    else
    {
        /* When the netif link is down this function must be called */
        netif_set_down(&gnetif);
    }
}

void APP_Setup(void)
{
#ifdef ENABLE_CLI
    osThreadDef(CLI, CLI_thread, osPriorityBelowNormal, 0, configMINIMAL_STACK_SIZE * 2);
    osThreadCreate(osThread(CLI), NULL);
    CLI_Clear();
#endif

    LOG_INF ("  State: Ethernet Initialization ...");

    /* Create tcp_ip stack thread */
    tcpip_init(NULL, NULL);

    /* Initialize the LwIP stack */
    Netif_Config();

    /* Initialize webserver demo */
    http_server_socket_init();

    /* Notify user about the network interface config */
    User_notification(&gnetif);

    dhcp_server_init(gnetif.ip_addr);
#ifdef ENABLE_DHCP_SERVER
    dhcp_server_start();
#endif

#ifdef ENABLE_DHCP
    /* Start DHCPClient */
    osThreadDef(DHCP, DHCP_thread, osPriorityBelowNormal, 0, configMINIMAL_STACK_SIZE * 2);
    osThreadCreate(osThread(DHCP), &gnetif);
#endif
}
