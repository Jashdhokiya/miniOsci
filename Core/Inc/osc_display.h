/**
 * @file    osc_display.h
 * @brief   Oscilloscope display rendering
 */

#ifndef OSC_DISPLAY_H
#define OSC_DISPLAY_H

#include <stdint.h>

void Display_Init(void);
void Display_DrawGrid(void);
void Display_DrawAxisLabels(uint8_t timeDivIdx, uint8_t voltDivIdx);
void Display_DrawWaveform(uint16_t* buffer, uint16_t size,
                          uint8_t channel, float voltDiv);
void Display_DrawMeasurements(float freq, float vpp, uint8_t channel,
                              uint8_t timeDivIdx, uint8_t voltDivIdx,
                              uint8_t isHold);
void Display_DrawStats(uint16_t* buffer, uint16_t size, uint8_t channel);
void Display_ShowWarning(const char* msg);
void Display_ClearWaveform(void);
void Display_ClearScreen(void);
uint8_t Display_NeedsGrid(void);

#endif /* OSC_DISPLAY_H */
