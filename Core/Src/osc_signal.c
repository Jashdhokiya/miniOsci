/**
 * @file    osc_signal.c
 * @brief   Signal processing — frequency, Vpp, trigger, pixel mapping
 */

#include "osc_signal.h"
#include "config.h"

float Signal_GetFrequency(uint16_t* buffer, uint16_t size, uint32_t sampleRate)
{
    if (size < 4) return 0.0f;

    uint16_t vMin = 4095, vMax = 0;
    for (uint16_t i = 0; i < size; i++) {
        if (buffer[i] < vMin) vMin = buffer[i];
        if (buffer[i] > vMax) vMax = buffer[i];
    }
    uint16_t midpoint = (vMin + vMax) / 2;
    if ((vMax - vMin) < 50) return 0.0f;

    uint32_t crossingCount = 0;
    uint32_t firstCrossing = 0, lastCrossing = 0;

    for (uint16_t i = 1; i < size; i++) {
        if (buffer[i - 1] < midpoint && buffer[i] >= midpoint) {
            crossingCount++;
            if (crossingCount == 1) firstCrossing = i;
            lastCrossing = i;
        }
    }
    if (crossingCount < 2) return 0.0f;

    float samplesPerPeriod = (float)(lastCrossing - firstCrossing) / (float)(crossingCount - 1);
    return (float)sampleRate / samplesPerPeriod;
}

float Signal_GetVpp(uint16_t* buffer, uint16_t size)
{
    uint16_t vMin = 4095, vMax = 0;
    for (uint16_t i = 0; i < size; i++) {
        if (buffer[i] < vMin) vMin = buffer[i];
        if (buffer[i] > vMax) vMax = buffer[i];
    }
    return ((float)(vMax - vMin) / (float)ADC_MAX) * VREF * VOLTAGE_DIVIDER_RATIO;
}

float Signal_GetVmin(uint16_t* buffer, uint16_t size)
{
    uint16_t vMin = 4095;
    for (uint16_t i = 0; i < size; i++)
        if (buffer[i] < vMin) vMin = buffer[i];
    return ((float)vMin / (float)ADC_MAX) * VREF * VOLTAGE_DIVIDER_RATIO;
}

float Signal_GetVmax(uint16_t* buffer, uint16_t size)
{
    uint16_t vMax = 0;
    for (uint16_t i = 0; i < size; i++)
        if (buffer[i] > vMax) vMax = buffer[i];
    return ((float)vMax / (float)ADC_MAX) * VREF * VOLTAGE_DIVIDER_RATIO;
}

float Signal_GetAvg(uint16_t* buffer, uint16_t size)
{
    uint32_t sum = 0;
    for (uint16_t i = 0; i < size; i++) sum += buffer[i];
    return ((float)sum / (float)size / (float)ADC_MAX) * VREF * VOLTAGE_DIVIDER_RATIO;
}

uint16_t Signal_FindTriggerPoint(uint16_t* buffer, uint16_t size, uint16_t threshold)
{
    if (threshold == 0) {
        uint16_t vMin = 4095, vMax = 0;
        for (uint16_t i = 0; i < size; i++) {
            if (buffer[i] < vMin) vMin = buffer[i];
            if (buffer[i] > vMax) vMax = buffer[i];
        }
        threshold = (vMin + vMax) / 2;
    }
    for (uint16_t i = 1; i < size - 1; i++) {
        if (buffer[i - 1] < threshold && buffer[i] >= threshold)
            return i;
    }
    return 0;
}

uint16_t Signal_MapToPixel(uint16_t adcVal, float voltDiv)
{
    uint16_t centerY = WAVE_Y_OFFSET + (WAVE_HEIGHT / 2);
    float adcPerDiv = (voltDiv / VOLTAGE_DIVIDER_RATIO / VREF) * (float)ADC_MAX;
    float totalAdcRange = adcPerDiv * GRID_DIVISIONS_Y;
    float pixelsPerAdc = (float)WAVE_HEIGHT / totalAdcRange;
    float offset = ((float)adcVal - ((float)ADC_MAX / 2.0f)) * pixelsPerAdc;
    int16_t pixelY = (int16_t)(centerY - offset);

    if (pixelY < WAVE_Y_OFFSET) pixelY = WAVE_Y_OFFSET;
    if (pixelY > (WAVE_Y_OFFSET + WAVE_HEIGHT - 1))
        pixelY = WAVE_Y_OFFSET + WAVE_HEIGHT - 1;
    return (uint16_t)pixelY;
}
