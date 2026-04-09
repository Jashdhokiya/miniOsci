/**
 * @file    func_gen.h
 * @brief   PWM function generator — waveform output on TIM4 CH3 (PB8)
 */

#ifndef FUNC_GEN_H
#define FUNC_GEN_H

#include <stdint.h>

typedef enum {
    WAVE_SINE,
    WAVE_SQUARE,
    WAVE_TRIANGLE,
    WAVE_SAWTOOTH,
    WAVE_TYPE_COUNT
} WaveformType;

void        FuncGen_Init(void);
void        FuncGen_Start(void);
void        FuncGen_Stop(void);
void        FuncGen_SetWaveform(WaveformType type);
void        FuncGen_SetFrequency(uint32_t freqHz);
void        FuncGen_StepISR(void);

const char* FuncGen_GetWaveformName(void);
uint32_t    FuncGen_GetFrequency(void);
uint8_t     FuncGen_IsRunning(void);

#endif /* FUNC_GEN_H */
