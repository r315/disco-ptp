/* ptpd.c */

#include "ptpd.h"
#include "syslog.h"

#define PTPD_THREAD_PRIO    (tskIDLE_PRIORITY + 2)

static sys_mbox_t ptp_alert_queue;
static osThreadId PTPTaskHandle;

// Statically allocated run-time configuration data.
static PtpClock ptpClock;
static RunTimeOpts rtOpts;
static ForeignMasterRecord ptpForeignRecords[DEFAULT_MAX_FOREIGN_RECORDS];

__IO uint32_t PTPTimer = 0;

static void ptpd_thread(void const *arg)
{
	// Initialize run-time options to default values.
	rtOpts.announceInterval = DEFAULT_ANNOUNCE_INTERVAL;
	rtOpts.syncInterval = DEFAULT_SYNC_INTERVAL;
	rtOpts.clockQuality.clockAccuracy = DEFAULT_CLOCK_ACCURACY;
	rtOpts.clockQuality.clockClass = DEFAULT_CLOCK_CLASS;
	rtOpts.clockQuality.offsetScaledLogVariance = DEFAULT_CLOCK_VARIANCE; /* 7.6.3.3 */
	rtOpts.priority1 = DEFAULT_PRIORITY1;
	rtOpts.priority2 = DEFAULT_PRIORITY2;
	rtOpts.domainNumber = DEFAULT_DOMAIN_NUMBER;
	rtOpts.slaveOnly = SLAVE_ONLY;
	rtOpts.currentUtcOffset = DEFAULT_UTC_OFFSET;
	rtOpts.servo.noResetClock = DEFAULT_NO_RESET_CLOCK;
	rtOpts.servo.noAdjust = NO_ADJUST;
	rtOpts.inboundLatency.nanoseconds = DEFAULT_INBOUND_LATENCY;
	rtOpts.outboundLatency.nanoseconds = DEFAULT_OUTBOUND_LATENCY;
	rtOpts.servo.sDelay = DEFAULT_DELAY_S;
	rtOpts.servo.sOffset = DEFAULT_OFFSET_S;
	rtOpts.servo.ap = DEFAULT_AP;
	rtOpts.servo.ai = DEFAULT_AI;
	rtOpts.maxForeignRecords = sizeof(ptpForeignRecords) / sizeof(ptpForeignRecords[0]);
	rtOpts.stats = PTP_TEXT_STATS;
	rtOpts.delayMechanism = DEFAULT_DELAY_MECHANISM;

	// Initialize run time options.
	if (ptpdStartup(&ptpClock, &rtOpts, ptpForeignRecords) != 0)
	{
		LOG_INF("PTPD: startup failed");
		return;
	}

	LOG_INF("PTP thread ready.\n");

#ifdef USE_DHCP
	// If DHCP, wait until the default interface has an IP address.
	while (!netif_default->ip_addr.addr)
	{
		// Sleep for 500 milliseconds.
		sys_msleep(500);
	}
#endif

	// Loop forever.
	for (;;)
	{
		void *msg;

		// Process the current state.
		do
		{
			// doState() has a switch for the actions and events to be
			// checked for 'port_state'. The actions and events may or may not change
			// 'port_state' by calling toState(), but once they are done we loop around
			// again and perform the actions required for the new 'port_state'.
			doState(&ptpClock);
		}
		while (netSelect(&ptpClock.netPath, 0) > 0);
		// Wait up to 100ms for something to do, then do something anyway.
		sys_arch_mbox_fetch(&ptp_alert_queue, &msg, 100);
	}
}


static void ptpd_displayStats(const PtpClock *ptpClock)
{
	const char *s;
	unsigned char *uuid;
	char sign;

	uuid = (unsigned char*) ptpClock->parentDS.parentPortIdentity.clockIdentity;

	/* Master clock UUID */
	LOG_PRINT("\tUID: %02X%02X:%02X%02X:%02X%02X:%02X%02X",
					uuid[0], uuid[1],
					uuid[2], uuid[3],
					uuid[4], uuid[5],
					uuid[6], uuid[7]);

	switch (ptpClock->portDS.portState)
	{
		case PTP_INITIALIZING:  s = "init";  break;
		case PTP_FAULTY:        s = "faulty";   break;
		case PTP_LISTENING:     s = "listening";  break;
		case PTP_PASSIVE:       s = "passive";  break;
		case PTP_UNCALIBRATED:  s = "uncalibrated";  break;
		case PTP_SLAVE:         s = "slave";   break;
		case PTP_PRE_MASTER:    s = "pre master";  break;
		case PTP_MASTER:        s = "master";   break;
		case PTP_DISABLED:      s = "disabled";  break;
		default:                s = "?";     break;
	}

	/* State of the PTP */
	LOG_PRINT("\tstate: %s", s);

	/* One way delay */
	switch (ptpClock->portDS.delayMechanism)
	{
		case E2E:
			LOG_PRINT("\tpath delay: %d nsec", (int)ptpClock->currentDS.meanPathDelay.nanoseconds);
			break;
		case P2P:
			LOG_PRINT("\tpath delay: %d nsec", (int)ptpClock->portDS.peerMeanPathDelay.nanoseconds);
			break;
		default:
			LOG_PRINT("\tpath delay: unknown");
			/* none */
			break;
	}

	/* Offset from master */
	if (ptpClock->currentDS.offsetFromMaster.seconds)
	{
		LOG_PRINT("\toffset: %d sec", (int)ptpClock->currentDS.offsetFromMaster.seconds);
	}
	else
	{
		LOG_PRINT("\toffset: %d nsec", (int)ptpClock->currentDS.offsetFromMaster.nanoseconds);
	}

	/* Observed drift from master */
	sign = ' ';
	if (ptpClock->observedDrift > 0) sign = '+';
	if (ptpClock->observedDrift < 0) sign = '-';

	LOG_PRINT("\tdrift: %c%d.%03d ppm", sign, abs(ptpClock->observedDrift / 1000), abs(ptpClock->observedDrift % 1000));
}

// Notify the PTP thread of a pending operation.
void ptpd_alert(void)
{
	// Send a message to the alert queue to wake up the PTP thread.
	sys_mbox_trypost(&ptp_alert_queue, NULL);
}

osThreadId ptpd_init(void)
{
	// Create the alert queue mailbox.
    if (sys_mbox_new(&ptp_alert_queue, 8) != ERR_OK){
        LOG_INF("PTPD: failed to create ptp_alert_queue mbox");
    }

	// Create the PTP daemon thread.
  	osThreadDef(PTPD, ptpd_thread, osPriorityAboveNormal, 0, DEFAULT_THREAD_STACKSIZE * 2);
  	PTPTaskHandle = osThreadCreate(osThread(PTPD), NULL);
	//sys_thread_t id = sys_thread_new("PTPD", ptpd_thread, NULL, DEFAULT_THREAD_STACKSIZE * 2, osPriorityAboveNormal);
  	return PTPTaskHandle;
}

void ptpd_stats(void)
{
    ptpd_displayStats(&ptpClock);
}
