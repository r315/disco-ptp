#ifndef PING_CMD_H
#define PING_CMD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lwip/ip_addr.h"

#define PING_ID         0x1400
#define PING_DATA_SIZE  (64 - sizeof(struct icmp_echo_hdr))
#define PING_DELAY_MS   1000
#define PING_TIMEOUT_MS 1000

int do_ping(ip_addr_t *addr, u16_t count);

#ifdef __cplusplus
}
#endif

#endif // PING_CMD_H
