#ifndef DHCP_SERVER_H
#define DHCP_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lwip/udp.h"
#include "lwip/ip_addr.h"
#include "lwip/pbuf.h"
#include "lwip/netif.h"
#include "lwip/err.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// Public API

/**
 * @brief Initialize the DHCP server.
 *
 * This sets up the UDP socket, registers the receive callback, and
 * creates the FreeRTOS task that handles incoming DHCP packets.
 */
void dhcp_server_init(ip_addr_t addr);

#ifdef __cplusplus
}
#endif

#endif // DHCP_SERVER_H
