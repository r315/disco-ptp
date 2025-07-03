#include <string.h>
#include <stdio.h>
#include "lwip/opt.h"
#include "lwip/icmp.h"
#include "lwip/ip.h"
#include "lwip/ip_addr.h"
#include "lwip/inet.h"
#include "lwip/raw.h"
#include "lwip/sys.h"
#include "lwip/timeouts.h"
#include "ping.h"


static int ping_result = -1;
static u16_t ping_seq_num = 1;

static u8_t ping_recv(void *arg, struct raw_pcb *pcb, struct pbuf *p, const ip_addr_t *addr)
{
    struct icmp_echo_hdr *iecho;

    if (p->tot_len >= (PBUF_IP_HLEN + sizeof(struct icmp_echo_hdr))) {
        iecho = (struct icmp_echo_hdr *)((u8_t *)p->payload + PBUF_IP_HLEN);

        if ((iecho->id == lwip_htons(PING_ID)) && (iecho->seqno == lwip_htons(ping_seq_num - 1))) {
            printf("Ping reply from %s: bytes=%d seq=%d\n",
                   ipaddr_ntoa(addr), p->tot_len, lwip_ntohs(iecho->seqno));
            ping_result = 0;
        }
    }

    pbuf_free(p);

    return 1; // We've handled the packet
}

static u16_t ping_checksum(void *data, int len)
{
    u32_t sum = 0;
    u16_t *ptr = data;

    for (; len > 1; len -= 2)
        sum += *ptr++;
    if (len == 1)
        sum += *(u8_t *)ptr;

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);

    return (u16_t)~sum;
}

static void ping_send(struct raw_pcb *pcb, const ip_addr_t *addr, u16_t seq_num)
{
    struct pbuf *p;
    struct icmp_echo_hdr *iecho;

    u16_t len = sizeof(struct icmp_echo_hdr) + PING_DATA_SIZE;

    p = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);

    if (!p) {
        printf("Failed to allocate pbuf for ping\n");
        return;
    }

    iecho = (struct icmp_echo_hdr *)p->payload;

    iecho->type = ICMP_ECHO;
    iecho->code = 0;
    iecho->chksum = 0;
    iecho->id = lwip_htons(PING_ID);
    iecho->seqno = lwip_htons(seq_num);

    uint8_t i;
    uint8_t* pdata;

    for (pdata = (uint8_t*)iecho, i = sizeof(struct icmp_echo_hdr); i < len; i++) {
        pdata[i] = i;
    }

    //iecho->chksum = ping_checksum(iecho, len);

    raw_sendto(pcb, p, addr);

    pbuf_free(p);
}

int do_ping(ip_addr_t *addr, u16_t count)
{
    struct raw_pcb *ping_pcb = raw_new(IP_PROTO_ICMP);

    if (!ping_pcb) {
        printf("Failed to create raw PCB\n");
        return -1;
    }

    raw_recv(ping_pcb, ping_recv, NULL);
    raw_bind(ping_pcb, IP_ADDR_ANY);

    do{
        ping_result = -1;
        ping_send(ping_pcb, addr, ping_seq_num++);

        // Wait for response or timeout
        u32_t start = sys_now();
        while ((sys_now() - start) < PING_TIMEOUT_MS) {
            sys_check_timeouts();  // Process lwIP timers
            if (ping_result == 0)
                break;
            sys_msleep(10);
        }

        if (ping_result == -1){
            printf("Ping timeout\n");
        }
    }while(--count);

    raw_remove(ping_pcb);

    return ping_result == 0 ? 0 : -1;
}

