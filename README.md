# MiniOsci — Dual-Channel Oscilloscope + Function Generator

A portable dual-channel oscilloscope with built-in function generator, built on the **STM32F411CEU6 (BlackPill)** with a **2.8″ ILI9341 TFT** display. Designed as an embedded systems lab project.

---

## Features

| Feature | Details |
|---|---|
| **Channels** | 2 (PA0 + PA1), interleaved ADC with DMA |
| **Display** | 240×320 ILI9341 TFT over SPI (50 MHz) |
| **Time/Div** | 10 µs – 1 ms (7 steps) |
| **Volt/Div** | 0.5 V – 5 V (4 steps) via 1:11 resistor divider |
| **Measurements** | Real-time frequency & Vpp |
| **UI Controls** | 4 push-buttons: Run/Hold, Channel, Time/Div, Volt/Div |
| **Function Generator** | PWM-based, 4 waveforms (sine, square, triangle, sawtooth), 100 Hz – 10 kHz |
| **ADC Watchdog** | Hardware over-voltage alert (threshold: 3700/4095) |
| **UART** | Debug output on USART1 (PA9/PA10) |

---

## Hardware

### Components

- STM32F411CEU6 (WeAct BlackPill V3)
- 2.8″ ILI9341 SPI TFT (240×320)
- 4× tactile push-buttons (PB0–PB3)
- 2× 10 kΩ + 100 kΩ resistor voltage dividers (1:11 ratio)
- **Function generator filter**: 1 kΩ resistor + 10 nF capacitor (RC low-pass, fc ≈ 16 kHz)

### Pin Mapping

| Pin | Function |
|-----|----------|
| PA0 | ADC Channel 1 |
| PA1 | ADC Channel 2 |
| PA3 | TFT DC (Data/Command) |
| PA4 | TFT CS (Chip Select) |
| PA5 | SPI1 SCK |
| PA7 | SPI1 MOSI |
| PA6 | TIM3 CH1 (Input Capture) |
| PA9 | USART1 TX |
| PA10 | USART1 RX |
| PB0 | Button — Run/Hold (EXTI) |
| PB1 | Button — Time/Div (EXTI) |
| PB2 | Button — Volt/Div (EXTI) |
| PB3 | Button — Channel Select (EXTI) |
| PB6 | TFT RST |
| PB7 | TFT LED (Backlight) |
| **PB8** | **Function Generator Output (TIM4 CH3 PWM)** |

---

## Function Generator

The built-in function generator outputs analog waveforms via **PWM + RC low-pass filtering** on pin **PB8**.

### How It Works

```
TIM5 ISR              TIM4 PWM             External RC Filter
(step timer) ──────► CCR3 update ──────►  PB8 → 1kΩ → ┬── Analog Out
64 samples/period     ~390 kHz carrier           10nF ┴ GND
```

- **TIM4** generates a ~390 kHz PWM carrier (8-bit resolution, ARR=255)
- **TIM5** fires interrupts to step through a 64-sample lookup table
- Each interrupt updates TIM4's duty cycle register (CCR3)
- The external **1 kΩ + 10 nF** RC filter smooths the PWM into an analog signal

### Supported Waveforms

| Waveform | LUT | Description |
|----------|-----|-------------|
| Sine | 64 samples | Pre-computed `sin()` values (0–255) |
| Square | 64 samples | First half = 255, second half = 0 |
| Triangle | 64 samples | Linear ramp up then down |
| Sawtooth | 64 samples | Linear ramp from 0 to 255 |

### Wiring

Connect the RC filter to PB8 and route the output to an ADC input to test:

```
PB8 ──[ 1kΩ ]──┬── Analog Output ──► PA0 (to test with oscilloscope)
               │
              [10nF]
               │
              GND
```

### Default Configuration

- **Waveform**: Sine
- **Frequency**: 1 kHz
- Starts automatically at boot
- Status shown on display bottom bar: `FG:SIN 1kHz`

---

## Project Structure

```
MiniOsci/
├── Core/
│   ├── Inc/                    # Header files
│   │   ├── config.h            # All oscilloscope & FG constants
│   │   ├── func_gen.h          # Function generator API
│   │   ├── osc_adc.h           # ADC + DMA control
│   │   ├── osc_display.h       # Waveform & grid rendering
│   │   ├── osc_signal.h        # Frequency & Vpp measurement
│   │   ├── osc_ui.h            # Button/state management
│   │   ├── fonts.h             # Bitmap font definitions
│   │   └── ...                 # CubeMX-generated peripheral headers
│   ├── Src/                    # Source files
│   │   ├── func_gen.c          # Function generator (LUTs + step ISR)
│   │   └── ...                 # Mirrors Inc/
│   └── Startup/                # Startup assembly (startup_stm32f411ceux.s)
├── Drivers/
│   ├── CMSIS/                  # ARM CMSIS core headers
│   ├── ILI9341/                # ILI9341 TFT display driver
│   └── STM32F4xx_HAL_Driver/   # STM32 HAL library
├── MiniOsci.ioc                # STM32CubeMX configuration
├── STM32F411CEUX_FLASH.ld      # Flash linker script
├── STM32F411CEUX_RAM.ld        # RAM linker script
├── .gitignore
└── README.md
```

---

## Getting Started

### Prerequisites

- [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html) (v1.13+)
- ST-Link V2 or compatible SWD debugger

### Build & Flash

1. Clone the repository:
   ```bash
   git clone https://github.com/Jashdhokiya/miniOsci.git
   ```
2. Open **STM32CubeIDE** → *File → Import → General → Import an Existing STM32CubeMX Configuration File (.ioc)* → select `MiniOsci.ioc`.
3. Build: **Project → Build All** (`Ctrl+B`).
4. Flash: **Run → Debug** (`F11`) with your ST-Link connected.

### Reconfiguring Peripherals

Open `MiniOsci.ioc` in STM32CubeMX to modify pin assignments, clock tree, or peripheral settings, then regenerate code.

---

## Architecture

```
┌─────────────┐     TIM2 TRGO      ┌──────────┐    DMA (Circular)
│  Timer 2    │ ──────────────────► │   ADC1   │ ──────────────────► adc_buffer[512]
│ (sampling)  │                     │ (2ch scan)│                     (interleaved)
└─────────────┘                     └──────────┘
                                                          │
                                         DMA Half/Complete Callback
                                                          ▼
                                                   ┌─────────────┐
                                                   │  Main Loop  │
                                                   │  (render)   │
                                                   └──────┬──────┘
                                                          │
                                          ┌───────────────┴───────────────┐
                                          ▼                               ▼
                                   Signal Processing              ILI9341 Display
                                   (freq, Vpp calc)              (grid + waveform)

┌─────────────┐     TIM5 ISR        ┌──────────┐    RC Filter
│  Timer 5    │ ──────────────────► │  TIM4    │ ──────────────────► Analog Out (PB8)
│ (step rate) │   update CCR3       │ (PWM CH3)│    1kΩ + 10nF
└─────────────┘                     └──────────┘
```

- **TIM2** triggers ADC conversions at a configurable rate
- **ADC1** scans 2 channels per trigger, results go to a circular DMA buffer
- On DMA complete callback, `bufferReady` flag is set
- Main loop processes the buffer: computes frequency/Vpp, draws the waveform
- **EXTI** interrupts on PB0–PB3 handle button presses with software debouncing
- **TIM4** outputs high-frequency PWM (~390 kHz) on PB8 for the function generator
- **TIM5** fires interrupts to step through a waveform lookup table, updating TIM4's duty cycle

---

## License

This project is for educational purposes as part of an embedded systems lab course.
STM32 HAL drivers and CMSIS are licensed by STMicroelectronics/ARM under their respective licenses (see `Drivers/*/LICENSE.txt`).
