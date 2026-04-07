/**
 * @file    osc_signal.h
 * @brief   Signal processing — frequency, Vpp, trigger, mapping
 */

#ifndef OSC_SIGNAL_H
#define OSC_SIGNAL_H

#include <stdint.h>

float Signal_GetFrequency(uint16_t* buffer, uint16_t size, uint32_t sampleRate);
float Signal_GetVpp(uint16_t* buffer, uint16_t size);
float Signal_GetVmin(uint16_t* buffer, uint16_t size);
float Signal_GetVmax(uint16_t* buffer, uint16_t size);
float Signal_GetAvg(uint16_t* buffer, uint16_t size);
uint16_t Signal_FindTriggerPoint(uint16_t* buffer, uint16_t size, uint16_t threshold);
uint16_t Signal_MapToPixel(uint16_t adcVal, float voltDiv);

#endif /* OSC_SIGNAL_H */
