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
#ifdef ENABLE_CLI
#include "cli_simple.h"
#endif

static struct netif gnetif; /* network interface structure */

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

#ifdef ENABLE_CLI
static int cmdReset(int argc, char **argv)
{
    NVIC_SystemReset();
    return CLI_OK;
}

static int cmdPhy(int argc, char **argv)
{
	extern ETH_HandleTypeDef heth;
	uint32_t reg_value;

	HAL_ETH_ReadPHYRegister(&heth, PHY_BCR, &reg_value);
	printf("BCR 0x%08lX\n", reg_value);
	HAL_ETH_ReadPHYRegister(&heth, PHY_BSR, &reg_value);
	printf("BSR 0x%08lX\n", reg_value);
	HAL_ETH_ReadPHYRegister(&heth, 2, &reg_value);
	printf("ID1 0x%08lX\n", reg_value);
	HAL_ETH_ReadPHYRegister(&heth, 3, &reg_value);
	printf("ID2 0x%08lX\n", reg_value);

	return CLI_OK;
}

static int cmdDhcps(int argc, char **argv)
{
	if(!strcmp(argv[1], "start")){
		//dhcp_server_start();
	}

	if(!strcmp(argv[1], "stop")){
		//dhcp_server_stop();
	}
    return CLI_OK;
}

static const cli_command_t cli_cmds [] = {
    {"help", ((int (*)(int, char**))CLI_Commands)},
    {"reset", cmdReset},
    {"phy", cmdPhy},
	{"dhcps", cmdDhcps},
};

void CLI_thread(void const *argument)
{
    CLI_RegisterCommand(cli_cmds, sizeof(cli_cmds)/sizeof(cli_command_t));
	CLI_Clear();

	printf("CPU clock: %luMHz\n", SystemCoreClock/1000000UL);

	while(1){
		 if(CLI_ReadLine() == CLI_LINE_READ){
			CLI_HandleLine();
		}
	}
}
#endif

void APP_Setup(void)
{
    /* Create tcp_ip stack thread */
    tcpip_init(NULL, NULL);

    /* Initialize the LwIP stack */
    Netif_Config();

    /* Initialize webserver demo */
    http_server_socket_init();

    /* Notify user about the network interface config */
    User_notification(&gnetif);

#ifdef ENABLE_DHCP
    /* Start DHCPClient */
    osThreadDef(DHCP, DHCP_thread, osPriorityBelowNormal, 0, configMINIMAL_STACK_SIZE * 2);
    osThreadCreate(osThread(DHCP), &gnetif);
#endif

#ifdef USE_CLI
    osThreadDef(CLI, CLI_thread, osPriorityBelowNormal, 0, configMINIMAL_STACK_SIZE);
    osThreadCreate(osThread(CLI), NULL);
#endif
}
