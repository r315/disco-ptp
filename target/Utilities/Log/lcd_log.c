/**
  ******************************************************************************
  * @file    lcd_log.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    18-November-2016
  * @brief   This file provides all the LCD Log firmware functions.
  *
  *          The LCD Log module allows to automatically set a header and footer
  *          on any application using the LCD display and allows to dump user,
  *          debug and error messages by using the following macros: LCD_ErrLog(),
  *          LCD_UsrLog() and LCD_DbgLog().
  *
  *          It supports also the scroll feature by embedding an internal software
  *          cache for display. This feature allows to dump message sequentially
  *          on the display even if the number of displayed lines is bigger than
  *          the total number of line allowed by the display.
  *
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

/* Includes ------------------------------------------------------------------*/
#include  <stdio.h>
#include  "lcd_log.h"

/** @addtogroup Utilities
  * @{
  */

/** @addtogroup STM32_EVAL
* @{
*/

/** @addtogroup Common
  * @{
  */

/** @defgroup LCD_LOG
* @brief LCD Log LCD_Application module
* @{
*/

/** @defgroup LCD_LOG_Private_Types
* @{
*/
typedef struct _lcd_state {
    LCD_LOG_line CacheBuffer [LCD_CACHE_DEPTH];
    uint32_t LineColor;
    uint16_t ScrollBackStep;
    uint16_t CacheBuffer_xptr;
    uint16_t CacheBuffer_yptr_top;
    uint16_t CacheBuffer_yptr_bottom;
    uint16_t CacheBuffer_yptr_top_bak;
    uint16_t CacheBuffer_yptr_bottom_bak;
    FunctionalState CacheBuffer_yptr_invert;
    FunctionalState ScrollActive;
    FunctionalState Lock;
    FunctionalState Scrolled;
    uint8_t esc_seq;
}lcd_state_t;
/**
* @}
*/

/** @defgroup LCD_LOG_Private_Defines
* @{
*/

/**
* @}
*/

/* Define the display window settings */
#define     YWINDOW_MIN         4

/** @defgroup LCD_LOG_Private_Macros
* @{
*/
/**
* @}
*/


/** @defgroup LCD_LOG_Private_Variables
* @{
*/

static lcd_state_t lcd_state;

/**
* @}
*/


/** @defgroup LCD_LOG_Private_FunctionPrototypes
* @{
*/

/**
* @}
*/


/** @defgroup LCD_LOG_Private_Functions
* @{
*/


/**
  * @brief  Initializes the LCD Log module
  * @param  None
  * @retval None
  */

void LCD_LOG_Init ( void)
{
  /* Deinit LCD cache */
  LCD_LOG_DeInit();

  /* Clear the LCD */
  BSP_LCD_Clear(LCD_LOG_BACKGROUND_COLOR);

  //memset(&lcd_state, 0, sizeof(lcd_state));
  lcd_state.LineColor = LCD_LOG_DEFAULT_COLOR;
  lcd_state.CacheBuffer_yptr_invert= ENABLE;
}

/**
  * @brief DeInitializes the LCD Log module.
  * @param  None
  * @retval None
  */
void LCD_LOG_DeInit(void)
{
  lcd_state.LineColor = LCD_LOG_TEXT_COLOR;
  lcd_state.CacheBuffer_xptr = 0;
  lcd_state.CacheBuffer_yptr_top = 0;
  lcd_state.CacheBuffer_yptr_bottom = 0;

  lcd_state.CacheBuffer_yptr_top_bak = 0;
  lcd_state.CacheBuffer_yptr_bottom_bak = 0;

  lcd_state.CacheBuffer_yptr_invert= ENABLE;
  lcd_state.ScrollActive = DISABLE;
  lcd_state.Lock = DISABLE;
  lcd_state.Scrolled = DISABLE;
  lcd_state.ScrollBackStep = 0;
}

/**
  * @brief  Display the application header on the LCD screen
  * @param  header: pointer to the string to be displayed
  * @retval None
  */
void LCD_LOG_SetHeader (uint8_t *header)
{
  /* Set the LCD Font */
  BSP_LCD_SetFont (&LCD_LOG_HEADER_FONT);

  BSP_LCD_SetTextColor(LCD_LOG_SOLID_BACKGROUND_COLOR);
  BSP_LCD_FillRect(0, 0, BSP_LCD_GetXSize(), LCD_LOG_HEADER_FONT.Height * 3);

  /* Set the LCD Text Color */
  BSP_LCD_SetTextColor(LCD_LOG_SOLID_TEXT_COLOR);
  BSP_LCD_SetBackColor(LCD_LOG_SOLID_BACKGROUND_COLOR);

  BSP_LCD_DisplayStringAt(0, LCD_LOG_HEADER_FONT.Height, header, CENTER_MODE);

  BSP_LCD_SetBackColor(LCD_LOG_BACKGROUND_COLOR);
  BSP_LCD_SetTextColor(LCD_LOG_TEXT_COLOR);
  BSP_LCD_SetFont (&LCD_LOG_TEXT_FONT);
}

/**
  * @brief  Display the application footer on the LCD screen
  * @param  footer: pointer to the string to be displayed
  * @retval None
  */
void LCD_LOG_SetFooter(uint8_t *footer)
{
  /* Set the LCD Font */
  BSP_LCD_SetFont (&LCD_LOG_FOOTER_FONT);

  BSP_LCD_SetTextColor(LCD_LOG_SOLID_BACKGROUND_COLOR);
  BSP_LCD_FillRect(0, BSP_LCD_GetYSize() - LCD_LOG_FOOTER_FONT.Height - 4, BSP_LCD_GetXSize(), LCD_LOG_FOOTER_FONT.Height + 4);

  /* Set the LCD Text Color */
  BSP_LCD_SetTextColor(LCD_LOG_SOLID_TEXT_COLOR);
  BSP_LCD_SetBackColor(LCD_LOG_SOLID_BACKGROUND_COLOR);

  BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - LCD_LOG_FOOTER_FONT.Height, footer, CENTER_MODE);

  BSP_LCD_SetBackColor(LCD_LOG_BACKGROUND_COLOR);
  BSP_LCD_SetTextColor(LCD_LOG_TEXT_COLOR);
  BSP_LCD_SetFont (&LCD_LOG_TEXT_FONT);
}

/**
  * @brief  Clear the Text Zone
  * @param  None
  * @retval None
  */
void LCD_LOG_ClearTextZone(void)
{
  uint8_t i=0;

  for (i= 0 ; i < YWINDOW_SIZE; i++)
  {
    BSP_LCD_ClearStringLine(i + YWINDOW_MIN);
  }

  LCD_LOG_DeInit();
}

/**
  * @brief  Redirect the printf to the LCD
  * TODO: Fix color, as each line has a color defined by LCD_LOG_line type,
  * having multiple text colors per line makes things more difficult
  *
  * @param  c: character to be displayed
  * @param  f: output file pointer
  * @retval None
 */
int LCD_LOG_Putchar(int ch)
{

  sFONT *cFont = BSP_LCD_GetFont();
  uint32_t idx;
volatile char dbg = (char)ch;

  if(lcd_state.esc_seq == 0){
    if(ch == '\e'){
        lcd_state.esc_seq++;
        return ch; // enter in escape sequence
    }
  }else if(lcd_state.esc_seq == 1){
    if(ch == '['){
        lcd_state.esc_seq++;
        return ch; // continue escape sequence
    }
    lcd_state.esc_seq = 0;
  }else if(lcd_state.esc_seq == 2){
    switch(ch){
        case '0':
            lcd_state.LineColor = LCD_LOG_DEFAULT_COLOR;
        case '1':
        case '2':
        case '3':
        default: break;
    }
    lcd_state.esc_seq++;
    return ch;
  }else if(lcd_state.esc_seq == 3){
    switch(ch){
        case 'm':
        case 'J':
            lcd_state.esc_seq = 0;
            return ch; // end of seq
        case '1':
            lcd_state.LineColor = LCD_COLOR_RED;
            break;
        case '2':
            lcd_state.LineColor = LCD_COLOR_GREEN;
            break;
        case '3':
            lcd_state.LineColor = LCD_COLOR_YELLOW;
            break;
        case '4':
            lcd_state.LineColor = LCD_COLOR_BLUE;
            break;
        case '5':
            lcd_state.LineColor = LCD_COLOR_MAGENTA;
            break;
        case '6':
            lcd_state.LineColor = LCD_COLOR_CYAN;
            break;
        default:
            break;
    }
    lcd_state.esc_seq++;
    return ch;
  }else if(lcd_state.esc_seq == 4){
    lcd_state.esc_seq = 0;
    return ch; // end seq
  }

  if(lcd_state.Lock == DISABLE)
  {
    if(lcd_state.ScrollActive == ENABLE)
    {
      lcd_state.CacheBuffer_yptr_bottom = lcd_state.CacheBuffer_yptr_bottom_bak;
      lcd_state.CacheBuffer_yptr_top    = lcd_state.CacheBuffer_yptr_top_bak;
      lcd_state.ScrollActive = DISABLE;
      lcd_state.Scrolled = DISABLE;
      lcd_state.ScrollBackStep = 0;

    }

    if(( lcd_state.CacheBuffer_xptr < (BSP_LCD_GetXSize()) /cFont->Width ) &&  ( ch != '\n') && ( ch != '\r'))
    {
      // Generic char within line
      lcd_state.CacheBuffer[lcd_state.CacheBuffer_yptr_bottom].line[lcd_state.CacheBuffer_xptr++] = (uint8_t)ch;
    }
    else
    {
      // new line or character is out of line end
      if(lcd_state.CacheBuffer_yptr_top >= lcd_state.CacheBuffer_yptr_bottom)
      {

        if(lcd_state.CacheBuffer_yptr_invert == DISABLE)
        {
          lcd_state.CacheBuffer_yptr_top++;

          if(lcd_state.CacheBuffer_yptr_top == LCD_CACHE_DEPTH)
          {
            lcd_state.CacheBuffer_yptr_top = 0;
          }
        }
        else
        {
          lcd_state.CacheBuffer_yptr_invert= DISABLE;
        }
      }

      // Cartridge return
      if( ch == '\r'){
        lcd_state.CacheBuffer_xptr = 0;
        return ch;
      }

      // Erases line or remaining characters after
      for(idx = lcd_state.CacheBuffer_xptr ; idx < (BSP_LCD_GetXSize()) /cFont->Width; idx++)
      {
        lcd_state.CacheBuffer[lcd_state.CacheBuffer_yptr_bottom].line[lcd_state.CacheBuffer_xptr++] = ' ';
      }
      // Continue with same color to next line
      lcd_state.CacheBuffer[lcd_state.CacheBuffer_yptr_bottom].color = lcd_state.LineColor;
      // Resets to first character
      lcd_state.CacheBuffer_xptr = 0;
      // Update line
      LCD_LOG_UpdateDisplay ();
      // Advance line index
      lcd_state.CacheBuffer_yptr_bottom ++;

      if (lcd_state.CacheBuffer_yptr_bottom == LCD_CACHE_DEPTH)
      {
        lcd_state.CacheBuffer_yptr_bottom = 0;
        lcd_state.CacheBuffer_yptr_top = 1;
        lcd_state.CacheBuffer_yptr_invert = ENABLE;
      }

      if( ch != '\n')
      {
        lcd_state.CacheBuffer[lcd_state.CacheBuffer_yptr_bottom].line[lcd_state.CacheBuffer_xptr++] = (uint8_t)ch;
      }

    }
  }
  return ch;
}

/**
  * @brief  Update the text area display
  * @param  None
  * @retval None
  */
void LCD_LOG_UpdateDisplay (void)
{
  uint8_t cnt = 0 ;
  uint16_t length = 0 ;
  uint16_t ptr = 0, index = 0;

  if((lcd_state.CacheBuffer_yptr_bottom  < (YWINDOW_SIZE -1)) &&
     (lcd_state.CacheBuffer_yptr_bottom  >= lcd_state.CacheBuffer_yptr_top))
  {
    BSP_LCD_SetTextColor(lcd_state.CacheBuffer[cnt + lcd_state.CacheBuffer_yptr_bottom].color);
    BSP_LCD_DisplayStringAtLine ((YWINDOW_MIN + lcd_state.CacheBuffer_yptr_bottom),
                           (uint8_t *)(lcd_state.CacheBuffer[cnt + lcd_state.CacheBuffer_yptr_bottom].line));
  }
  else
  {

    if(lcd_state.CacheBuffer_yptr_bottom < lcd_state.CacheBuffer_yptr_top)
    {
      /* Virtual length for rolling */
      length = LCD_CACHE_DEPTH + lcd_state.CacheBuffer_yptr_bottom ;
    }
    else
    {
      length = lcd_state.CacheBuffer_yptr_bottom;
    }

    ptr = length - YWINDOW_SIZE + 1;

    for  (cnt = 0 ; cnt < YWINDOW_SIZE ; cnt ++)
    {

      index = (cnt + ptr )% LCD_CACHE_DEPTH ;

      BSP_LCD_SetTextColor(lcd_state.CacheBuffer[index].color);
      BSP_LCD_DisplayStringAtLine ((cnt + YWINDOW_MIN),
                             (uint8_t *)(lcd_state.CacheBuffer[index].line));

    }
  }

}

#if( LCD_SCROLL_ENABLED == 1)
/**
  * @brief  Display previous text frame
  * @param  None
  * @retval Status
  */
ErrorStatus LCD_LOG_ScrollBack(void)
{

  if(lcd_state.ScrollActive == DISABLE)
  {

    lcd_state.CacheBuffer_yptr_bottom_bak = lcd_state.CacheBuffer_yptr_bottom;
    lcd_state.CacheBuffer_yptr_top_bak    = lcd_state.CacheBuffer_yptr_top;


    if(lcd_state.CacheBuffer_yptr_bottom > lcd_state.CacheBuffer_yptr_top)
    {

      if ((lcd_state.CacheBuffer_yptr_bottom - lcd_state.CacheBuffer_yptr_top) <=  YWINDOW_SIZE)
      {
        lcd_state.Lock = DISABLE;
        return ERROR;
      }
    }
    lcd_state.ScrollActive = ENABLE;

    if((lcd_state.CacheBuffer_yptr_bottom  > lcd_state.CacheBuffer_yptr_top)&&
       (lcd_state.Scrolled == DISABLE ))
    {
      lcd_state.CacheBuffer_yptr_bottom--;
      lcd_state.Scrolled = ENABLE;
    }

  }

  if(lcd_state.ScrollActive == ENABLE)
  {
    lcd_state.Lock = ENABLE;

    if(lcd_state.CacheBuffer_yptr_bottom > lcd_state.CacheBuffer_yptr_top)
    {

      if((lcd_state.CacheBuffer_yptr_bottom  - lcd_state.CacheBuffer_yptr_top) <  YWINDOW_SIZE )
      {
        lcd_state.Lock = DISABLE;
        return ERROR;
      }

      lcd_state.CacheBuffer_yptr_bottom --;
    }
    else if(lcd_state.CacheBuffer_yptr_bottom <= lcd_state.CacheBuffer_yptr_top)
    {

      if((LCD_CACHE_DEPTH  - lcd_state.CacheBuffer_yptr_top + lcd_state.CacheBuffer_yptr_bottom) < YWINDOW_SIZE)
      {
        lcd_state.Lock = DISABLE;
        return ERROR;
      }
      lcd_state.CacheBuffer_yptr_bottom --;

      if(lcd_state.CacheBuffer_yptr_bottom == 0xFFFF)
      {
        lcd_state.CacheBuffer_yptr_bottom = LCD_CACHE_DEPTH - 2;
      }
    }
    lcd_state.ScrollBackStep++;
    LCD_LOG_UpdateDisplay();
    lcd_state.Lock = DISABLE;
  }
  return SUCCESS;
}

/**
  * @brief  Display next text frame
  * @param  None
  * @retval Status
  */
ErrorStatus LCD_LOG_ScrollForward(void)
{

  if(lcd_state.ScrollBackStep != 0)
  {
    if(lcd_state.ScrollActive == DISABLE)
    {

      lcd_state.CacheBuffer_yptr_bottom_bak = lcd_state.CacheBuffer_yptr_bottom;
      lcd_state.CacheBuffer_yptr_top_bak    = lcd_state.CacheBuffer_yptr_top;

      if(lcd_state.CacheBuffer_yptr_bottom > lcd_state.CacheBuffer_yptr_top)
      {

        if ((lcd_state.CacheBuffer_yptr_bottom - lcd_state.CacheBuffer_yptr_top) <=  YWINDOW_SIZE)
        {
          lcd_state.Lock = DISABLE;
          return ERROR;
        }
      }
      lcd_state.ScrollActive = ENABLE;

      if((lcd_state.CacheBuffer_yptr_bottom  > lcd_state.CacheBuffer_yptr_top)&&
         (lcd_state.Scrolled == DISABLE ))
      {
        lcd_state.CacheBuffer_yptr_bottom--;
        lcd_state.Scrolled = ENABLE;
      }

    }

    if(lcd_state.ScrollActive == ENABLE)
    {
      lcd_state.Lock = ENABLE;
      lcd_state.ScrollBackStep--;

      if(++lcd_state.CacheBuffer_yptr_bottom == LCD_CACHE_DEPTH)
      {
        lcd_state.CacheBuffer_yptr_bottom = 0;
      }

      LCD_LOG_UpdateDisplay();
      lcd_state.Lock = DISABLE;

    }
    return SUCCESS;
  }
  else // LCD_ScrollBackStep == 0
  {
    lcd_state.Lock = DISABLE;
    return ERROR;
  }
}
#endif /* LCD_SCROLL_ENABLED */

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
