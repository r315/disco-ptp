#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__


#include "lwip/err.h"
#include "lwip/netif.h"
#include "cmsis_os.h"

#define ETH_PTP_FLAG_TSARU        ((uint32_t)0x00000020)  /*!< Addend Register Update */
#define ETH_PTP_FLAG_TSITE        ((uint32_t)0x00000010)  /*!< Time Stamp Interrupt Trigger */
#define ETH_PTP_FLAG_TSSTU        ((uint32_t)0x00000008)  /*!< Time Stamp Update */
#define ETH_PTP_FLAG_TSSTI        ((uint32_t)0x00000004)  /*!< Time Stamp Initialize */

#define ETH_PTP_FLAG_TSTTR        ((uint32_t)0x10000002)  /* Time stamp target time reached */
#define ETH_PTP_FLAG_TSSO         ((uint32_t)0x10000001)  /* Time stamp seconds overflow */

#define ETH_PTP_PositiveTime      ((uint32_t)0x00000000)  /*!< Positive time value */
#define ETH_PTP_NegativeTime      ((uint32_t)0x80000000)  /*!< Negative time value */

/* Examples of subsecond increment and addend values using SysClk = 200 MHz

   Addend * Increment = 2^63 / SysClk

   ptp_tick = Increment * 10^9 / 2^31

   +-----------+-----------+------------+
   | ptp tick  | Increment | Addend     |
   +-----------+-----------+------------+
   | 20.023 ns |    43     | 0x3FECD300 |
   +-----------+-----------+------------+
*/

#define ADJ_FREQ_BASE_ADDEND      0x3FECD300
#define ADJ_FREQ_BASE_INCREMENT   43


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
