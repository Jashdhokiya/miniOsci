/**
 * @file    osc_ui.h
 * @brief   UI state machine and button handling
 */

#ifndef OSC_UI_H
#define OSC_UI_H

#include <stdint.h>

typedef enum { STATE_RUN, STATE_HOLD } OscState;

void UI_HandleButton(uint16_t pin);
OscState UI_GetState(void);
uint8_t UI_GetChannel(void);
uint8_t UI_GetTimeDivIndex(void);
uint8_t UI_GetVoltDivIndex(void);
uint32_t UI_GetTimeDivUs(void);
float UI_GetVoltDiv(void);

#endif /* OSC_UI_H */
