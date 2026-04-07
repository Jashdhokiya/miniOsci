/**
 * @file    osc_adc.h
 * @brief   Oscilloscope ADC application layer (uses CubeMX hadc1/htim2)
 */

#ifndef OSC_ADC_H
#define OSC_ADC_H

#include "stm32f4xx_hal.h"
#include "config.h"

/** Start ADC+DMA+Timer sampling */
void OSC_ADC_Start(void);

/** Stop ADC+DMA+Timer */
void OSC_ADC_Stop(void);

/** Get de-interleaved buffer. channel: 0=CH1, 1=CH2 */
uint16_t* OSC_ADC_GetBuffer(uint8_t channel);

/** Update sample rate by changing TIM2 ARR */
void OSC_ADC_SetSampleRate(uint32_t arr);

/** Flag set by DMA complete callback */
extern volatile uint8_t bufferReady;

#endif /* OSC_ADC_H */
