#ifndef CONSTANTS_H_
#define CONSTANTS_H_

/**
 * \file
 * \brief Default values and constants used in ptpdv2
 *
 * This header file includes all default values used during initialization
 * and enumeration defined in the spec.
 */

/* 5.3.4 ClockIdentity */
#define CLOCK_IDENTITY_LENGTH 8

#define MANUFACTURER_ID \
		"PTPd;2.0.1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"

/* Ethernet protocol */
#define IEEE802_3            1
#define IPV4                2
#define PROTOCOL            IPV4    /**< Protocol in use: IEEE802_3 or IPv4 */

#if PROTOCOL == IEEE802_3
#define PTP_MCAST_MAC "\x01\x1b\x19\x0\x0\x0"  /**< PTP Multicast MAC (IEEE 1588-2008, L2 transport) */
#else
#define PTP_MCAST_MAC (const uint8_t*)"\x01\x00\x5e\x00\x01\x81" /**< PTP Multicast MAC for 224.0.1.129 (L4 transport) */
#endif

/* Implementation specific constants */
#define DEFAULT_INBOUND_LATENCY         0       /**< In nanoseconds */
#define DEFAULT_OUTBOUND_LATENCY        0       /**< In nanoseconds */
#define DEFAULT_NO_RESET_CLOCK          FALSE   /**< Servo reset clock */
#define DEFAULT_DOMAIN_NUMBER           0
#define DEFAULT_DELAY_MECHANISM         E2E
#define DEFAULT_AP                      0.5f
#define DEFAULT_AI                      0.08f
#define DEFAULT_DELAY_S                 6       /**< Exponential smoothing - 2^s */
#define DEFAULT_OFFSET_S                0       /**< Exponential smoothing - 2^s */
#define DEFAULT_ANNOUNCE_INTERVAL       2       /**< 0 in 802.1AS */
#define DEFAULT_UTC_OFFSET              0
#define DEFAULT_UTC_VALID               FALSE
#define DEFAULT_PDELAYREQ_INTERVAL      1       /**< -4 in 802.1AS */
#define DEFAULT_DELAYREQ_INTERVAL       0       /**< Between DEFAULT_SYNC_INTERVAL and DEFAULT_SYNC_INTERVAL + 5 */
#define DEFAULT_SYNC_INTERVAL           0       /**< -7 in 802.1AS */
#define DEFAULT_SYNC_RECEIPT_TIMEOUT    3
#define DEFAULT_ANNOUNCE_RECEIPT_TIMEOUT 6      /**< Default is 3 */
#define DEFAULT_QUALIFICATION_TIMEOUT   -9      /**< DEFAULT_ANNOUNCE_INTERVAL + N */
#define DEFAULT_FOREIGN_MASTER_TIME_WINDOW 4
#define DEFAULT_FOREIGN_MASTER_THRESHOLD 2
#define DEFAULT_CLOCK_CLASS             248
#define DEFAULT_CLOCK_CLASS_SLAVE_ONLY  255
#define DEFAULT_CLOCK_ACCURACY          0xFE
#define DEFAULT_PRIORITY1               128
#define DEFAULT_PRIORITY2               128
#define DEFAULT_CLOCK_VARIANCE          5000
#define DEFAULT_MAX_FOREIGN_RECORDS     5
#define DEFAULT_PARENTS_STATS           FALSE
#define DEFAULT_TWO_STEP_FLAG           TRUE    /**< Transmit SYNC or SYNC+FOLLOW_UP */
#define DEFAULT_TIME_SOURCE             GPS     /**< Time source */
#define DEFAULT_TIME_TRACEABLE          FALSE
#define DEFAULT_FREQUENCY_TRACEABLE     FALSE
#define DEFAULT_TIMESCALE               ARB_TIMESCALE
#define DEFAULT_TRANSPORT_SPECIFIC      0

#define DEFAULT_CALIBRATED_OFFSET_NS    10000       /**< Offset from master < 10us */
#define DEFAULT_UNCALIBRATED_OFFSET_NS  1000000     /**< Offset from master > 1000us */
#define MAX_ADJ_OFFSET_NS               100000000   /**< Max offset to adjust: < 100ms */

/* Feature support configuration */
#define NUMBER_PORTS      1
#define VERSION_PTP       2
#define BOUNDARY_CLOCK    FALSE
#define SLAVE_ONLY        FALSE
#define NO_ADJUST         FALSE

/**
 * \name Packet length
 * \brief Minimal length values for each message. TLV may increase length.
 * \{
 */
#define HEADER_LENGTH                 34
#define ANNOUNCE_LENGTH               64
#define SYNC_LENGTH                   44
#define FOLLOW_UP_LENGTH              44
#define PDELAY_REQ_LENGTH             54
#define DELAY_REQ_LENGTH              44
#define DELAY_RESP_LENGTH             54
#define PDELAY_RESP_LENGTH            54
#define PDELAY_RESP_FOLLOW_UP_LENGTH  54
#define MANAGEMENT_LENGTH             48
/** \} */

/** \brief Domain Number (Table 2 in the spec) */
enum
{
	DFLT_DOMAIN_NUMBER = 0,
	ALT1_DOMAIN_NUMBER,
	ALT2_DOMAIN_NUMBER,
	ALT3_DOMAIN_NUMBER
};

/** \brief Network Protocol (Table 3 in the spec) */
enum
{
	UDP_IPV4 = 1,
	UDP_IPV6,
	IEE_802_3,
	DeviceNet,
	ControlNet,
	PROFINET
};

/** \brief Time Source (Table 7 in the spec) */
enum
{
	ATOMIC_CLOCK = 0x10,        /**< Atomic time source */
	GPS = 0x20,                 /**< GPS time source */
	TERRESTRIAL_RADIO = 0x30,   /**< Terrestrial radio time source */
	PTP = 0x40,                 /**< PTP time source */
	NTP = 0x50,                 /**< NTP time source */
	HAND_SET = 0x60,            /**< Handset time source */
	OTHER = 0x90,               /**< Other time source */
	INTERNAL_OSCILLATOR = 0xA0  /**< Internal time source */
};

/** \brief PTP State (Table 8 in the spec) */
enum
{
	PTP_INITIALIZING = 0,
	PTP_FAULTY,
	PTP_DISABLED,
	PTP_LISTENING,
	PTP_PRE_MASTER,
	PTP_MASTER,
	PTP_PASSIVE,
	PTP_UNCALIBRATED,
	PTP_SLAVE,
	PTP_SHUTDOWN
};

/** \brief Delay mechanism (Table 9 in the spec) */
enum
{
	E2E = 1,
	P2P = 2,
	DELAY_DISABLED = 0xFE
};

/** \brief PTP timers */
enum
{
	PDELAYREQ_INTERVAL_TIMER = 0,  /**< Timer handling the PdelayReq Interval */
	DELAYREQ_INTERVAL_TIMER,       /**< Timer handling the delayReq Interval */
	SYNC_INTERVAL_TIMER,           /**< Timer handling Sync interval */
	ANNOUNCE_RECEIPT_TIMER,        /**< Timer for announce receipt timeout */
	ANNOUNCE_INTERVAL_TIMER,       /**< Timer for announce interval */
	QUALIFICATION_TIMEOUT,
	TIMER_ARRAY_SIZE               /**< Non-spec: total number of timers */
};

/** \brief PTP Messages (Table 19) */
enum
{
	SYNC = 0x0,
	DELAY_REQ,
	PDELAY_REQ,
	PDELAY_RESP,
	FOLLOW_UP = 0x8,
	DELAY_RESP,
	PDELAY_RESP_FOLLOW_UP,
	ANNOUNCE,
	SIGNALING,
	MANAGEMENT
};

/** \brief PTP Messages control field (Table 23) */
enum
{
	CTRL_SYNC = 0x00,
	CTRL_DELAY_REQ,
	CTRL_FOLLOW_UP,
	CTRL_DELAY_RESP,
	CTRL_MANAGEMENT,
	CTRL_OTHER
};

/** \brief Output statistics mode */
enum
{
	PTP_NO_STATS = 0,
	PTP_TEXT_STATS,
	PTP_CSV_STATS /**< Not implemented */
};

/** \brief Message flags - byte 0 */
enum
{
	FLAG0_ALTERNATE_MASTER         = 0x01,
	FLAG0_TWO_STEP                 = 0x02,
	FLAG0_UNICAST                  = 0x04,
	FLAG0_PTP_PROFILE_SPECIFIC_1  = 0x20,
	FLAG0_PTP_PROFILE_SPECIFIC_2  = 0x40,
	FLAG0_SECURITY                 = 0x80
};

/** \brief Message flags - byte 1 */
enum
{
	FLAG1_LEAP61               = 0x01,
	FLAG1_LEAP59               = 0x02,
	FLAG1_UTC_OFFSET_VALID     = 0x04,
	FLAG1_PTP_TIMESCALE        = 0x08,
	FLAG1_TIME_TRACEABLE       = 0x10,
	FLAG1_FREQUENCY_TRACEABLE  = 0x20
};

/** \brief PTP stack events */
enum
{
	POWERUP                          = 0x0001,
	INITIALIZE                       = 0x0002,
	DESIGNATED_ENABLED               = 0x0004,
	DESIGNATED_DISABLED              = 0x0008,
	FAULT_CLEARED                    = 0x0010,
	FAULT_DETECTED                   = 0x0020,
	STATE_DECISION_EVENT             = 0x0040,
	QUALIFICATION_TIMEOUT_EXPIRES   = 0x0080,
	ANNOUNCE_RECEIPT_TIMEOUT_EXPIRES= 0x0100,
	SYNCHRONIZATION_FAULT           = 0x0200,
	MASTER_CLOCK_SELECTED           = 0x0400,
	MASTER_CLOCK_CHANGED            = 0x0800 /**< Non-spec */
};

/** \brief PTP time scale */
enum
{
	ARB_TIMESCALE,
	PTP_TIMESCALE
};

#endif /* CONSTANTS_H_ */
