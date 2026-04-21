/**
 * @file    ili9341.c
 * @brief   ILI9341 SPI driver — uses CubeMX's hspi1
 *
 * SPI and GPIO are already initialized by CubeMX.
 * This file only does: ILI9341 init sequence + drawing primitives.
 */

#include "ili9341.h"
#include "spi.h"     /* CubeMX: extern hspi1 */
#include "config.h"
#include "fonts.h"

/* ── Pin control macros ──
 * Use the GPIO pin labels from CubeMX main.h or hardcode port/pin.
 * CubeMX configured: PA4=CS, PB6=DC, PB7=RST, PA3=BL
 */
#define CS_LOW()   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET)
#define CS_HIGH()  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET)
#define DC_LOW()   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET)
#define DC_HIGH()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET)
#define RST_LOW()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET)
#define RST_HIGH() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET)

static void WriteCommand(uint8_t cmd)
{
    DC_LOW(); CS_LOW();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    CS_HIGH();
}

static void WriteData(uint8_t data)
{
    DC_HIGH(); CS_LOW();
    HAL_SPI_Transmit(&hspi1, &data, 1, HAL_MAX_DELAY);
    CS_HIGH();
}

/* ════════════════════════════════════════════════════════════════════════ */

void ILI9341_Init(void)
{
    ILI9341_Reset();

    WriteCommand(0x01); HAL_Delay(150);  /* Software reset */
    WriteCommand(0x11); HAL_Delay(150);  /* Exit sleep */

    WriteCommand(0x3A); WriteData(0x55); /* 16-bit RGB565 */

    /* Memory Access Control — landscape mode, no mirror */
    WriteCommand(0x36); WriteData(0x68);

    /* Display Function Control */
    WriteCommand(0xB6); WriteData(0x0A); WriteData(0xA2);

    /* Power Control */
    WriteCommand(0xC0); WriteData(0x35);
    WriteCommand(0xC1); WriteData(0x11);

    /* VCOM */
    WriteCommand(0xC5); WriteData(0x45); WriteData(0x45);
    WriteCommand(0xC7); WriteData(0xA2);

    /* Frame Rate */
    WriteCommand(0xB1); WriteData(0x00); WriteData(0x18);

    /* Gamma */
    WriteCommand(0x26); WriteData(0x01);
    WriteCommand(0xE0);
    uint8_t pg[] = {0x0F,0x2A,0x28,0x08,0x0E,0x08,0x54,0xA9,0x43,0x0A,0x0F,0x00,0x00,0x00,0x00};
    for (int i = 0; i < 15; i++) WriteData(pg[i]);
    WriteCommand(0xE1);
    uint8_t ng[] = {0x00,0x15,0x17,0x07,0x11,0x06,0x2B,0x56,0x3C,0x05,0x10,0x0F,0x3F,0x3F,0x0F};
    for (int i = 0; i < 15; i++) WriteData(ng[i]);

    WriteCommand(0x29); HAL_Delay(50);   /* Display ON */

    /* Backlight ON */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
}

void ILI9341_Reset(void)
{
    RST_HIGH(); HAL_Delay(10);
    RST_LOW();  HAL_Delay(50);
    RST_HIGH(); HAL_Delay(50);
}

void ILI9341_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    WriteCommand(0x2A);
    uint8_t bufX[4] = { x0 >> 8, x0 & 0xFF, x1 >> 8, x1 & 0xFF };
    DC_HIGH(); CS_LOW();
    HAL_SPI_Transmit(&hspi1, bufX, 4, HAL_MAX_DELAY);
    CS_HIGH();

    WriteCommand(0x2B);
    uint8_t bufY[4] = { y0 >> 8, y0 & 0xFF, y1 >> 8, y1 & 0xFF };
    DC_HIGH(); CS_LOW();
    HAL_SPI_Transmit(&hspi1, bufY, 4, HAL_MAX_DELAY);
    CS_HIGH();

    WriteCommand(0x2C);
}

void ILI9341_WriteData16(uint16_t data)
{
    DC_HIGH(); CS_LOW();
    uint8_t buf[2] = { data >> 8, data & 0xFF };
    HAL_SPI_Transmit(&hspi1, buf, 2, HAL_MAX_DELAY);
    CS_HIGH();
}

void ILI9341_WriteData16_Repeat(uint16_t data, uint32_t count)
{
    uint8_t buf[64];
    uint8_t hi = data >> 8;
    uint8_t lo = data & 0xFF;
    for (int i = 0; i < 32; i++) {
        buf[i*2] = hi;
        buf[i*2+1] = lo;
    }

    DC_HIGH(); CS_LOW();
    uint32_t blocks = count / 32;
    for (uint32_t i = 0; i < blocks; i++) {
        HAL_SPI_Transmit(&hspi1, buf, 64, HAL_MAX_DELAY);
    }
    uint32_t rem = count % 32;
    if (rem > 0) {
        HAL_SPI_Transmit(&hspi1, buf, rem * 2, HAL_MAX_DELAY);
    }
    CS_HIGH();
}

void ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) return;
    ILI9341_SetWindow(x, y, x, y);
    ILI9341_WriteData16(color);
}

void ILI9341_FillScreen(uint16_t color)
{
    ILI9341_FillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, color);
}

void ILI9341_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) return;
    if (x + w > SCREEN_WIDTH) w = SCREEN_WIDTH - x;
    if (y + h > SCREEN_HEIGHT) h = SCREEN_HEIGHT - y;
    ILI9341_SetWindow(x, y, x + w - 1, y + h - 1);
    ILI9341_WriteData16_Repeat(color, (uint32_t)w * h);
}

void ILI9341_DrawHLine(uint16_t x, uint16_t y, uint16_t w, uint16_t color)
{ ILI9341_FillRect(x, y, w, 1, color); }

void ILI9341_DrawVLine(uint16_t x, uint16_t y, uint16_t h, uint16_t color)
{ ILI9341_FillRect(x, y, 1, h, color); }

void ILI9341_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    ILI9341_DrawHLine(x, y, w, color);
    ILI9341_DrawHLine(x, y + h - 1, w, color);
    ILI9341_DrawVLine(x, y, h, color);
    ILI9341_DrawVLine(x + w - 1, y, h, color);
}

void ILI9341_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
    int16_t dx = (x1>x0)?(x1-x0):(x0-x1), dy = (y1>y0)?(y1-y0):(y0-y1);
    int16_t sx = (x0<x1)?1:-1, sy = (y0<y1)?1:-1;
    int16_t err = dx - dy;
    while (1) {
        ILI9341_DrawPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int16_t e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 <  dx) { err += dx; y0 += sy; }
    }
}

void ILI9341_DrawChar(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg, uint8_t size)
{
    if (c < 32 || c > 126) c = '?';
    uint8_t idx = c - 32;
    for (uint8_t col = 0; col < FONT_WIDTH; col++) {
        uint8_t line = Font5x7[idx][col];
        for (uint8_t row = 0; row < FONT_HEIGHT; row++) {
            uint16_t dc = (line & (1 << row)) ? color : bg;
            if (size == 1) ILI9341_DrawPixel(x + col, y + row, dc);
            else ILI9341_FillRect(x + col*size, y + row*size, size, size, dc);
        }
    }
    if (size == 1) for (uint8_t r = 0; r < FONT_HEIGHT; r++) ILI9341_DrawPixel(x+FONT_WIDTH, y+r, bg);
    else ILI9341_FillRect(x+FONT_WIDTH*size, y, size, FONT_HEIGHT*size, bg);
}

void ILI9341_DrawString(uint16_t x, uint16_t y, const char* str, uint16_t color, uint16_t bg, uint8_t size)
{
    uint16_t curX = x;
    while (*str) {
        ILI9341_DrawChar(curX, y, *str, color, bg, size);
        curX += (FONT_WIDTH + 1) * size;
        str++;
    }
}
