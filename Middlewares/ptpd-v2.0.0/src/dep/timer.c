/* timer.c */

#include "ptpd.h"

#ifdef PTPD_TIMER_DBG
#define TIMER_DBG DBG
#else
#define TIMER_DBG(...)
#endif

/* An array to hold the various system timer handles. */
static PTP_TIMER ptpdTimers[TIMER_ARRAY_SIZE];
static bool ptpdTimersExpired[TIMER_ARRAY_SIZE];

static void timerCallback(PTP_TIMER timer)
{
	int index = (int)PTP_TIMER_GETID(timer);
	TIMER_DBG("timerCallback ID:%d\n",index);
	// Sanity check the index.
	if (index < TIMER_ARRAY_SIZE)
	{
		/* Mark the indicated timer as expired. */
		ptpdTimersExpired[index] = TRUE;

		/* Notify the PTP thread of a pending operation. */
		ptpd_alert();
	}
}

void initTimer(void)
{
	int32_t i;

	TIMER_DBG("initTimer\n");

	/* Create the various timers used in the system. */
  for (i = 0; i < TIMER_ARRAY_SIZE; i++)
  {
		// Mark the timer as not expired.
		// Initialize the timer.
		//sys_timer_new(&ptpdTimers[i], timerCallback, osTimerOnce, (void *) i);
		ptpdTimers[i] = PTP_TIMER_NEW(timerCallback, osTimerPeriodic, (void *) i);
		TIMER_DBG("Timer: %x \tID: %x\n", (unsigned int)ptpdTimers[i], (unsigned int)PTP_TIMER_GETID(ptpdTimers[i]));
		ptpdTimersExpired[i] = FALSE;
	}
}

void timerStop(int32_t index)
{
	/* Sanity check the index. */
	if (index >= TIMER_ARRAY_SIZE) return;

	// Cancel the timer and reset the expired flag.
	TIMER_DBG("timerStop: stop timer %d\n", (int)index);
  	//sys_timer_stop(&ptpdTimers[index]);
	PTP_TIMER_STOP(ptpdTimers[index]);

	ptpdTimersExpired[index] = FALSE;
}

void timerStart(int32_t index, uint32_t interval_ms)
{
	/* Sanity check the index. */
	if (index >= TIMER_ARRAY_SIZE) return;

	// Set the timer duration and start the timer.
	TIMER_DBG("timerStart: set timer %d to %d\n", (int)index, (int)interval_ms);
	ptpdTimersExpired[index] = FALSE;
  	//sys_timer_start(&ptpdTimers[index], interval_ms);
	PTP_TIMER_START(ptpdTimers[index],interval_ms);
}

bool timerExpired(int32_t index)
{
	/* Sanity check the index. */
	if (index >= TIMER_ARRAY_SIZE) return FALSE;

	/* Determine if the timer expired. */
	if (!ptpdTimersExpired[index]) return FALSE;
	TIMER_DBG("timerExpired: timer %d expired\n", (int)index);
	ptpdTimersExpired[index] = FALSE;

	return TRUE;
}

void shutdownTimer(void)
{
    int32_t i;

	TIMER_DBG("%s\n", __func__);

    for (i = 0; i < TIMER_ARRAY_SIZE; i++)
    {
		timerStop(i);
	}
}