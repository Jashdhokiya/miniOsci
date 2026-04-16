/**
 * @file    config.h
 * @brief   Oscilloscope constants — adapted for CubeMX project
 */

#ifndef CONFIG_H
#define CONFIG_H

/* ── Sampling ── */
#define SAMPLE_BUFFER_SIZE    512
#define ADC_BUFFER_SIZE       (SAMPLE_BUFFER_SIZE * 2)  /* interleaved CH1+CH2 */
#define ADC_MAX               4095
#define VREF                  3.3f
#define NUM_CHANNELS          2

/* ── Display (landscape: 320×240) ── */
#define SCREEN_WIDTH          320
#define SCREEN_HEIGHT         240
#define WAVE_X_OFFSET         0
#define WAVE_Y_OFFSET         20
#define WAVE_WIDTH            SCREEN_WIDTH
#define WAVE_HEIGHT           (SCREEN_HEIGHT - 50)
#define GRID_DIVISIONS_X      10
#define GRID_DIVISIONS_Y      6

/* ── Colors — Dark Pro Theme (RGB565) ── */
#define COLOR_BLACK           0x0000
#define COLOR_WHITE           0xFFFF
#define COLOR_GREEN           0x07E0
#define COLOR_YELLOW          0xFFE0
#define COLOR_CYAN            0x07FF
#define COLOR_RED             0xF800
#define COLOR_DARK_GREEN      0x03E0
#define COLOR_DARK_GRAY       0x2104   /* Subtle grid lines */
#define COLOR_LIGHT_GRAY      0xC618
#define COLOR_ORANGE          0xFD20
#define COLOR_MAGENTA         0xF81F
#define COLOR_TEAL            0x0410   /* Muted teal for accents */
#define COLOR_PANEL_BG        0x39C7   /* Dark charcoal panels — visible! */
#define COLOR_PANEL_BORDER    0x5ACB   /* Panel border accent */
#define COLOR_AXIS_LABEL      0x8C51   /* Dim gray for axis text */
#define COLOR_STATS_BG        0x2945   /* Dark blue-gray for stats box */
#define COLOR_STATS_TEXT      0xBDF7   /* Light gray for stats values */
#define COLOR_CH1             0xFFC0   /* Warm amber for CH1 */
#define COLOR_CH2             0x07FF   /* Cyan for CH2 */
#define COLOR_GRID            COLOR_DARK_GRAY
#define COLOR_TEXT            COLOR_WHITE
#define COLOR_BACKGROUND      COLOR_BLACK
#define COLOR_WARNING         COLOR_RED

/* ── Time Division ── */
#define NUM_TIME_DIVS         7
__attribute__((unused))
static const uint32_t TIME_DIV_US[] = { 10, 20, 50, 100, 200, 500, 1000 };
__attribute__((unused))
static const char* TIME_DIV_LABELS[] = {
    "10us", "20us", "50us", "100us", "200us", "500us", "1ms"
};
__attribute__((unused))
static const uint32_t TIME_DIV_ARR[] = {
    100-1, 200-1, 500-1, 1000-1, 2000-1, 5000-1, 10000-1
};

/* ── Voltage Division ── */
#define NUM_VOLT_DIVS         4
__attribute__((unused))
static const float VOLT_DIV_VALUES[] = { 0.5f, 1.0f, 2.0f, 5.0f };
__attribute__((unused))
static const char* VOLT_DIV_LABELS[] = { "0.5V", "1V", "2V", "5V" };
#define VOLTAGE_DIVIDER_RATIO  11.0f

/* ── Debounce ── */
#define BUTTON_DEBOUNCE_MS    200

/* ── ADC Watchdog ── */
#define ADC_WATCHDOG_HIGH     3700
#define ADC_WATCHDOG_LOW      0

/* ── Function Generator ── */
#define FUNCGEN_LUT_SIZE      64
#define FUNCGEN_PWM_ARR       255       /* 8-bit resolution */
#define FUNCGEN_DEFAULT_FREQ  10000     /* 10 kHz */
#define FUNCGEN_TIM_CLOCK     100000000UL  /* APB1 timer clock (MHz) */

#endif /* CONFIG_H */
