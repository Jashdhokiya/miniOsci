/**
 * @file    config.h
 * @brief   Oscilloscope constants — adapted for CubeMX project
 */

#ifndef CONFIG_H
#define CONFIG_H

/* ── Sampling ── */
#define SAMPLE_BUFFER_SIZE    256
#define ADC_BUFFER_SIZE       (SAMPLE_BUFFER_SIZE * 2)  /* interleaved CH1+CH2 */
#define ADC_MAX               4095
#define VREF                  3.3f
#define NUM_CHANNELS          2

/* ── Display ── */
#define SCREEN_WIDTH          240
#define SCREEN_HEIGHT         320
#define WAVE_X_OFFSET         0
#define WAVE_Y_OFFSET         20
#define WAVE_WIDTH            SCREEN_WIDTH
#define WAVE_HEIGHT           (SCREEN_HEIGHT - 50)
#define GRID_DIVISIONS_X      8
#define GRID_DIVISIONS_Y      6

/* ── Colors (RGB565) ── */
#define COLOR_BLACK           0x0000
#define COLOR_WHITE           0xFFFF
#define COLOR_GREEN           0x07E0
#define COLOR_YELLOW          0xFFE0
#define COLOR_CYAN            0x07FF
#define COLOR_RED             0xF800
#define COLOR_DARK_GREEN      0x03E0
#define COLOR_DARK_GRAY       0x4208
#define COLOR_LIGHT_GRAY      0xC618
#define COLOR_ORANGE          0xFD20
#define COLOR_MAGENTA         0xF81F
#define COLOR_CH1             COLOR_YELLOW
#define COLOR_CH2             COLOR_CYAN
#define COLOR_GRID            COLOR_DARK_GRAY
#define COLOR_TEXT            COLOR_WHITE
#define COLOR_BACKGROUND      COLOR_BLACK
#define COLOR_WARNING         COLOR_RED

/* ── Time Division ── */
#define NUM_TIME_DIVS         7
static const uint32_t TIME_DIV_US[] = { 10, 20, 50, 100, 200, 500, 1000 };
static const char* TIME_DIV_LABELS[] = {
    "10us", "20us", "50us", "100us", "200us", "500us", "1ms"
};
static const uint32_t TIME_DIV_ARR[] = {
    100-1, 200-1, 500-1, 1000-1, 2000-1, 5000-1, 10000-1
};

/* ── Voltage Division ── */
#define NUM_VOLT_DIVS         4
static const float VOLT_DIV_VALUES[] = { 0.5f, 1.0f, 2.0f, 5.0f };
static const char* VOLT_DIV_LABELS[] = { "0.5V", "1V", "2V", "5V" };
#define VOLTAGE_DIVIDER_RATIO  11.0f

/* ── Debounce ── */
#define BUTTON_DEBOUNCE_MS    200

/* ── ADC Watchdog ── */
#define ADC_WATCHDOG_HIGH     3700
#define ADC_WATCHDOG_LOW      0

#endif /* CONFIG_H */
