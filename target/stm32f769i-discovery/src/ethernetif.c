/**
  ******************************************************************************
  * @file    LwIP/LwIP_HTTP_Server_Socket_RTOS/Src/ethernetif.c
  * @author  MCD Application Team
  * @brief   This file implements Ethernet network interface drivers for lwIP
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V.
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice,
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other
  *    contributors to this software may be used to endorse or promote products
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under
  *    this license is void and will automatically terminate your rights under
  *    this license.
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "stm32f7xx_hal.h"
#include "lwip/opt.h"
#include "lwip/memp.h"
#include "lwip/timeouts.h"
#include "netif/ethernet.h"
#include "netif/etharp.h"
#include "ethernetif.h"
#include "ptpd.h"
#include "lan8742/lan8742.h"

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
    struct pbuf_custom pbuf_custom;
    uint8_t buff[(ETH_RX_BUF_SIZE + 31) & ~31];
} RxBuff_t;
/* Private define ------------------------------------------------------------*/
/* The time to block waiting for input. */
#define TIME_WAITING_FOR_INPUT                 ( osWaitForever )
/* Stack size of the interface thread */
#define INTERFACE_THREAD_STACK_SIZE            ( 350 )
#define ETHIF_TX_TIMEOUT                       (2000U)

/* Define those to better describe your network interface. */
#define IFNAME0 's'
#define IFNAME1 't'
#define ETH_DMA_TRANSMIT_TIMEOUT               (20U)

/* This app buffers receive packets of its primary service protocol for processing later. */
#define ETH_RX_BUFFER_CNT                      10
/* HAL_ETH_Transmit(_IT) may attach two buffers per descriptor. */
#define ETH_TX_BUFFER_MAX                      ((ETH_TX_DESC_CNT) * 2)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/*
  @Note: The DMARxDscrTab and DMATxDscrTab must be declared in a device non cacheable memory region
         In this example they are declared in the first 256 Byte of SRAM2 memory, so this
         memory region is configured by MPU as a device memory.

         In this example the ETH buffers are located in the SRAM2 with MPU configured as normal
         not cacheable memory.

         Please refer to MPU_Config() in main.c file.
 */
#if defined ( __CC_ARM   )
ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT] __attribute__((at(0x2007C000)));/* Ethernet Rx DMA Descriptors */
ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT] __attribute__((at(0x2007C0A0)));/* Ethernet Tx DMA Descriptors *//
#elif defined ( __ICCARM__ ) /*!< IAR Compiler */
#pragma data_alignment=4
#pragma location=0x2007C000
__no_init ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT];/* Ethernet Rx DMA Descriptors */
#pragma location=0x2007C0A0
__no_init ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT];/* Ethernet Tx DMA Descriptors */
#elif defined ( __GNUC__ ) /*!< GNU Compiler */
static ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT] __attribute__((section(".RxDescripSection")));/* Ethernet Rx DMA Descriptors */
static ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT] __attribute__((section(".TxDescripSection")));/* Ethernet Tx DMA Descriptors */
static LWIP_MEMPOOL_DECLARE(RX_POOL, ETH_RX_BUFFER_CNT, sizeof(RxBuff_t), "Zero-copy RX PBUF pool");
#endif

static osSemaphoreId RxPktSemaphore;
static osSemaphoreId TxPktSemaphore;
static ETH_TxPacketConfigTypeDef TxConfig;
static lan8742_Object_t LAN8742;
static uint8_t RxAllocStatus;
/* Global variables ---------------------------------------------------------*/
ETH_HandleTypeDef EthHandle;

/* Private function prototypes -----------------------------------------------*/
static void ethernetif_input_thread( void const * argument );
static void RMII_Thread( void const * argument );

/* Private functions ---------------------------------------------------------*/
static int32_t ETH_PHY_IO_Init(void) {HAL_ETH_SetMDIOClockRange(&EthHandle); return 0;}
static int32_t ETH_PHY_IO_DeInit (void){return 0;}
static int32_t ETH_PHY_IO_ReadReg(uint32_t DevAddr, uint32_t RegAddr, uint32_t *pRegVal){
    return (HAL_ETH_ReadPHYRegister(&EthHandle, DevAddr, RegAddr, pRegVal) == HAL_OK) ? 0 : -1;
}
static int32_t ETH_PHY_IO_WriteReg(uint32_t DevAddr, uint32_t RegAddr, uint32_t RegVal){
    return (HAL_ETH_WritePHYRegister(&EthHandle, DevAddr, RegAddr, RegVal) == HAL_OK) ? 0 : -1;
}
static int32_t ETH_PHY_IO_GetTick(void) { return HAL_GetTick(); }
static const lan8742_IOCtx_t  LAN8742_IOCtx = {
    ETH_PHY_IO_Init,
    ETH_PHY_IO_DeInit,
    ETH_PHY_IO_WriteReg,
    ETH_PHY_IO_ReadReg,
    ETH_PHY_IO_GetTick
};

static uint32_t subsecond_to_nanosecond(uint32_t SubSecondValue)
{
  uint64_t val = SubSecondValue * 1000000000ll;
  val >>= 31;
  return val;
}

static void pbuf_free_custom(struct pbuf *p)
{
    struct pbuf_custom* custom_pbuf = (struct pbuf_custom*)p;
    LWIP_MEMPOOL_FREE(RX_POOL, custom_pbuf);

    if (RxAllocStatus == RX_ALLOC_ERROR){
        RxAllocStatus = RX_ALLOC_OK;
        osSemaphoreRelease(RxPktSemaphore);
    }
}

/*******************************************************************************
                       LL Driver Interface ( LwIP stack --> ETH)
*******************************************************************************/
static void low_level_init_phy(struct netif *netif)
{
    uint32_t duplex, speed;
    int32_t PHYLinkState;
    ETH_MACConfigTypeDef MACConf;

    /* Set PHY IO functions */
    LAN8742_RegisterBusIO(&LAN8742, &LAN8742_IOCtx);

    LAN8742_DeInit(&LAN8742);

    /* Initialize the LAN8742 ETH PHY */
    if(LAN8742_Init(&LAN8742) != LAN8742_STATUS_OK){
        netif_set_link_down(netif);
        netif_set_down(netif);
        return;
    }
    /* Wait for link */
    osDelay(3000);

    PHYLinkState = LAN8742_GetLinkState(&LAN8742);

    /* Get link state */
    if(PHYLinkState <= LAN8742_STATUS_LINK_DOWN) {
        netif_set_link_down(netif);
        netif_set_down(netif);
    } else {
        switch (PHYLinkState) {
            case LAN8742_STATUS_100MBITS_FULLDUPLEX:
                duplex = ETH_FULLDUPLEX_MODE;
                speed = ETH_SPEED_100M;
                break;
            case LAN8742_STATUS_100MBITS_HALFDUPLEX:
                duplex = ETH_HALFDUPLEX_MODE;
                speed = ETH_SPEED_100M;
                break;
            case LAN8742_STATUS_10MBITS_FULLDUPLEX:
                duplex = ETH_FULLDUPLEX_MODE;
                speed = ETH_SPEED_10M;
                break;
            case LAN8742_STATUS_10MBITS_HALFDUPLEX:
                duplex = ETH_HALFDUPLEX_MODE;
                speed = ETH_SPEED_10M;
                break;
            default:
                duplex = ETH_FULLDUPLEX_MODE;
                speed = ETH_SPEED_100M;
                break;
        }

        /* Get MAC Config MAC */
        HAL_ETH_GetMACConfig(&EthHandle, &MACConf);
        MACConf.DuplexMode = duplex;
        MACConf.Speed = speed;
        HAL_ETH_SetMACConfig(&EthHandle, &MACConf);
        HAL_ETH_Start_IT(&EthHandle);
        netif_set_up(netif);
        netif_set_link_up(netif);
    }

    /* Check if it's a STM32F76xxx Revision A */
    if(HAL_GetREVID() == 0x1000) {
        /*
            This thread will keep resetting the RMII interface until good frames are received
        */
        osThreadDef(RMII_Watchdog, RMII_Thread, osPriorityRealtime, 0, configMINIMAL_STACK_SIZE);
        osThreadCreate (osThread(RMII_Watchdog), NULL);
    }
}
/**
  * @brief In this function, the hardware should be initialized.
  * Called from ethernetif_init().
  *
  * @param netif the already initialized lwip network interface structure
  *        for this ethernetif
  */
static void low_level_init(struct netif *netif)
{
    uint8_t macaddress[6]= { MAC_ADDR0, MAC_ADDR1, MAC_ADDR2, MAC_ADDR3, MAC_ADDR4, MAC_ADDR5 };

    EthHandle.Instance = ETH;
    EthHandle.Init.MACAddr = macaddress;
    EthHandle.Init.MediaInterface = ETH_MEDIA_INTERFACE_RMII;
    EthHandle.Init.RxDesc = DMARxDscrTab;
    EthHandle.Init.TxDesc = DMATxDscrTab;
    EthHandle.Init.RxBuffLen = ETH_RX_BUF_SIZE;

    /* configure ethernet peripheral (GPIOs, clocks, MAC, DMA) */
    HAL_ETH_Init(&EthHandle);

    /* set netif MAC hardware address length */
    netif->hwaddr_len = ETHARP_HWADDR_LEN;

    /* set netif MAC hardware address */
    netif->hwaddr[0] =  MAC_ADDR0;
    netif->hwaddr[1] =  MAC_ADDR1;
    netif->hwaddr[2] =  MAC_ADDR2;
    netif->hwaddr[3] =  MAC_ADDR3;
    netif->hwaddr[4] =  MAC_ADDR4;
    netif->hwaddr[5] =  MAC_ADDR5;

    /* set netif maximum transfer unit */
    netif->mtu = ETH_MAX_PAYLOAD;

    /* device capabilities */
    /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
    netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;

    /* Initialize the RX POOL */
    LWIP_MEMPOOL_INIT(RX_POOL);

    /* Set Tx packet config common parameters */
    memset(&TxConfig, 0 , sizeof(ETH_TxPacketConfig));
    TxConfig.Attributes = ETH_TX_PACKETS_FEATURES_CSUM | ETH_TX_PACKETS_FEATURES_CRCPAD;
    TxConfig.ChecksumCtrl = ETH_CHECKSUM_IPHDR_PAYLOAD_INSERT_PHDR_CALC;
    TxConfig.CRCPadCtrl = ETH_CRC_PAD_INSERT;

    /* create a binary semaphore used for informing ethernetif of frame reception */
    RxPktSemaphore = xSemaphoreCreateBinary();
    TxPktSemaphore = xSemaphoreCreateBinary();

    /* create the task that handles the ETH_MAC */
    osThreadDef(EthIf, ethernetif_input_thread, osPriorityRealtime, 0, INTERFACE_THREAD_STACK_SIZE);
    osThreadCreate (osThread(EthIf), netif);

    low_level_init_phy(netif);
}


/**
  * @brief This function should do the actual transmission of the packet. The packet is
  * contained in the pbuf that is passed to the function. This pbuf
  * might be chained.
  *
  * @param netif the lwip network interface structure for this ethernetif
  * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
  * @return ERR_OK if the packet could be sent
  *         an err_t value if the packet couldn't be sent
  *
  * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
  *       strange results. You might consider waiting for space in the DMA queue
  *       to become available since the stack doesn't retry to send a packet
  *       dropped because of memory failure (except for the TCP timers).
  */
static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
    uint32_t i = 0U;
    struct pbuf *q = NULL;
    err_t errval = ERR_OK;
    ETH_BufferTypeDef Txbuffer[ETH_TX_DESC_CNT];

    memset (Txbuffer, 0, ETH_TX_DESC_CNT * sizeof (ETH_BufferTypeDef));

    for (q = p; q != NULL; q = q->next)
    {
        if (i >= ETH_TX_DESC_CNT)
            return ERR_IF;

        Txbuffer[i].buffer = q->payload;
        Txbuffer[i].len    = q->len;

        if (i > 0)
        {
            Txbuffer[i - 1].next = &Txbuffer[i];
        }

        if (q->next == NULL)
        {
            Txbuffer[i].next = NULL;
        }

        i++;
    }

    TxConfig.Length   = p->tot_len;
    TxConfig.TxBuffer = Txbuffer;
    TxConfig.pData    = p;

    pbuf_ref (p);

    do {
        HAL_ETH_PTP_InsertTxTimestamp(&EthHandle);

        if (HAL_ETH_Transmit_IT (&EthHandle, &TxConfig) == HAL_OK)
        {
            errval = ERR_OK;
        }
        else
        {
            if (HAL_ETH_GetError (&EthHandle) & HAL_ETH_ERROR_BUSY)
            {
                /* Wait for descriptors to become available */
                osSemaphoreWait (TxPktSemaphore, ETHIF_TX_TIMEOUT);
                HAL_ETH_ReleaseTxPacket (&EthHandle);
                errval = ERR_BUF;
            }
            else
            {
                /* Other error */
                pbuf_free (p);
                errval = ERR_IF;
            }
        }
    } while (errval == ERR_BUF);

    return errval;
}

/**
  * @brief Should allocate a pbuf and transfer the bytes of the incoming
  * packet from the interface into the pbuf.
  *
  * @param netif the lwip network interface structure for this ethernetif
  * @return a pbuf filled with the received packet (including MAC header)
  *         NULL on memory error
  */
static struct pbuf * low_level_input(struct netif *netif)
{
    struct pbuf *p = NULL;

    if(RxAllocStatus == RX_ALLOC_OK) {
        HAL_ETH_ReadData(&EthHandle, (void **)&p);
    }

    return p;
}

/**
  * @brief  Checks whether the specified ETHERNET PTP flag is set or not.
  * @param  flag: specifies the flag to check.
  *   This parameter can be one of the following values:
  *     @arg ETH_PTP_FLAG_TSARU : Addend Register Update
  *     @arg ETH_PTP_FLAG_TSITE : Time Stamp Interrupt Trigger Enable
  *     @arg ETH_PTP_FLAG_TSSTU : Time Stamp Update
  *     @arg ETH_PTP_FLAG_TSSTI  : Time Stamp Initialize
  * @retval The new state of ETHERNET PTP Flag (SET or RESET).
  */
static FlagStatus ll_ptp_get_flag(uint32_t flag)
{
  FlagStatus bitstatus = RESET;

  if ((EthHandle.Instance->PTPTSCR & flag) != (uint32_t)RESET)
  {
    bitstatus = SET;
  }
  else
  {
    bitstatus = RESET;
  }
  return bitstatus;
}

/**
  * @brief  Sets the Time Stamp update sign and values.
  * @param  Sign: specifies the PTP Time update value sign.
  *   This parameter can be one of the following values:
  *     @arg ETH_PTP_PositiveTime : positive time value.
  *     @arg ETH_PTP_NegativeTime : negative time value.
  * @param  SecondValue: specifies the PTP Time update second value.
  * @param  SubSecondValue: specifies the PTP Time update sub-second value.
  *   This parameter is a 31 bit value, bit32 correspond to the sign.
  * @retval None
  */
static void ll_ptp_set_time_stamp_update(uint32_t Sign, uint32_t SecondValue, uint32_t SubSecondValue)
{
    /* Set the PTP Time Update High Register */
    EthHandle.Instance->PTPTSHUR = SecondValue;
    /* Set the PTP Time Update Low Register with sign */
    EthHandle.Instance->PTPTSLUR = Sign | SubSecondValue;
}

/**
  * @brief This function is the ethernetif_input task, it is processed when a packet
  * is ready to be read from the interface. It uses the function low_level_input()
  * that should handle the actual reception of bytes from the network
  * interface. Then the type of the received packet is determined and
  * the appropriate input function is called.
  *
  * @param netif the lwip network interface structure for this ethernetif
  */
static void ethernetif_input_thread( void const * argument )
{
  struct pbuf *p;
  struct netif *netif = (struct netif *) argument;

  for( ;; )
  {
    if (osSemaphoreWait(RxPktSemaphore, TIME_WAITING_FOR_INPUT)==osOK)
    {
      do
      {
        p = low_level_input( netif );
        if (p != NULL)
        {
          if (netif->input( p, netif) != ERR_OK )
          {
            pbuf_free(p);
          }
        }
      }while(p!=NULL);
    }
  }
}

/**
  * @brief Should be called at the beginning of the program to set up the
  * network interface. It calls the function low_level_init() to do the
  * actual setup of the hardware.
  *
  * This function should be passed as a parameter to netif_add().
  *
  * @param netif the lwip network interface structure for this ethernetif
  * @return ERR_OK if the loopif is initialized
  *         ERR_MEM if private data couldn't be allocated
  *         any other err_t on error
  */
err_t ethernetif_init(struct netif *netif)
{
    LWIP_ASSERT("netif != NULL", (netif != NULL));

#if LWIP_NETIF_HOSTNAME
    /* Initialize interface hostname */
    netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

    netif->name[0] = IFNAME0;
    netif->name[1] = IFNAME1;

    /* We directly use etharp_output() here to save a function call.
    * You can instead declare your own function an call etharp_output()
    * from it if you have to do some checks before sending (e.g. if link
    * is available...) */
    netif->output = etharp_output;
    netif->linkoutput = low_level_output;

    /* initialize the hardware */
    low_level_init(netif);

    return ERR_OK;
}

void ethernetif_ptp_init(void)
{
    ETH_PTP_ConfigTypeDef ptp_cfg;

    ptp_cfg.TimestampSubsecondInc = ADJ_FREQ_BASE_INCREMENT;
    ptp_cfg.TimestampAddend = ADJ_FREQ_BASE_ADDEND;

    ptp_cfg.Timestamp = ENABLE;
    ptp_cfg.TimestampUpdate = ENABLE;   // Fine update
    ptp_cfg.TimestampAll = ENABLE;
    ptp_cfg.TimestampV2 = 0;            // Time stamp PTP packet snooping
    ptp_cfg.TimestampIPv6 = ENABLE;
    ptp_cfg.TimestampIPv4 = ENABLE;
    ptp_cfg.TimestampEthernet = 0;      // Time stamp snapshot for PTP over ethernet frames enable ????
    ptp_cfg.TimestampEvent = 0;         // Should be enabled for master????
    ptp_cfg.TimestampMaster = ENABLE;
    ptp_cfg.TimestampFilter = 0;        // Filter by MAC address
    ptp_cfg.TimestampClockType = 0;     // Ordinary clock
    ptp_cfg.TimestampRolloverMode = ETH_PTP_ROLLOVER_MODE;

    ptp_cfg.TimestampAddendUpdate = ENABLE;
    ptp_cfg.TimestampUpdateMode = ENABLE;

    HAL_ETH_PTP_SetConfig(&EthHandle, &ptp_cfg);
}

/**
 * @brief
 * 0000: 1 Hz with a pulse width of 125 ms for binary rollover and, of 100 ms for digital rollover
 * 0001: 2 Hz with 50% duty cycle for binary rollover (digital rollover not recommended)
 * 0010: 4 Hz with 50% duty cycle for binary rollover (digital rollover not recommended)
 * 0011: 8 Hz with 50% duty cycle for binary rollover (digital rollover not recommended)
 * 0100: 16 Hz with 50% duty cycle for binary rollover (digital rollover not recommended)
 * ....
 * 1111: 32768 Hz with 50% duty cycle for binary rollover (digital rollover not recommended)
 */
void ethernetif_ptp_set_pps_output(uint8_t freq)
{
    GPIO_InitTypeDef gpio_init;

    ETH->PTPPPSCR = freq & 0x0f;

    gpio_init.Pin = GPIO_PIN_5;  //LL_GPIO_PIN_8
    gpio_init.Mode = GPIO_MODE_AF_PP;
    gpio_init.Pull = GPIO_NOPULL;
    gpio_init.Speed = GPIO_SPEED_FREQ_LOW;
    gpio_init.Alternate = GPIO_AF11_ETH;
    HAL_GPIO_Init(GPIOB, &gpio_init); //GPIOG
}

void ethernetif_ptp_set_time(struct ptptime_t * timestamp)
{
    uint32_t Sign;
    uint32_t SecondValue;
    uint32_t NanoSecondValue;
    uint32_t SubSecondValue;

    /* determine sign and correct Second and Nanosecond values */
    if(timestamp->tv_sec < 0 || (timestamp->tv_sec == 0 && timestamp->tv_nsec < 0)) {
        Sign = ETH_PTP_NegativeTime;
        SecondValue = -timestamp->tv_sec;
        NanoSecondValue = -timestamp->tv_nsec;
    } else {
        Sign = ETH_PTP_PositiveTime;
        SecondValue = timestamp->tv_sec;
        NanoSecondValue = timestamp->tv_nsec;
    }
    /* convert nanosecond to subseconds */
    SubSecondValue = subsecond_to_nanosecond(NanoSecondValue);
    /* Write the offset (positive or negative) in the Time stamp update high and low registers. */
    ll_ptp_set_time_stamp_update(Sign, SecondValue, SubSecondValue);
    /* Set Time stamp control register bit 2 (Time stamp init). */
    EthHandle.Instance->PTPTSCR |= ETH_PTPTSCR_TSSTI;
    /* The Time stamp counter starts operation as soon as it is initialized
    * with the value written in the Time stamp update register. */
    while(ll_ptp_get_flag(ETH_PTP_FLAG_TSSTI) == SET);
}

void ethernetif_ptp_get_time(struct ptptime_t * timestamp)
{
    timestamp->tv_nsec = subsecond_to_nanosecond(EthHandle.Instance->PTPTSLR);
    timestamp->tv_sec = EthHandle.Instance->PTPTSHR;
}

void ethernetif_ptp_update_offset(struct ptptime_t * timeoffset)
{
  uint32_t Sign;
  uint32_t SecondValue;
  uint32_t NanoSecondValue;
  uint32_t SubSecondValue;
  uint32_t addend;

  /* determine sign and correct Second and Nanosecond values */
  if(timeoffset->tv_sec < 0 || (timeoffset->tv_sec == 0 && timeoffset->tv_nsec < 0))
  {
    Sign = ETH_PTP_NegativeTime;
    SecondValue = -timeoffset->tv_sec;
    NanoSecondValue = -timeoffset->tv_nsec;
  }
  else
  {
    Sign = ETH_PTP_PositiveTime;
    SecondValue = timeoffset->tv_sec;
    NanoSecondValue = timeoffset->tv_nsec;
  }

  /* convert nanosecond to subseconds */
  SubSecondValue = subsecond_to_nanosecond(NanoSecondValue);

  /* read old addend register value*/
  addend = EthHandle.Instance->PTPTSAR;

  while(ll_ptp_get_flag(ETH_PTP_FLAG_TSSTU) == SET);
  while(ll_ptp_get_flag(ETH_PTP_FLAG_TSSTI) == SET);

  /* Write the offset (positive or negative) in the Time stamp update high and low registers. */
  ll_ptp_set_time_stamp_update(Sign, SecondValue, SubSecondValue);

  /* Set bit 3 (TSSTU) in the Time stamp control register. */
  EthHandle.Instance->PTPTSCR |= ETH_PTPTSCR_TSSTU;

  /* The value in the Time stamp update registers is added to or subtracted from the system */
  /* time when the TSSTU bit is cleared. */
  while(ll_ptp_get_flag(ETH_PTP_FLAG_TSSTU) == SET);

  /* Write back old addend register value. */
  EthHandle.Instance->PTPTSAR = addend;
  EthHandle.Instance->PTPTSCR |= ETH_PTPTSCR_TSARU;
}

void ethernetif_ptp_adj_freq(int32_t Adj)
{
    uint32_t addend;

  /* calculate the rate by which you want to speed up or slow down the system time
     increments */

  /* precise */
  /*
  int64_t addend;
  addend = Adj;
  addend *= ADJ_FREQ_BASE_ADDEND;
  addend /= 1000000000-Adj;
  addend += ADJ_FREQ_BASE_ADDEND;
  */

  /* 32bit estimation
  ADJ_LIMIT = ((1l<<63)/275/ADJ_FREQ_BASE_ADDEND) = 11258181 = 11 258 ppm*/
  if( Adj > 5120000) Adj = 5120000;
  if( Adj < -5120000) Adj = -5120000;

  addend = ((((275LL * Adj)>>8) * (ADJ_FREQ_BASE_ADDEND >> 24)) >> 6) + ADJ_FREQ_BASE_ADDEND;

  /* Reprogram the Time stamp addend register with new Rate value and set ETH_TPTSCR */
  EthHandle.Instance->PTPTSAR = addend;
  EthHandle.Instance->PTPTSCR |= ETH_PTPTSCR_TSARU;
}

/**
 * @brief get timestamp of last sent package
 * @param time
 */
void ethernetif_ptp_get_tx_timestamp(TimeInternal *time)
{
    osSemaphoreWait(TxPktSemaphore, ETHIF_TX_TIMEOUT);
    HAL_ETH_ReleaseTxPacket(&EthHandle);
    time->nanoseconds = subsecond_to_nanosecond(EthHandle.TxTimestamp.TimeStampLow);
    time->seconds = EthHandle.TxTimestamp.TimeStampHigh;
}

/**
  * @brief  RMII interface watchdog thread
  * @param  argument
  * @retval None
  */
static void RMII_Thread( void const * argument )
{
  (void) argument;

  for(;;)
  {
    /* some unicast good packets are received */
    if(EthHandle.Instance->MMCRGUFCR > 0U)
    {
      /* RMII Init is OK: Delete the Thread */
      osThreadTerminate(NULL);
    }
    else if(EthHandle.Instance->MMCRFCECR > 10U)
    {
      /* ETH received too many packets with CRC errors, resetting RMII */
      SYSCFG->PMC &= ~SYSCFG_PMC_MII_RMII_SEL;
      SYSCFG->PMC |= SYSCFG_PMC_MII_RMII_SEL;

      EthHandle.Instance->MMCCR |= ETH_MMCCR_CR;
    }
    else
    {
      /* Delay 200 ms */
      osDelay(200);
    }
  }
}

/*******************************************************************************
                       Ethernet MSP Routines
*******************************************************************************/
/**
  * @brief  Initializes the ETH MSP.
  * @param  heth: ETH handle
  * @retval None
  */
void HAL_ETH_MspInit(ETH_HandleTypeDef *heth)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable GPIOs clocks */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

/* Ethernet pins configuration ************************************************/
  /*
        RMII_REF_CLK ----------------------> PA1
        RMII_MDIO -------------------------> PA2
        RMII_MDC --------------------------> PC1
        RMII_MII_CRS_DV -------------------> PA7
        RMII_MII_RXD0 ---------------------> PC4
        RMII_MII_RXD1 ---------------------> PC5
        RMII_MII_TX_EN --------------------> PG11
        RMII_MII_TXD0 ---------------------> PG13
        RMII_MII_TXD1 ---------------------> PG14
  */

  /* Configure PA1, PA2 and PA7 */
  GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Alternate = GPIO_AF11_ETH;
  GPIO_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_7;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Configure PC1, PC4 and PC5 */
  GPIO_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* Configure PG11, PG13 and PG14 */
  GPIO_InitStructure.Pin =  GPIO_PIN_11 | GPIO_PIN_13 | GPIO_PIN_14;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);

  /* Enable the Ethernet global Interrupt */
  HAL_NVIC_SetPriority(ETH_IRQn, 0x7, 0);
  HAL_NVIC_EnableIRQ(ETH_IRQn);

  /* Enable ETHERNET clock  */
  __HAL_RCC_ETH_CLK_ENABLE();
}

/**
  * @brief  Ethernet Rx Transfer completed callback
  * @param  heth: ETH handle
  * @retval None
  */
void HAL_ETH_RxCpltCallback(ETH_HandleTypeDef *heth)
{
    osSemaphoreRelease(RxPktSemaphore);
}

void HAL_ETH_TxCpltCallback(ETH_HandleTypeDef *heth)
{
    osSemaphoreRelease(TxPktSemaphore);
}

void HAL_ETH_TxFreeCallback(uint32_t * buff)
{
    pbuf_free((struct pbuf *)buff);
}

void HAL_ETH_RxAllocateCallback(uint8_t **buff)
{
    struct pbuf_custom *p = LWIP_MEMPOOL_ALLOC(RX_POOL);

    if (p) {
        /* Get the buff from the struct pbuf address. */
        *buff = (uint8_t *)p + offsetof(RxBuff_t, buff);
        p->custom_free_function = pbuf_free_custom;
        /* Initialize the struct pbuf.
        * This must be performed whenever a buffer's allocated because it may be
        * changed by lwIP or the app, e.g., pbuf_free decrements ref. */
        pbuf_alloced_custom(PBUF_RAW, 0, PBUF_REF, p, *buff, ETH_RX_BUF_SIZE);
    } else {
        RxAllocStatus = RX_ALLOC_ERROR;
        *buff = NULL;
    }
}

void HAL_ETH_RxLinkCallback(void **pStart, void **pEnd, uint8_t *buff, uint16_t Length)
{
    struct pbuf **ppStart = (struct pbuf **)pStart;
    struct pbuf **ppEnd = (struct pbuf **)pEnd;
    struct pbuf *p = NULL;

    /* Get the struct pbuf from the buff address. */
    p = (struct pbuf *)(buff - offsetof(RxBuff_t, buff));
    p->next = NULL;
    p->tot_len = 0;
    p->len = Length;

    /* Chain the buffer. */
    if (!*ppStart) {
        /* The first buffer of the packet. */
        *ppStart = p;
    } else {
        /* Chain the buffer to the end of the packet. */
        (*ppEnd)->next = p;
    }
        *ppEnd  = p;

    /* Update the total length of all the buffers of the chain. Each pbuf in the chain should have its tot_len
    * set to its own length, plus the length of all the following pbufs in the chain. */
    for (p = *ppStart; p != NULL; p = p->next)
    {
        p->tot_len += Length;
    }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
