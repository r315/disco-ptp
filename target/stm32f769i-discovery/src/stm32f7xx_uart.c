/*
 * stm32f769_discovery_uart.c
 *
 *  Created on: 7 Apr 2021
 *      Author: hmr
 */
#include <stdint.h>
#include "cmsis_os.h"
#include "stm32f7xx.h"
#include "stm32f7xx_hal.h"

#ifdef ENABLE_UART_CMSIS_OS
#include "cmsis_os.h"
#endif

#define UART_UART1_TIMEOUT  1000
#define UART_IRQ            USART1_IRQn
#define UART_QUEUE_SIZE     256

static UART_HandleTypeDef huart1;

#ifdef ENABLE_UART_CMSIS_OS
#if (osCMSIS <= 0x20000U)

static osMessageQId uartRxQueueId, uartTxQueueId;

static void usartHandler(UART_HandleTypeDef *huart)
{
    uint32_t isrflags = huart->Instance->ISR;
    uint32_t cr1its = huart->Instance->CR1;

    /* If no error occurs */
    uint32_t errorflags = (isrflags & (uint32_t)(USART_ISR_PE |
                                                 USART_ISR_FE | USART_ISR_ORE | USART_ISR_NE | USART_ISR_RTOF));
    if (errorflags == 0U) {
        if (((isrflags & USART_ISR_RXNE) != 0U) && ((cr1its & USART_CR1_RXNEIE) != 0U)) {
            osStatus status = osMessagePut(uartRxQueueId, (uint32_t)huart->Instance->RDR, 0);
            if (status != osOK) {
                return;
            }
        }

        if (((isrflags & USART_ISR_TXE) != 0U) && ((cr1its & USART_CR1_TXEIE) != 0U)) {
            osEvent event = osMessageGet(uartTxQueueId, 0);

            if (event.status != osEventMessage) {
                /* No data, disable TXE interrupt */
                CLEAR_BIT(huart->Instance->CR1, USART_CR1_TXEIE);
            }

            huart->Instance->TDR = (uint32_t)event.value.v;
        }
    } else {
        // Clr flags
        huart->Instance->ICR = errorflags;
    }
}

uint32_t UART_Available(void)
{
    return osMessageWaiting(uartRxQueueId);
}

uint32_t UART_Write(const uint8_t *data, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++, data++)
    {
        osStatus status = osMessagePut(uartTxQueueId, (uint32_t)*data, UART_UART1_TIMEOUT);
        if (status != osOK) {
            return 0;
        }
    }

    SET_BIT(huart1.Instance->CR1, USART_CR1_TXEIE);

    return len;
}

uint32_t UART_Read(uint8_t *data, uint32_t len)
{
    uint32_t count = 0;

    while (count < len){
        while (UART_Available() == 0);
        osEvent event = osMessageGet(uartRxQueueId, portMAX_DELAY);
        if (event.status == osEventMessage) {
            *data++ = (uint8_t)event.value.v;
            count++;
        }
    }

    return count;
}

void USART1_IRQHandler(void)
{
    usartHandler(&huart1);
}

#else
static osMessageQueueId_t uartTxQueueId, uartRxQueueId;
//TODO: cmsis_os2
#endif /* ENABLE_UART_CMSIS_OS */


#elif defined(ENABLE_UART_FIFO)

static fifo_t txfifo;
static fifo_t rxfifo;

UART_HANDLER_PROTOTYPE
{
    uint32_t isrflags = huart1.Instance->ISR;
    uint32_t cr1its = huart1.Instance->CR1;

    /* If no error occurs */
    uint32_t errorflags = isrflags & (uint32_t)(USART_ISR_PE | USART_ISR_FE | USART_ISR_NE | USART_ISR_ORE | USART_ISR_RTOF);

    if (errorflags == 0U)
    {
        if (((isrflags & USART_ISR_RXNE) != 0U) && ((cr1its & USART_CR1_RXNEIE) != 0U))
        {
            fifo_put(&rxfifo, (uint8_t)READ_REG(huart1.Instance->RDR));
        }

        if (((isrflags & USART_ISR_TXE) != 0U) && ((cr1its & USART_CR1_TXEIE) != 0U))
        {
            if (fifo_get(&txfifo, (uint8_t *)&huart1.Instance->TDR) == 0U)
            {
                /* No data transmitted, disable TXE interrupt */
                CLEAR_BIT(huart1.Instance->CR1, USART_CR1_TXEIE);
            }
        }
    }
    else
    {
        Error_Handler();
    }
}

uint32_t UART_Write(const uint8_t *data, uint32_t len)
{
    uint32_t sent = 0;
    while (len--)
    {
        if (fifo_free(&txfifo) == 0)
        {
            SET_BIT(huart1.Instance->CR1, USART_CR1_TXEIE);
            while (fifo_free(&txfifo) == 0)
                ;
        }
        fifo_put(&txfifo, *data++);
        sent++;
    }

    SET_BIT(huart1.Instance->CR1, USART_CR1_TXEIE);

    return sent;
}

uint8_t UART_Available(void)
{
    return fifo_avail(&rxfifo);
}

uint32_t UART_Read(uint8_t *data, uint32_t len)
{
    uint32_t count = len;

    while (count--)
    {
        while (UART_Available() == 0)
            ;
        fifo_get(&rxfifo, (uint8_t *)data++);
    }

    return len;
}

#else  /* BOARD_UART_FIFO_ENABLE */
uint32_t UART_Write(const uint8_t *data, uint32_t len)
{
    return HAL_UART_Transmit(&huart1, data, len, UART_UART1_TIMEOUT);
}

uint32_t UART_Read(uint8_t *data, uint32_t len)
{
    return HAL_UART_Receive(&huart1, data, len, UART_UART1_TIMEOUT);
}
#endif /* CMSIS_OS */

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (huart->Instance == USART1)
    {
        __HAL_RCC_USART1_CLK_ENABLE();

        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**USART1 GPIO Configuration
        PA10     ------> USART1_RX
        PA9     ------> USART1_TX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}

uint8_t UART_Init(void)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        return 0;
    }

#if defined(ENABLE_UART_CMSIS_OS)
    if (osKernelRunning())
    {
        osMessageQDef_t queue_def;

        queue_def.queue_sz = UART_QUEUE_SIZE;
        queue_def.item_sz = 1;

        uartTxQueueId = osMessageCreate(&queue_def, osThreadGetId());
        uartRxQueueId = osMessageCreate(&queue_def, osThreadGetId());

        if (uartRxQueueId == NULL || uartTxQueueId == NULL) {
            return 0;
        }

        HAL_NVIC_SetPriority(UART_IRQ, 5, 0);
        HAL_NVIC_EnableIRQ(UART_IRQ);
        SET_BIT(huart1.Instance->CR1, USART_CR1_RXNEIE);
    }
#elif defined(BOARD_UART_FIFO_ENABLE)
    rxfifo.size = sizeof(rxfifo.buf);
    txfifo.size = sizeof(txfifo.buf);
    fifo_init(&txfifo);
    fifo_init(&rxfifo);
    fifo_flush(&txfifo);
    fifo_flush(&rxfifo);

    HAL_NVIC_SetPriority(UART_IRQ, 5, 0);
    HAL_NVIC_EnableIRQ(UART_IRQ);

    SET_BIT(huart1.Instance->CR1, USART_CR1_RXNEIE);

#endif
    return 1;
}
