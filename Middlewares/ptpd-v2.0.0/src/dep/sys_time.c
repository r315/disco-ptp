/* sys.c */

#include "ptpd.h"

/**
  * @brief  Returns PTP time in seconds and nanoseconds
  *
  * @param  time
  *
  * @retval Nore
  */
void getTime(TimeInternal *time)
{
	struct ptptime_t timestamp;
	ethernetif_ptp_get_time(&timestamp);
	time->seconds = timestamp.tv_sec;
	time->nanoseconds = timestamp.tv_nsec;
}

void setTime(const TimeInternal *time)
{
	struct ptptime_t ts;

	ts.tv_sec = time->seconds;
	ts.tv_nsec = time->nanoseconds;
	ethernetif_ptp_set_time(&ts);
	DBG("Setting system clock to %d sec %d nsec\n", (int)time->seconds, (int)time->nanoseconds);
}

void updateTime(const TimeInternal *time)
{
	struct ptptime_t timeoffset;

	DBGV("updateTime: %d sec %d nsec\n", (int)time->seconds, (int)time->nanoseconds);

	timeoffset.tv_sec = -time->seconds;
	timeoffset.tv_nsec = -time->nanoseconds;

	/* Coarse update method */
	ethernetif_ptp_update_offset(&timeoffset);
	DBGV("updateTime: updated\n");
}

uint32_t getRand(uint32_t randMax)
{
    // TODO: Use HW module
	return rand() % randMax;
}

bool adjTime(int32_t adj)
{
	DBGV("adjTime %d\n", (int)adj);

	if (adj > ADJ_FREQ_MAX)
		adj = ADJ_FREQ_MAX;
	else if (adj < -ADJ_FREQ_MAX)
		adj = -ADJ_FREQ_MAX;

	/* Fine update method */
	ethernetif_ptp_adj_freq(adj);

	return TRUE;
}

/**
  * @brief  Returns the current time in milliseconds
  *         when LWIP_TIMERS == 1 and NO_SYS == 1
  * @param  None
  * @retval Time
  */
u32_t sys_now(void)
{
  return HAL_GetTick();
}
