/**
 * @file    osc_display.c
 * @brief   Oscilloscope display rendering — Dark Pro Theme
 */

#include "osc_display.h"
#include "ili9341.h"
#include "osc_signal.h"
#include "func_gen.h"
#include "config.h"
#include <stdio.h>
#include <string.h>

static uint16_t prevWaveY[SCREEN_WIDTH];
static uint8_t prevWaveValid = 0;

/* ═══════════════════════════════════════════════════════════════════ */
/*                          Init / Clear                              */
/* ═══════════════════════════════════════════════════════════════════ */

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
    ILI9341_FillRect(WAVE_X_OFFSET, WAVE_Y_OFFSET,
                     WAVE_WIDTH, WAVE_HEIGHT, COLOR_BACKGROUND);
    prevWaveValid = 0;
}

/* ═══════════════════════════════════════════════════════════════════ */
/*                          Grid Drawing                              */
/* ═══════════════════════════════════════════════════════════════════ */

void Display_DrawGrid(void)
{
    uint16_t xStep = WAVE_WIDTH / GRID_DIVISIONS_X;
    uint16_t yStep = WAVE_HEIGHT / GRID_DIVISIONS_Y;

    /* Dotted vertical grid lines */
    for (uint8_t gx = 1; gx < GRID_DIVISIONS_X; gx++) {
        uint16_t x = WAVE_X_OFFSET + gx * xStep;
        for (uint16_t y = WAVE_Y_OFFSET; y < WAVE_Y_OFFSET + WAVE_HEIGHT; y += 4)
            ILI9341_DrawPixel(x, y, COLOR_GRID);
    }

    /* Dotted horizontal grid lines */
    for (uint8_t gy = 1; gy < GRID_DIVISIONS_Y; gy++) {
        uint16_t y = WAVE_Y_OFFSET + gy * yStep;
        for (uint16_t x = WAVE_X_OFFSET; x < WAVE_X_OFFSET + WAVE_WIDTH; x += 4)
            ILI9341_DrawPixel(x, y, COLOR_GRID);
    }

    /* Center crosshair — denser dash */
    uint16_t cx = WAVE_X_OFFSET + WAVE_WIDTH / 2;
    uint16_t cy = WAVE_Y_OFFSET + WAVE_HEIGHT / 2;
    for (uint16_t x = WAVE_X_OFFSET; x < WAVE_X_OFFSET + WAVE_WIDTH; x += 2)
        ILI9341_DrawPixel(x, cy, COLOR_DARK_GREEN);
    for (uint16_t y = WAVE_Y_OFFSET; y < WAVE_Y_OFFSET + WAVE_HEIGHT; y += 2)
        ILI9341_DrawPixel(cx, y, COLOR_DARK_GREEN);

    /* Small tick marks on center axes for scale reference */
    for (uint8_t gx = 1; gx < GRID_DIVISIONS_X; gx++) {
        uint16_t x = WAVE_X_OFFSET + gx * xStep;
        ILI9341_DrawVLine(x, cy - 2, 5, COLOR_DARK_GREEN);
    }
    for (uint8_t gy = 1; gy < GRID_DIVISIONS_Y; gy++) {
        uint16_t y = WAVE_Y_OFFSET + gy * yStep;
        ILI9341_DrawHLine(cx - 2, y, 5, COLOR_DARK_GREEN);
    }

    /* Border with accent */
    ILI9341_DrawRect(WAVE_X_OFFSET, WAVE_Y_OFFSET,
                     WAVE_WIDTH, WAVE_HEIGHT, COLOR_PANEL_BORDER);
}

/* ═══════════════════════════════════════════════════════════════════ */
/*                        Axis Labels                                 */
/* ═══════════════════════════════════════════════════════════════════ */

void Display_DrawAxisLabels(uint8_t timeDivIdx, uint8_t voltDivIdx)
{
    char buf[12];
    uint16_t yStep = WAVE_HEIGHT / GRID_DIVISIONS_Y;
    float voltDiv = VOLT_DIV_VALUES[voltDivIdx];

    /* Y-axis voltage labels (right side, inside waveform area) */
    for (int8_t gy = 0; gy <= GRID_DIVISIONS_Y; gy++) {
        uint16_t y = WAVE_Y_OFFSET + gy * yStep;
        float volts = (GRID_DIVISIONS_Y / 2 - gy) * voltDiv;
        if (volts >= 10.0f || volts <= -10.0f)
            snprintf(buf, sizeof(buf), "%+.0f", volts);
        else
            snprintf(buf, sizeof(buf), "%+.1f", volts);
        /* Draw at right edge */
        ILI9341_DrawString(WAVE_X_OFFSET + WAVE_WIDTH - 30,
                          y - 3, buf, COLOR_AXIS_LABEL, COLOR_BACKGROUND, 1);
    }
}

/* ═══════════════════════════════════════════════════════════════════ */
/*                      Waveform Drawing                              */
/* ═══════════════════════════════════════════════════════════════════ */

void Display_DrawWaveform(uint16_t* buffer, uint16_t size,
                          uint8_t channel, float voltDiv)
{
    uint16_t waveColor = (channel == 0) ? COLOR_CH1 : COLOR_CH2;
    uint16_t triggerIdx = Signal_FindTriggerPoint(buffer, size, 0);
    uint16_t displaySamples = size - triggerIdx;
    if (displaySamples > WAVE_WIDTH) displaySamples = WAVE_WIDTH;

    /* Auto-center: find signal midpoint from visible portion */
    uint16_t vMin = 4095, vMax = 0;
    for (uint16_t i = 0; i < displaySamples; i++) {
        uint16_t idx = triggerIdx + i;
        if (idx >= size) break;
        if (buffer[idx] < vMin) vMin = buffer[idx];
        if (buffer[idx] > vMax) vMax = buffer[idx];
    }
    float vMid = (float)(vMin + vMax) / 2.0f;

    /* Scale: ADC counts per volt-division through the divider */
    float adcPerDiv = (voltDiv / VOLTAGE_DIVIDER_RATIO / VREF) * (float)ADC_MAX;
    float totalAdcRange = adcPerDiv * GRID_DIVISIONS_Y;
    float pixelsPerAdc = (float)WAVE_HEIGHT / totalAdcRange;

    uint16_t centerY = WAVE_Y_OFFSET + (WAVE_HEIGHT / 2);
    uint16_t prevY = 0;

    for (uint16_t i = 0; i < displaySamples && i < WAVE_WIDTH; i++) {
        uint16_t sampleIdx = triggerIdx + i;
        if (sampleIdx >= size) break;

        uint16_t x = WAVE_X_OFFSET + i;
        float offset = ((float)buffer[sampleIdx] - vMid) * pixelsPerAdc;
        int16_t pixelY = (int16_t)(centerY - offset);

        if (pixelY < WAVE_Y_OFFSET) pixelY = WAVE_Y_OFFSET;
        if (pixelY > (WAVE_Y_OFFSET + WAVE_HEIGHT - 1))
            pixelY = WAVE_Y_OFFSET + WAVE_HEIGHT - 1;

        uint16_t y = (uint16_t)pixelY;
        ILI9341_DrawPixel(x, y, waveColor);

        if (i > 0)
            ILI9341_DrawLine(WAVE_X_OFFSET + i - 1, prevY, x, y, waveColor);

        prevY = y;
    }
    prevWaveValid = 1;
}

/* ═══════════════════════════════════════════════════════════════════ */
/*                   Top & Bottom Status Bars                         */
/* ═══════════════════════════════════════════════════════════════════ */

void Display_DrawMeasurements(float freq, float vpp, uint8_t channel,
                              uint8_t timeDivIdx, uint8_t voltDivIdx,
                              uint8_t isHold)
{
    char buf[32];
    uint16_t chColor = (channel == 0) ? COLOR_CH1 : COLOR_CH2;
    const char* chLabel = (channel == 0) ? "CH1" : "CH2";

    /* ── Top Bar: dark panel ── */
    ILI9341_FillRect(0, 0, SCREEN_WIDTH, WAVE_Y_OFFSET, COLOR_PANEL_BG);
    ILI9341_DrawHLine(0, WAVE_Y_OFFSET - 1, SCREEN_WIDTH, COLOR_PANEL_BORDER);

    /* Channel indicator with color dot */
    ILI9341_FillRect(4, 4, 4, 12, chColor);  /* Color bar */
    ILI9341_DrawString(12, 4, chLabel, chColor, COLOR_PANEL_BG, 1);

    /* Frequency */
    if (freq > 1000.0f) snprintf(buf, sizeof(buf), "F:%.1fkHz", freq / 1000.0f);
    else if (freq > 0.0f) snprintf(buf, sizeof(buf), "F:%.0fHz", freq);
    else snprintf(buf, sizeof(buf), "F:---");
    ILI9341_DrawString(56, 4, buf, COLOR_TEXT, COLOR_PANEL_BG, 1);

    /* Vpp */
    if (vpp > 0.01f) snprintf(buf, sizeof(buf), "Vpp:%.2fV", vpp);
    else snprintf(buf, sizeof(buf), "Vpp:---");
    ILI9341_DrawString(160, 4, buf, COLOR_TEXT, COLOR_PANEL_BG, 1);

    /* RUN/HOLD indicator */
    if (isHold) {
        ILI9341_FillRect(278, 2, 38, 15, COLOR_RED);
        ILI9341_DrawString(281, 4, "HOLD", COLOR_WHITE, COLOR_RED, 1);
    } else {
        ILI9341_FillRect(278, 2, 38, 15, COLOR_DARK_GREEN);
        ILI9341_DrawString(284, 4, "RUN", COLOR_WHITE, COLOR_DARK_GREEN, 1);
    }

    /* ── Bottom Bar: dark panel ── */
    uint16_t bottomY = SCREEN_HEIGHT - 28;
    ILI9341_FillRect(0, bottomY, SCREEN_WIDTH, 28, COLOR_PANEL_BG);
    ILI9341_DrawHLine(0, bottomY, SCREEN_WIDTH, COLOR_PANEL_BORDER);

    /* Time/div */
    snprintf(buf, sizeof(buf), "T:%s/d", TIME_DIV_LABELS[timeDivIdx]);
    ILI9341_DrawString(4, bottomY + 4, buf, COLOR_GREEN, COLOR_PANEL_BG, 1);

    /* Volt/div */
    snprintf(buf, sizeof(buf), "V:%s/d", VOLT_DIV_LABELS[voltDivIdx]);
    ILI9341_DrawString(100, bottomY + 4, buf, COLOR_GREEN, COLOR_PANEL_BG, 1);

    /* Function generator status */
    if (FuncGen_IsRunning()) {
        uint32_t fgFreq = FuncGen_GetFrequency();
        if (fgFreq >= 1000)
            snprintf(buf, sizeof(buf), "FG:%s %lukHz",
                     FuncGen_GetWaveformName(), (unsigned long)(fgFreq / 1000));
        else
            snprintf(buf, sizeof(buf), "FG:%s %luHz",
                     FuncGen_GetWaveformName(), (unsigned long)fgFreq);
        ILI9341_DrawString(4, bottomY + 16, buf, COLOR_ORANGE, COLOR_PANEL_BG, 1);
    }

    /* Divider ratio indicator */
    snprintf(buf, sizeof(buf), "1:%d", (int)VOLTAGE_DIVIDER_RATIO);
    ILI9341_DrawString(200, bottomY + 4, buf, COLOR_AXIS_LABEL, COLOR_PANEL_BG, 1);
}

/* ═══════════════════════════════════════════════════════════════════ */
/*                      Stats Overlay (PB4)                           */
/* ═══════════════════════════════════════════════════════════════════ */

void Display_DrawStats(uint16_t* buffer, uint16_t size, uint8_t channel)
{
    char buf[24];
    uint16_t chColor = (channel == 0) ? COLOR_CH1 : COLOR_CH2;

    float vMax = Signal_GetVmax(buffer, size);
    float vMin = Signal_GetVmin(buffer, size);
    float vAvg = Signal_GetAvg(buffer, size);
    float vpp  = Signal_GetVpp(buffer, size);

    /* Stats panel: top-right corner overlay */
    uint16_t bx = SCREEN_WIDTH - 120;
    uint16_t by = WAVE_Y_OFFSET + 4;
    uint16_t bw = 116;
    uint16_t bh = 72;

    /* Panel background with border */
    ILI9341_FillRect(bx, by, bw, bh, COLOR_STATS_BG);
    ILI9341_DrawRect(bx, by, bw, bh, COLOR_PANEL_BORDER);

    /* Header */
    const char* chLabel = (channel == 0) ? "CH1 Stats" : "CH2 Stats";
    ILI9341_DrawString(bx + 4, by + 2, chLabel, chColor, COLOR_STATS_BG, 1);
    ILI9341_DrawHLine(bx + 2, by + 12, bw - 4, COLOR_PANEL_BORDER);

    /* Values */
    snprintf(buf, sizeof(buf), "Vmax:%6.2fV", vMax);
    ILI9341_DrawString(bx + 4, by + 16, buf, COLOR_STATS_TEXT, COLOR_STATS_BG, 1);

    snprintf(buf, sizeof(buf), "Vmin:%6.2fV", vMin);
    ILI9341_DrawString(bx + 4, by + 28, buf, COLOR_STATS_TEXT, COLOR_STATS_BG, 1);

    snprintf(buf, sizeof(buf), "Vavg:%6.2fV", vAvg);
    ILI9341_DrawString(bx + 4, by + 40, buf, COLOR_STATS_TEXT, COLOR_STATS_BG, 1);

    snprintf(buf, sizeof(buf), " Vpp:%6.2fV", vpp);
    ILI9341_DrawString(bx + 4, by + 52, buf, COLOR_GREEN, COLOR_STATS_BG, 1);
}

/* ═══════════════════════════════════════════════════════════════════ */
/*                         Warning Box                                */
/* ═══════════════════════════════════════════════════════════════════ */

void Display_ShowWarning(const char* msg)
{
    uint16_t bx = 60, by = 90, bw = 200, bh = 50;
    ILI9341_FillRect(bx, by, bw, bh, COLOR_RED);
    ILI9341_DrawRect(bx, by, bw, bh, COLOR_WHITE);
    ILI9341_DrawRect(bx + 1, by + 1, bw - 2, bh - 2, COLOR_WHITE);
    uint16_t tx = bx + (bw - strlen(msg) * 12) / 2;
    ILI9341_DrawString(tx, by + 17, msg, COLOR_WHITE, COLOR_RED, 2);
}
