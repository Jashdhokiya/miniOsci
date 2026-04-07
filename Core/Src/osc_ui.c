/**
 * @file    osc_ui.c
 * @brief   UI state machine — adapted for CubeMX
 *
 * GPIO and EXTI are already initialized by CubeMX's MX_GPIO_Init().
 * This file only handles button logic and state management.
 */

#include "osc_ui.h"
#include "osc_adc.h"
#include "config.h"
#include "stm32f4xx_hal.h"

static volatile OscState currentState  = STATE_RUN;
static volatile uint8_t  activeChannel = 0;
static volatile uint8_t  timeDivIndex  = 3;   /* 100us/div */
static volatile uint8_t  voltDivIndex  = 1;   /* 1V/div */
static volatile uint32_t lastButtonTick = 0;

void UI_HandleButton(uint16_t pin)
{
    uint32_t now = HAL_GetTick();
    if ((now - lastButtonTick) < BUTTON_DEBOUNCE_MS) return;
    lastButtonTick = now;

    if (pin == GPIO_PIN_0) {          /* PB0 — Mode */
        if (currentState == STATE_RUN) {
            currentState = STATE_HOLD;
            OSC_ADC_Stop();
        } else {
            currentState = STATE_RUN;
            OSC_ADC_Start();
        }
    }
    else if (pin == GPIO_PIN_1) {     /* PB1 — Time Div */
        timeDivIndex = (timeDivIndex + 1) % NUM_TIME_DIVS;
        OSC_ADC_SetSampleRate(TIME_DIV_ARR[timeDivIndex]);
    }
    else if (pin == GPIO_PIN_2) {     /* PB2 — Volt Div */
        voltDivIndex = (voltDivIndex + 1) % NUM_VOLT_DIVS;
    }
    else if (pin == GPIO_PIN_3) {     /* PB3 — Channel */
        activeChannel = (activeChannel + 1) % NUM_CHANNELS;
    }
}

OscState UI_GetState(void)      { return currentState; }
uint8_t  UI_GetChannel(void)    { return activeChannel; }
uint8_t  UI_GetTimeDivIndex(void) { return timeDivIndex; }
uint8_t  UI_GetVoltDivIndex(void) { return voltDivIndex; }
uint32_t UI_GetTimeDivUs(void)  { return TIME_DIV_US[timeDivIndex]; }
float    UI_GetVoltDiv(void)    { return VOLT_DIV_VALUES[voltDivIndex]; }
