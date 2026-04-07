/**
 * @file    ili9341.h
 * @brief   ILI9341 driver — uses CubeMX's hspi1
 */

#ifndef ILI9341_H
#define ILI9341_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

void ILI9341_Init(void);
void ILI9341_Reset(void);
void ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void ILI9341_FillScreen(uint16_t color);
void ILI9341_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ILI9341_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void ILI9341_DrawHLine(uint16_t x, uint16_t y, uint16_t w, uint16_t color);
void ILI9341_DrawVLine(uint16_t x, uint16_t y, uint16_t h, uint16_t color);
void ILI9341_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ILI9341_DrawChar(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg, uint8_t size);
void ILI9341_DrawString(uint16_t x, uint16_t y, const char* str, uint16_t color, uint16_t bg, uint8_t size);
void ILI9341_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void ILI9341_WriteData16(uint16_t data);
void ILI9341_WriteData16_Repeat(uint16_t data, uint32_t count);

#endif /* ILI9341_H */
