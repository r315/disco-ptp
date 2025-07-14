#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__


#include "lwip/err.h"
#include "lwip/netif.h"
#include "cmsis_os.h"
#include "datatypes.h"

#define ETH_PTP_FLAG_TSARU        ((uint32_t)0x00000020)  /*!< Addend Register Update */
#define ETH_PTP_FLAG_TSITE        ((uint32_t)0x00000010)  /*!< Time Stamp Interrupt Trigger */
#define ETH_PTP_FLAG_TSSTU        ((uint32_t)0x00000008)  /*!< Time Stamp Update */
#define ETH_PTP_FLAG_TSSTI        ((uint32_t)0x00000004)  /*!< Time Stamp Initialize */

#define ETH_PTP_FLAG_TSTTR        ((uint32_t)0x10000002)  /* Time stamp target time reached */
#define ETH_PTP_FLAG_TSSO         ((uint32_t)0x10000001)  /* Time stamp seconds overflow */

#define ETH_PTP_PositiveTime      ((uint32_t)0x00000000)  /*!< Positive time value */
#define ETH_PTP_NegativeTime      ((uint32_t)0x80000000)  /*!< Negative time value */

#define ETH_PTP_ROLLOVER_DIGITAL    1

#if ETH_PTP_ROLLOVER_DIGITAL
/**
 * With digital rollover (TSSSR=1), it is recommended not to use the PPS output with a
 * frequency other than 1 Hz as it would have irregular waveforms (though its average frequency
 * would always be correct during any one-second window).
 *
 * Addend * increment = (2^32 * 10^9) / SysClk
 *
 * ptp_tick = Increment

   +-----------+-----------+------------+
   | ptp tick  | Increment | Addend     |
   +-----------+-----------+------------+
   | 20.0 ns   |    20     | 0x40000000 |
   +-----------+-----------+------------+
 * */

#define ADJ_FREQ_BASE_ADDEND    0x40000000
#define ADJ_FREQ_BASE_INCREMENT 20
#define ETH_PTP_ROLLOVER_MODE   1  /* subsecond counter counts to 1000 000 000 */

#else
/*
    Example of subsecond increment and addend values
    for binary rollover (TSSSR = 0) using SysClk = 200 MHz

   Addend * Increment = 2^(32 + 31) / SysClk

   ptp_tick = Increment * 10^9 / 2^31
   <=> ptp_tick = Increment * 0,467

   +-----------+-----------+------------+
   | ptp tick  | Increment | Addend     |
   +-----------+-----------+------------+
   | 20.023 ns |    43     | 0x3FECD300 |
   +-----------+-----------+------------+
*/

#define ADJ_FREQ_BASE_ADDEND    0x3FECD300
#define ADJ_FREQ_BASE_INCREMENT 43
#define ETH_PTP_ROLLOVER_MODE   1
#define ETH_PTP_ROLLOVER_MODE   0  /* subsecond counter counts to 2147483648 (0x8000 0000)*/

#endif

/* Within 'USER CODE' section, code will be kept by default at each generation */
/* USER CODE BEGIN 0 */
struct ptptime_t {
  s32_t tv_sec;
  s32_t tv_nsec;
};

/* Exported types ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
err_t ethernetif_init(struct netif *netif);
void ethernetif_ptp_init(void);
void ethernetif_ptp_set_pps_output(uint8_t freq);
void ethernetif_ptp_set_time(struct ptptime_t * timestamp);
void ethernetif_ptp_get_time(struct ptptime_t * timestamp);
void ethernetif_ptp_update_offset(struct ptptime_t * timeoffset);
void ethernetif_ptp_adj_freq(int32_t Adj);
void ethernetif_ptp_get_tx_timestamp(TimeInternal *time);
#endif
