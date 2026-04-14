/**
 * @file    osc_adc.c
 * @brief   ADC application layer — uses CubeMX-generated hadc1, htim2
 *
 * CubeMX already initializes ADC1, DMA, and TIM2.
 * This file only handles: start/stop, buffer management, de-interleave.
 */

#include "osc_adc.h"
#include "adc.h"    /* CubeMX: extern hadc1 */
#include "tim.h"    /* CubeMX: extern htim2 */
#include "config.h"

/* ── Buffers ── */
static uint16_t adcRawBuffer[ADC_BUFFER_SIZE];
static uint16_t ch1Buffer[SAMPLE_BUFFER_SIZE];
static uint16_t ch2Buffer[SAMPLE_BUFFER_SIZE];

/* ── Flag ── */
volatile uint8_t bufferReady = 0;

/* ── Private ── */
static void DeInterleave(void);

void OSC_ADC_Start(void)
{
    /* Start DMA in circular mode — CubeMX already configured everything */
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adcRawBuffer, ADC_BUFFER_SIZE);

    /* Start timer — this triggers ADC via TRGO */
    HAL_TIM_Base_Start(&htim2);
}

void OSC_ADC_Stop(void)
{
    HAL_TIM_Base_Stop(&htim2);
    HAL_ADC_Stop_DMA(&hadc1);
}

uint16_t* OSC_ADC_GetBuffer(uint8_t channel)
{
    DeInterleave();
    return (channel == 0) ? ch1Buffer : ch2Buffer;
}

void OSC_ADC_SetSampleRate(uint32_t arr)
{
    /* Fully stop the ADC+DMA+Timer chain first */
    HAL_TIM_Base_Stop(&htim2);
    HAL_ADC_Stop_DMA(&hadc1);

    /* Update timer period */
    __HAL_TIM_SET_AUTORELOAD(&htim2, arr);
    __HAL_TIM_SET_COUNTER(&htim2, 0);

    /* Clear any pending flag and restart cleanly */
    bufferReady = 0;
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adcRawBuffer, ADC_BUFFER_SIZE);
    HAL_TIM_Base_Start(&htim2);
}

static void DeInterleave(void)
{
    for (uint16_t i = 0; i < SAMPLE_BUFFER_SIZE; i++) {
        ch1Buffer[i] = adcRawBuffer[i * 2];
        ch2Buffer[i] = adcRawBuffer[i * 2 + 1];
    }
}

/* ── HAL Callback — called by DMA ISR via HAL ── */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if (hadc->Instance == ADC1) {
        bufferReady = 1;
    }
}

/* ── HAL Callback — ADC Analog Watchdog (overvoltage) ── */
void HAL_ADC_LevelOutOfWindowCallback(ADC_HandleTypeDef* hadc)
{
    if (hadc->Instance == ADC1) {
        OSC_ADC_Stop();
        /* Display warning will be called from main loop */
    }
}
