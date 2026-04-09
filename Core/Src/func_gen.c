/**
 * @file    func_gen.c
 * @brief   PWM function generator implementation
 *
 * TIM4 CH3 (PB8) outputs a high-frequency PWM (~390 kHz).
 * TIM5 fires interrupts to step through a 64-sample lookup table,
 * updating TIM4's CCR3 (duty cycle) each tick.
 * External RC filter (1k + 10nF, fc~16 kHz) smooths the output.
 */

#include "func_gen.h"
#include "tim.h"
#include "config.h"

/* ── 64-sample Lookup Tables (0–255) ── */

static const uint8_t sineLUT[FUNCGEN_LUT_SIZE] = {
    128,140,152,165,176,187,198,208,
    218,226,234,240,245,250,253,254,
    255,254,253,250,245,240,234,226,
    218,208,198,187,176,165,152,140,
    128,115,103, 90, 79, 68, 57, 47,
     37, 29, 21, 15, 10,  5,  2,  1,
      0,  1,  2,  5, 10, 15, 21, 29,
     37, 47, 57, 68, 79, 90,103,115
};

static const uint8_t squareLUT[FUNCGEN_LUT_SIZE] = {
    255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,
      0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0
};

static const uint8_t triangleLUT[FUNCGEN_LUT_SIZE] = {
      0,  8, 16, 25, 33, 41, 49, 57,
     66, 74, 82, 90, 99,107,115,123,
    132,140,148,156,164,173,181,189,
    197,206,214,222,230,239,247,255,
    255,247,239,230,222,214,206,197,
    189,181,173,164,156,148,140,132,
    123,115,107, 99, 90, 82, 74, 66,
     57, 49, 41, 33, 25, 16,  8,  0
};

static const uint8_t sawtoothLUT[FUNCGEN_LUT_SIZE] = {
      0,  4,  8, 12, 16, 20, 24, 29,
     33, 37, 41, 45, 49, 53, 57, 61,
     65, 69, 73, 77, 81, 85, 89, 93,
     97,101,105,109,113,117,121,125,
    130,134,138,142,146,150,154,158,
    162,166,170,174,178,182,186,190,
    194,198,202,206,210,214,218,222,
    226,231,235,239,243,247,251,255
};

/* ── State ── */
static const uint8_t *activeLUT = sineLUT;
static volatile uint8_t lutIndex = 0;
static WaveformType currentWaveform = WAVE_SINE;
static uint32_t currentFreq = FUNCGEN_DEFAULT_FREQ;
static uint8_t running = 0;

/* ── API ── */

void FuncGen_Init(void)
{
    currentWaveform = WAVE_SINE;
    activeLUT = sineLUT;
    lutIndex = 0;
    currentFreq = FUNCGEN_DEFAULT_FREQ;
    running = 0;
}

void FuncGen_Start(void)
{
    lutIndex = 0;
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
    __HAL_TIM_SET_COUNTER(&htim5, 0);
    __HAL_TIM_ENABLE_IT(&htim5, TIM_IT_UPDATE);
    HAL_TIM_Base_Start(&htim5);
    running = 1;
}

void FuncGen_Stop(void)
{
    HAL_TIM_Base_Stop(&htim5);
    __HAL_TIM_DISABLE_IT(&htim5, TIM_IT_UPDATE);
    TIM4->CCR3 = 0;
    HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_3);
    running = 0;
}

void FuncGen_SetWaveform(WaveformType type)
{
    currentWaveform = type;
    switch (type) {
        case WAVE_SINE:     activeLUT = sineLUT;     break;
        case WAVE_SQUARE:   activeLUT = squareLUT;   break;
        case WAVE_TRIANGLE: activeLUT = triangleLUT; break;
        case WAVE_SAWTOOTH: activeLUT = sawtoothLUT; break;
        default:            activeLUT = sineLUT;     break;
    }
}

void FuncGen_SetFrequency(uint32_t freqHz)
{
    if (freqHz == 0) freqHz = 1;
    currentFreq = freqHz;
    uint32_t arr = (FUNCGEN_TIM_CLOCK / (freqHz * FUNCGEN_LUT_SIZE)) - 1;
    if (arr < 1) arr = 1;
    __HAL_TIM_SET_AUTORELOAD(&htim5, arr);
    __HAL_TIM_SET_COUNTER(&htim5, 0);
}

/* Called from TIM5_IRQHandler — keep minimal for performance */
void FuncGen_StepISR(void)
{
    TIM4->CCR3 = activeLUT[lutIndex];
    lutIndex = (lutIndex + 1) & (FUNCGEN_LUT_SIZE - 1);
}

const char* FuncGen_GetWaveformName(void)
{
    switch (currentWaveform) {
        case WAVE_SINE:     return "SIN";
        case WAVE_SQUARE:   return "SQR";
        case WAVE_TRIANGLE: return "TRI";
        case WAVE_SAWTOOTH: return "SAW";
        default:            return "???";
    }
}

uint32_t FuncGen_GetFrequency(void) { return currentFreq; }
uint8_t  FuncGen_IsRunning(void)    { return running; }
