/**
 * @file    osc_display.c
 * @brief   Oscilloscope display rendering using ILI9341
 */

#include "osc_display.h"
#include "ili9341.h"
#include "osc_signal.h"
#include "config.h"
#include <stdio.h>
#include <string.h>

static uint16_t prevWaveY[SCREEN_WIDTH];
static uint8_t prevWaveValid = 0;

void Display_Init(void)
{
    ILI9341_Init();
    ILI9341_FillScreen(COLOR_BACKGROUND);
    for (int i = 0; i < SCREEN_WIDTH; i++)
        prevWaveY[i] = WAVE_Y_OFFSET + WAVE_HEIGHT / 2;
    prevWaveValid = 0;
}

void Display_ClearScreen(void)
{
    ILI9341_FillScreen(COLOR_BACKGROUND);
    prevWaveValid = 0;
}

void Display_ClearWaveform(void)
{
    ILI9341_FillRect(WAVE_X_OFFSET, WAVE_Y_OFFSET, WAVE_WIDTH, WAVE_HEIGHT, COLOR_BACKGROUND);
    prevWaveValid = 0;
}

void Display_DrawGrid(void)
{
    uint16_t xStep = WAVE_WIDTH / GRID_DIVISIONS_X;
    uint16_t yStep = WAVE_HEIGHT / GRID_DIVISIONS_Y;

    for (uint8_t gx = 1; gx < GRID_DIVISIONS_X; gx++) {
        uint16_t x = WAVE_X_OFFSET + gx * xStep;
        for (uint16_t y = WAVE_Y_OFFSET; y < WAVE_Y_OFFSET + WAVE_HEIGHT; y += 4)
            ILI9341_DrawPixel(x, y, COLOR_GRID);
    }
    for (uint8_t gy = 1; gy < GRID_DIVISIONS_Y; gy++) {
        uint16_t y = WAVE_Y_OFFSET + gy * yStep;
        for (uint16_t x = WAVE_X_OFFSET; x < WAVE_X_OFFSET + WAVE_WIDTH; x += 4)
            ILI9341_DrawPixel(x, y, COLOR_GRID);
    }

    uint16_t cx = WAVE_X_OFFSET + WAVE_WIDTH / 2;
    uint16_t cy = WAVE_Y_OFFSET + WAVE_HEIGHT / 2;
    for (uint16_t x = WAVE_X_OFFSET; x < WAVE_X_OFFSET + WAVE_WIDTH; x += 2)
        ILI9341_DrawPixel(x, cy, COLOR_DARK_GREEN);
    for (uint16_t y = WAVE_Y_OFFSET; y < WAVE_Y_OFFSET + WAVE_HEIGHT; y += 2)
        ILI9341_DrawPixel(cx, y, COLOR_DARK_GREEN);

    ILI9341_DrawRect(WAVE_X_OFFSET, WAVE_Y_OFFSET, WAVE_WIDTH, WAVE_HEIGHT, COLOR_DARK_GRAY);
}

void Display_DrawWaveform(uint16_t* buffer, uint16_t size, uint8_t channel, float voltDiv)
{
    uint16_t waveColor = (channel == 0) ? COLOR_CH1 : COLOR_CH2;
    uint16_t triggerIdx = Signal_FindTriggerPoint(buffer, size, 0);
    uint16_t displaySamples = size - triggerIdx;
    if (displaySamples > WAVE_WIDTH) displaySamples = WAVE_WIDTH;

    uint16_t prevY = 0;
    for (uint16_t i = 0; i < displaySamples && i < WAVE_WIDTH; i++) {
        uint16_t sampleIdx = triggerIdx + i;
        if (sampleIdx >= size) break;

        uint16_t x = WAVE_X_OFFSET + i;
        uint16_t y = Signal_MapToPixel(buffer[sampleIdx], voltDiv);

        ILI9341_DrawPixel(x, y, waveColor);

        if (i > 0 && (x != (WAVE_X_OFFSET + i - 1) || y != prevY))
            ILI9341_DrawLine(WAVE_X_OFFSET + i - 1, prevY, x, y, waveColor);

        prevY = y;
    }
    prevWaveValid = 1;
}

void Display_DrawMeasurements(float freq, float vpp, uint8_t channel,
                              uint8_t timeDivIdx, uint8_t voltDivIdx, uint8_t isHold)
{
    char buf[32];
    uint16_t chColor = (channel == 0) ? COLOR_CH1 : COLOR_CH2;
    const char* chLabel = (channel == 0) ? "CH1" : "CH2";

    /* Top bar */
    ILI9341_FillRect(0, 0, SCREEN_WIDTH, WAVE_Y_OFFSET, COLOR_BACKGROUND);
    ILI9341_DrawString(2, 4, chLabel, chColor, COLOR_BACKGROUND, 1);

    if (freq > 1000.0f) snprintf(buf, sizeof(buf), "%.1fkHz", freq / 1000.0f);
    else if (freq > 0.0f) snprintf(buf, sizeof(buf), "%.0fHz", freq);
    else snprintf(buf, sizeof(buf), "---Hz");
    ILI9341_DrawString(40, 4, buf, COLOR_TEXT, COLOR_BACKGROUND, 1);

    if (vpp > 0.01f) snprintf(buf, sizeof(buf), "Vpp:%.2fV", vpp);
    else snprintf(buf, sizeof(buf), "Vpp:---");
    ILI9341_DrawString(140, 4, buf, COLOR_TEXT, COLOR_BACKGROUND, 1);

    /* Bottom bar */
    uint16_t bottomY = SCREEN_HEIGHT - 28;
    ILI9341_FillRect(0, bottomY, SCREEN_WIDTH, 28, COLOR_BACKGROUND);

    snprintf(buf, sizeof(buf), "T:%s", TIME_DIV_LABELS[timeDivIdx]);
    ILI9341_DrawString(2, bottomY + 4, buf, COLOR_GREEN, COLOR_BACKGROUND, 1);

    snprintf(buf, sizeof(buf), "V:%s", VOLT_DIV_LABELS[voltDivIdx]);
    ILI9341_DrawString(80, bottomY + 4, buf, COLOR_GREEN, COLOR_BACKGROUND, 1);

    if (isHold) ILI9341_DrawString(170, bottomY + 4, "HOLD", COLOR_RED, COLOR_BACKGROUND, 1);
    else ILI9341_DrawString(170, bottomY + 4, " RUN", COLOR_GREEN, COLOR_BACKGROUND, 1);
}

void Display_ShowWarning(const char* msg)
{
    uint16_t bx = 30, by = 140, bw = 180, bh = 40;
    ILI9341_FillRect(bx, by, bw, bh, COLOR_RED);
    ILI9341_DrawRect(bx, by, bw, bh, COLOR_WHITE);
    uint16_t tx = bx + (bw - strlen(msg)*12) / 2;
    ILI9341_DrawString(tx, by + 13, msg, COLOR_WHITE, COLOR_RED, 2);
}
