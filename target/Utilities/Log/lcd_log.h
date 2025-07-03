/**
  ******************************************************************************
  * @file    lcd_log.h
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    18-November-2016
  * @brief   header for the lcd_log.c file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef  __LCD_LOG_H__
#define  __LCD_LOG_H__

/* Includes ------------------------------------------------------------------*/

#include "lcd_log_conf.h"

/** @addtogroup Utilities
  * @{
  */

/** @addtogroup STM32_EVAL
  * @{
  */

/** @addtogroup Common
  * @{
  */

/** @addtogroup LCD_LOG
  * @{
  */

/** @defgroup LCD_LOG
  * @brief
  * @{
  */


/** @defgroup LCD_LOG_Exported_Defines
  * @{
  */

#if (LCD_SCROLL_ENABLED == 1)
 #define     LCD_CACHE_DEPTH     (YWINDOW_SIZE + CACHE_SIZE)
#else
 #define     LCD_CACHE_DEPTH     YWINDOW_SIZE
#endif
/**
  * @}
  */

/** @defgroup LCD_LOG_Exported_Types
  * @{
  */
typedef struct _LCD_LOG_line
{
  uint8_t  line[128];
  uint32_t color;
}LCD_LOG_line;

/**
  * @}
  */

/** @defgroup LCD_LOG_Exported_Macros
  * @{
  */

#define LOG_PRINT(...) \
    do { printf(__VA_ARGS__); putchar('\n'); } while(0)

#if 0
#define  LCD_ErrLog(...)    do { \
                                 lcd_state.LineColor = LCD_COLOR_RED;\
                                 printf("ERROR: ") ;\
                                 printf(__VA_ARGS__);\
                                 lcd_state.LineColor = LCD_LOG_DEFAULT_COLOR;\
                               }while (0)

                               #define  LCD_UsrLog(...)    do { \
	                             LCD_LineColor = LCD_LOG_TEXT_COLOR;\
                                 printf(__VA_ARGS__);\
                               } while (0)


#define  LCD_DbgLog(...)    do { \
                                 lcd_state.LineColor = LCD_COLOR_CYAN;\
                                 printf(__VA_ARGS__);\
                                 lcd_state.LineColor = LCD_LOG_DEFAULT_COLOR;\
                               }while (0)
                    #endif



//https://gist.github.com/viniciusdaniel/53a98cbb1d8cac1bb473da23f5708836
#define VT100_CLEAR     "\e[2J"
#define VT100_NORMAL    "\e[0m"
#define VT100_BOLD      "\e[1m"
#define VT100_RED       "\e[31m"
#define VT100_GREEN     "\e[32m"
#define VT100_YELLOW    "\e[33m"
#define VT100_BLUE      "\e[34m"
#define VT100_MAGENTA   "\e[35m"
#define VT100_CYAN      "\e[36m"

#define LOG_INF(...) LOG_PRINT(VT100_GREEN "INFO: " VT100_NORMAL __VA_ARGS__)
#define LOG_WRN(...) LOG_PRINT(VT100_YELLOW "WARN: " VT100_NORMAL __VA_ARGS__)
#define LOG_ERR(...) LOG_PRINT(VT100_RED "ERROR: " VT100_NORMAL __VA_ARGS__)
#define LOG_DBG(...) LOG_PRINT(VT100_CYAN "DBG: " VT100_NORMAL __VA_ARGS__)

/**
  * @}
  */

/** @defgroup LCD_LOG_Exported_Variables
  * @{
  */
extern uint32_t LCD_LineColor;

/**
  * @}
  */

/** @defgroup LCD_LOG_Exported_FunctionsPrototype
  * @{
  */
void LCD_LOG_Init(void);
void LCD_LOG_DeInit(void);
void LCD_LOG_SetHeader(uint8_t *Title);
void LCD_LOG_SetFooter(uint8_t *Status);
void LCD_LOG_ClearTextZone(void);
void LCD_LOG_UpdateDisplay (void);
int LCD_LOG_Putchar(int ch);

#if (LCD_SCROLL_ENABLED == 1)
 ErrorStatus LCD_LOG_ScrollBack(void);
 ErrorStatus LCD_LOG_ScrollForward(void);
#endif
/**
  * @}
  */


#endif /* __LCD_LOG_H__ */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
