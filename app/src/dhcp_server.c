#include "lwip/udp.h"
#include "lwip/ip_addr.h"
#include "lwip/pbuf.h"
#include "lwip/netif.h"
#include "lwip/inet.h"
#include "lwip/mem.h"
#include "lwip/timeouts.h"
#include "string.h"
#include "cmsis_os.h"
#include <stdio.h>

#define DHCP_SERVER_PORT 67
#define DHCP_CLIENT_PORT 68
#define DHCP_MSG_LEN     276 //548

#define DHCPDISCOVER    1
#define DHCPOFFER       2
#define DHCPREQUEST     3
#define DHCPACK         5
#define DHCPMAGIC       0x63825363

// Simplified DHCP message
struct dhcp_payload {
    uint8_t op, htype, hlen, hops;
    uint32_t xid;
    uint16_t secs, flags;
    uint32_t ciaddr;
    ip_addr_t yiaddr;
    uint32_t siaddr;
    uint32_t giaddr;
    uint8_t chaddr[16];
    uint8_t sname[64];
    uint8_t file[128];
    uint32_t magic;
};

typedef struct _dhcps {
    struct udp_pcb *pcb;
    SemaphoreHandle_t sem;
    struct pbuf *pending_buf;
    ip_addr_t pending_addr;
    ip_addr_t ip_addr;
    osThreadId threadid;
}dhcps_t;

struct dhcp_msg {
    struct dhcp_payload payload;
    uint8_t options[36];
};

static dhcps_t dhcps;

// Function to send OFFER or ACK
static void send_dhcp_reply(const ip_addr_t *addr, uint32_t xid, uint8_t msg_type, uint8_t *chaddr)
{
    struct dhcp_msg msg;
    ip_addr_t dst_ip;

    memset(&msg, 0, sizeof(msg));

    IP_ADDR4(&dst_ip, 192 ,168 , 0 , 80);

    msg.payload.op = 2;
    msg.payload.htype = 1;
    msg.payload.hlen = 6;
    msg.payload.xid = xid;
    msg.payload.ciaddr = 0;
    msg.payload.yiaddr = dst_ip;
    memcpy(&msg.payload.siaddr, &dhcps.ip_addr, 4);
    memcpy(&msg.payload.chaddr, chaddr, 6);
    msg.payload.magic = htonl(DHCPMAGIC);

    uint8_t *popt = msg.options;

    /* Config option(53) */
    *popt++ = 53; *popt++ = 1; *popt++ = msg_type;

    /* Config option(54) */
    uint8_t *pton = (uint8_t*)&dhcps.ip_addr;
    *popt++ = 54;
    *popt++ = 4;
    *popt++ = pton[0];
    *popt++ = pton[1];
    *popt++ = pton[2];
    *popt++ = pton[3];

    /* Config option(51) */
    *popt++ = 51;
    *popt++ = 4;
    *popt++ = 0;
    *popt++ = 0;
    *popt++ = 0x0e;
    *popt++ = 0x10;

    /* Config option(58) */

    /* Config option(59) */

    /* Config option(1) */
    *popt++ = 1;
    *popt++ = 4;
    *popt++ = 255;
    *popt++ = 255;
    *popt++ = 255;
    *popt++ = 0;

    /* Config option(28) */

    /* Config option(3) */

    /* END */
    *popt++ = 255;

    struct pbuf *reply = pbuf_alloc(PBUF_TRANSPORT, sizeof(msg), PBUF_RAM);
    if (reply) {
        memcpy(reply->payload, &msg, sizeof(msg));
        udp_sendto(dhcps.pcb, reply, addr, DHCP_CLIENT_PORT);
        pbuf_free(reply);
    }
}

// Callback when packet is received
static void dhcp_recv_cb(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                         const ip_addr_t *addr, u16_t port)
{
    if (p && p->len >= sizeof(struct dhcp_payload)) {
        dhcps.pending_buf = p;
        dhcps.pending_addr = *addr;
        xSemaphoreGiveFromISR(dhcps.sem, NULL);
    } else if (p) {
        pbuf_free(p);
    }
}

// Public: DHCP server thread (FreeRTOS task)
static void dhcp_server_thread(const void *arg)
{
    while (1) {
        if (xSemaphoreTake(dhcps.sem, portMAX_DELAY) == pdTRUE && dhcps.pending_buf) {
            struct dhcp_msg *msg = (struct dhcp_msg *)dhcps.pending_buf->payload;

            uint8_t dhcp_type = 0;
            /* Search for option(53) */
            for (int i = 0; i < DHCP_MSG_LEN;) {
                if (msg->options[i] == 53) {
                    dhcp_type = msg->options[i + 2];
                    break;
                }

                i = msg->options[i + 1] + 2;
            }

            if (dhcp_type == DHCPDISCOVER) {
                printf("Received DHCPDISCOVER\n");
                send_dhcp_reply(&dhcps.pending_addr, msg->payload.xid, DHCPOFFER, msg->payload.chaddr);
            } else if (dhcp_type == DHCPREQUEST) {
                printf("Received DHCPREQUEST\n");
                send_dhcp_reply(&dhcps.pending_addr, msg->payload.xid, DHCPACK, msg->payload.chaddr);
            }

            pbuf_free(dhcps.pending_buf);
            dhcps.pending_buf = NULL;
        }
    }
}


// Public: DHCP server initialization
osStatus dhcp_server_init(ip_addr_t addr)
{
    dhcps.sem = xSemaphoreCreateBinary();
    if (dhcps.sem == NULL) {
        printf("Failed to create DHCP semaphore\n");
        return osErrorOS;
    }

    dhcps.pcb = udp_new();
    if (dhcps.pcb == NULL) {
        printf("Failed to create DHCP PCB\n");
        vSemaphoreDelete(dhcps.sem);
        return osErrorOS;
    }

    if (udp_bind(dhcps.pcb, IP_ADDR_ANY, DHCP_SERVER_PORT) != ERR_OK) {
        printf("Failed to bind DHCP PCB\n");
        return osErrorOS;
    }

    udp_recv(dhcps.pcb, dhcp_recv_cb, NULL);

    dhcps.ip_addr = addr;

    return osOK;
}

osStatus dhcp_server_start(void)
{
    osThreadDef(dhcp_server, dhcp_server_thread, osPriorityAboveNormal, 0, configMINIMAL_STACK_SIZE * 2);
    dhcps.threadid = osThreadCreate(osThread(dhcp_server), NULL);

    if(dhcps.threadid == NULL){
        printf("Failed to create DHCP server task\n");
        return osErrorOS;
    }

    printf("DHCP Server: Listening on port %d\n", DHCP_SERVER_PORT);
    return osOK;
}

void dhcp_server_stop(void)
{
    printf("DHCP Server: stopped\n");
}

void dhcp_server_cleanup(void)
{
    osThreadTerminate(dhcps.threadid);

    if(dhcps.pcb){
        udp_remove(dhcps.pcb);
        dhcps.pcb = NULL;
    }

    if(dhcps.sem){
        vSemaphoreDelete(dhcps.sem);
        dhcps.sem = NULL;
    }
}