#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__


#include "lwip/err.h"
#include "lwip/netif.h"
#include "cmsis_os.h"

/* Within 'USER CODE' section, code will be kept by default at each generation */
/* USER CODE BEGIN 0 */
struct ptptime_t {
  s32_t tv_sec;
  s32_t tv_nsec;
};

/* Exported types ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
err_t ethernetif_init(struct netif *netif);

void ethernetif_ptp_set_pps_output(uint8_t freq);
void ethernetif_ptp_set_time(struct ptptime_t * timestamp);
void ethernetif_ptp_get_time(struct ptptime_t * timestamp);
void ethernetif_ptp_update_offset(struct ptptime_t * timeoffset);
void ethernetif_ptp_adj_freq(int32_t Adj);
#endif
